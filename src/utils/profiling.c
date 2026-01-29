/**
 * @file profiling.c
 * @brief GStreamer pipeline profiling and performance measurement implementation
 *
 * Implements performance profiling for identifying bottlenecks in queue buffering
 * and synchronization. Uses GStreamer probes and sampling to collect metrics.
 */

#include "profiling.h"
#include "logging.h"
#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

/* ========== Utility Functions ========== */

/**
 * Get current time in microseconds
 */
static guint64 get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (guint64) tv.tv_sec * 1000000 + (guint64) tv.tv_usec;
}

/**
 * Calculate average from array of values
 */
static gdouble calculate_average(GQueue *queue)
{
    if (g_queue_is_empty(queue)) {
        return 0.0;
    }

    gdouble sum = 0.0;
    guint count = 0;
    GList *item = g_queue_peek_head_link(queue);

    while (item) {
        PerformanceSample *sample = (PerformanceSample *) item->data;
        sum += sample->current_fps;
        count++;
        item = g_list_next(item);
    }

    return count > 0 ? sum / count : 0.0;
}

/**
 * Calculate standard deviation
 */
static gdouble calculate_std_dev(GQueue *queue, gdouble mean)
{
    if (g_queue_is_empty(queue) || queue->length < 2) {
        return 0.0;
    }

    gdouble sum_sq_dev = 0.0;
    GList *item = g_queue_peek_head_link(queue);

    while (item) {
        PerformanceSample *sample = (PerformanceSample *) item->data;
        gdouble dev = sample->current_fps - mean;
        sum_sq_dev += dev * dev;
        item = g_list_next(item);
    }

    return sqrt(sum_sq_dev / queue->length);
}

/**
 * GStreamer pad probe callback to measure latency
 */
static GstPadProbeReturn probe_callback(GstPad *pad G_GNUC_UNUSED, GstPadProbeInfo *info,
                                        gpointer user_data)
{
    ProfilingContext *ctx = (ProfilingContext *) user_data;
    if (!ctx->is_active) {
        return GST_PAD_PROBE_OK;
    }

    if (GST_IS_BUFFER(info->data)) {
        GstBuffer *buffer = GST_BUFFER_CAST(info->data);
        GstClockTime timestamp = GST_BUFFER_TIMESTAMP(buffer);
        guint64 arrival_us = get_time_us();

        profiling_record_frame(ctx, (guint) (ctx->frame_timings->length), timestamp, arrival_us,
                               FALSE);
    }

    return GST_PAD_PROBE_OK;
}

/* ========== Public API Implementation ========== */

ProfilingContext *profiling_context_create(GstElement *pipeline, guint sample_interval_ms,
                                           guint max_samples)
{
    if (!GST_IS_PIPELINE(pipeline)) {
        LOG_ERROR("Invalid pipeline object");
        return NULL;
    }

    ProfilingContext *ctx = g_new0(ProfilingContext, 1);
    if (!ctx) {
        LOG_ERROR("Failed to allocate profiling context");
        return NULL;
    }

    ctx->pipeline = pipeline;
    ctx->sample_interval_ms = sample_interval_ms > 0 ? sample_interval_ms : 100;
    ctx->max_samples = max_samples > 0 ? max_samples : 0; // 0 = unlimited
    ctx->frame_timings = g_queue_new();
    ctx->performance_samples = g_queue_new();
    ctx->element_metrics = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    ctx->profile_start_time_us = get_time_us();
    ctx->is_active = FALSE;

    LOG_DEBUG("Created profiling context: %p, interval=%ums", ctx, ctx->sample_interval_ms);
    return ctx;
}

gboolean profiling_start(ProfilingContext *ctx)
{
    if (!ctx || !ctx->pipeline) {
        LOG_ERROR("Invalid profiling context");
        return FALSE;
    }

    ctx->is_active = TRUE;
    ctx->profile_start_time_us = get_time_us();

    /* Install probes on key elements */
    GstIterator *iter = gst_bin_iterate_elements(GST_BIN(ctx->pipeline));
    if (!iter) {
        LOG_WARNING("Could not create element iterator");
        ctx->is_active = FALSE;
        return FALSE;
    }

    GValue item = G_VALUE_INIT;
    while (gst_iterator_next(iter, &item) == GST_ITERATOR_OK) {
        GstElement *element = g_value_get_object(&item);
        if (!element)
            continue;

        GstPad *sink_pad = gst_element_get_static_pad(element, "sink");
        if (sink_pad) {
            gst_pad_add_probe(sink_pad, GST_PAD_PROBE_TYPE_BUFFER, probe_callback, ctx, NULL);
            gst_object_unref(sink_pad);
        }

        g_value_reset(&item);
    }
    g_value_unset(&item);
    gst_iterator_free(iter);

    LOG_INFO("Profiling started");
    return TRUE;
}

void profiling_stop(ProfilingContext *ctx)
{
    if (!ctx)
        return;

    ctx->is_active = FALSE;
    LOG_INFO("Profiling stopped");
}

gboolean profiling_collect_sample(ProfilingContext *ctx)
{
    if (!ctx || !ctx->is_active) {
        return FALSE;
    }

    guint64 current_time_us = get_time_us();

    /* Calculate current FPS from recent frame timings */
    gdouble current_fps = 0.0;
    if (!g_queue_is_empty(ctx->frame_timings)) {
        /* Count frames in last second */
        guint frames_in_last_sec = 0;
        guint64 one_sec_ago = current_time_us - 1000000;
        GList *item = g_queue_peek_tail_link(ctx->frame_timings);

        while (item) {
            FrameTiming *timing = (FrameTiming *) item->data;
            if (timing->arrival_time_us >= one_sec_ago) {
                frames_in_last_sec++;
            } else {
                break;
            }
            item = g_list_previous(item);
        }
        current_fps = (gdouble) frames_in_last_sec;
    }

    PerformanceSample *sample = g_new0(PerformanceSample, 1);
    sample->timestamp_us = current_time_us;
    sample->frame_number = (guint) (ctx->frame_timings->length);
    sample->current_fps = current_fps;
    sample->cpu_usage_percent = 0.0; /* Placeholder */
    sample->queue_depth_bytes = 0;   /* Placeholder */
    sample->num_dropped_frames = 0;  /* Placeholder */

    g_queue_push_tail(ctx->performance_samples, sample);

    /* Trim old samples if max_samples is set */
    if (ctx->max_samples > 0) {
        while ((guint) ctx->performance_samples->length > ctx->max_samples) {
            PerformanceSample *old =
                (PerformanceSample *) g_queue_pop_head(ctx->performance_samples);
            g_free(old);
        }
    }

    return TRUE;
}

void profiling_record_frame(ProfilingContext *ctx, guint frame_num, guint64 timestamp_ns,
                            guint64 arrival_time_us, gboolean dropped)
{
    if (!ctx)
        return;

    FrameTiming *timing = g_new0(FrameTiming, 1);
    timing->frame_number = frame_num;
    timing->timestamp_ns = timestamp_ns;
    timing->arrival_time_us = arrival_time_us;
    timing->was_dropped = dropped;

    /* Calculate source-to-sink latency if timestamp available */
    if (timestamp_ns != GST_CLOCK_TIME_NONE) {
        guint64 timestamp_us = timestamp_ns / 1000;
        if (arrival_time_us >= timestamp_us) {
            timing->source_to_sink_latency_us = arrival_time_us - timestamp_us;
        }
    }

    g_queue_push_tail(ctx->frame_timings, timing);

    /* Trim old frames if max_samples is set */
    if (ctx->max_samples > 0) {
        while ((guint) ctx->frame_timings->length > ctx->max_samples) {
            FrameTiming *old = (FrameTiming *) g_queue_pop_head(ctx->frame_timings);
            g_free(old);
        }
    }
}

ProfileMetrics *profiling_identify_bottlenecks(ProfilingContext *ctx)
{
    if (!ctx)
        return NULL;

    /* Analyze collected samples to identify bottlenecks */
    /* Bottleneck criteria:
     * - High queue depth (element not consuming fast enough)
     * - High buffer drop rate
     * - High latency through element
     * - Element causing FPS drops
     */

    /* This is a simplified implementation */
    ProfileMetrics *metrics = NULL;

    if (g_hash_table_size(ctx->element_metrics) > 0) {
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init(&iter, ctx->element_metrics);

        while (g_hash_table_iter_next(&iter, &key, &value)) {
            ProfileMetrics *m = (ProfileMetrics *) value;
            /* Identify as bottleneck if drop ratio > 1% or avg latency > 50ms */
            if (m->buffer_drop_ratio > 0.01 || m->avg_latency_us > 50000) {
                m->is_bottleneck = TRUE;
            }
        }
    }

    return metrics;
}

ProfileMetrics *profiling_get_element_metrics(ProfilingContext *ctx, const char *element_name)
{
    if (!ctx || !element_name) {
        return NULL;
    }

    return (ProfileMetrics *) g_hash_table_lookup(ctx->element_metrics, element_name);
}

gboolean profiling_get_fps_stats(ProfilingContext *ctx, gdouble *current_fps, gdouble *average_fps,
                                 gdouble *min_fps, gdouble *max_fps, gdouble *std_dev_fps)
{
    if (!ctx) {
        return FALSE;
    }

    if (g_queue_is_empty(ctx->performance_samples)) {
        if (current_fps)
            *current_fps = 0.0;
        if (average_fps)
            *average_fps = 0.0;
        if (min_fps)
            *min_fps = 0.0;
        if (max_fps)
            *max_fps = 0.0;
        if (std_dev_fps)
            *std_dev_fps = 0.0;
        return FALSE;
    }

    gdouble avg = calculate_average(ctx->performance_samples);
    gdouble min_val = G_MAXDOUBLE;
    gdouble max_val = 0.0;

    GList *item = g_queue_peek_head_link(ctx->performance_samples);
    while (item) {
        PerformanceSample *sample = (PerformanceSample *) item->data;
        if (sample->current_fps < min_val)
            min_val = sample->current_fps;
        if (sample->current_fps > max_val)
            max_val = sample->current_fps;
        item = g_list_next(item);
    }

    if (current_fps) {
        PerformanceSample *last = (PerformanceSample *) g_queue_peek_tail(ctx->performance_samples);
        *current_fps = last ? last->current_fps : 0.0;
    }
    if (average_fps)
        *average_fps = avg;
    if (min_fps)
        *min_fps = min_val == G_MAXDOUBLE ? 0.0 : min_val;
    if (max_fps)
        *max_fps = max_val;
    if (std_dev_fps)
        *std_dev_fps = calculate_std_dev(ctx->performance_samples, avg);

    return TRUE;
}

gboolean profiling_get_queue_stats(ProfilingContext *ctx, guint64 *total_queue_bytes,
                                   guint *max_queue_depth, guint *num_queues)
{
    if (!ctx) {
        return FALSE;
    }

    if (total_queue_bytes)
        *total_queue_bytes = 0;
    if (max_queue_depth)
        *max_queue_depth = 0;
    if (num_queues)
        *num_queues = 0;

    /* Query GStreamer queue elements for current state */
    GstIterator *iter = gst_bin_iterate_elements(GST_BIN(ctx->pipeline));
    if (!iter) {
        return FALSE;
    }

    GValue item = G_VALUE_INIT;
    guint queue_count = 0;
    guint64 total_bytes = 0;
    guint max_depth = 0;

    while (gst_iterator_next(iter, &item) == GST_ITERATOR_OK) {
        GstElement *element = g_value_get_object(&item);
        if (!element) {
            g_value_reset(&item);
            continue;
        }

        const char *element_name = GST_ELEMENT_NAME(element);
        if (g_str_has_prefix(element_name, "queue")) {
            queue_count++;

            guint cur_level_buffers = 0;
            g_object_get(G_OBJECT(element), "current-level-buffers", &cur_level_buffers, NULL);

            guint cur_level_bytes = 0;
            g_object_get(G_OBJECT(element), "current-level-bytes", &cur_level_bytes, NULL);

            total_bytes += (guint64) cur_level_bytes;
            if ((guint) cur_level_buffers > max_depth) {
                max_depth = (guint) cur_level_buffers;
            }
        }

        g_value_reset(&item);
    }

    g_value_unset(&item);
    gst_iterator_free(iter);

    if (total_queue_bytes)
        *total_queue_bytes = total_bytes;
    if (max_queue_depth)
        *max_queue_depth = max_depth;
    if (num_queues)
        *num_queues = queue_count;

    return queue_count > 0;
}

gboolean profiling_get_sync_metrics(ProfilingContext *ctx, gdouble *buffer_drop_ratio,
                                    guint64 *jitter_us, guint64 *max_latency_us)
{
    if (!ctx) {
        return FALSE;
    }

    if (buffer_drop_ratio)
        *buffer_drop_ratio = 0.0;
    if (jitter_us)
        *jitter_us = 0;
    if (max_latency_us)
        *max_latency_us = 0;

    if (g_queue_is_empty(ctx->frame_timings)) {
        return FALSE;
    }

    /* Calculate buffer drop ratio */
    guint dropped = 0;
    guint total = 0;
    guint64 max_lat = 0;
    guint64 prev_latency = 0;
    guint64 jitter_sum = 0;
    guint latency_samples = 0;

    GList *item = g_queue_peek_head_link(ctx->frame_timings);
    while (item) {
        FrameTiming *timing = (FrameTiming *) item->data;
        total++;

        if (timing->was_dropped) {
            dropped++;
        }

        if (timing->source_to_sink_latency_us > max_lat) {
            max_lat = timing->source_to_sink_latency_us;
        }

        /* Calculate jitter (difference in latencies) */
        if (latency_samples > 0) {
            guint64 latency_diff = timing->source_to_sink_latency_us > prev_latency
                                       ? timing->source_to_sink_latency_us - prev_latency
                                       : prev_latency - timing->source_to_sink_latency_us;
            jitter_sum += latency_diff;
        }
        prev_latency = timing->source_to_sink_latency_us;
        latency_samples++;

        item = g_list_next(item);
    }

    if (buffer_drop_ratio) {
        *buffer_drop_ratio = total > 0 ? (gdouble) dropped / total : 0.0;
    }
    if (jitter_us) {
        *jitter_us = latency_samples > 1 ? jitter_sum / (latency_samples - 1) : 0;
    }
    if (max_latency_us) {
        *max_latency_us = max_lat;
    }

    return TRUE;
}

gchar *profiling_generate_report(ProfilingContext *ctx)
{
    if (!ctx) {
        return g_strdup("Invalid profiling context");
    }

    GString *report = g_string_new("");
    g_string_append_printf(report, "========================================\n"
                                   "GStreamer Pipeline Profiling Report\n"
                                   "========================================\n\n");

    /* FPS Statistics */
    gdouble current_fps = 0.0, avg_fps = 0.0, min_fps = 0.0, max_fps = 0.0, std_dev = 0.0;
    if (profiling_get_fps_stats(ctx, &current_fps, &avg_fps, &min_fps, &max_fps, &std_dev)) {
        g_string_append_printf(report,
                               "Frame Rate Statistics:\n"
                               "  Current FPS:  %.2f\n"
                               "  Average FPS:  %.2f\n"
                               "  Min FPS:      %.2f\n"
                               "  Max FPS:      %.2f\n"
                               "  Std Dev:      %.2f\n\n",
                               current_fps, avg_fps, min_fps, max_fps, std_dev);
    }

    /* Queue Statistics */
    guint64 total_queue_bytes = 0;
    guint max_queue_depth = 0;
    guint num_queues = 0;
    if (profiling_get_queue_stats(ctx, &total_queue_bytes, &max_queue_depth, &num_queues)) {
        g_string_append_printf(report,
                               "Queue Buffering Statistics:\n"
                               "  Number of Queues:  %u\n"
                               "  Total Queue Depth: %" G_GUINT64_FORMAT " bytes\n"
                               "  Max Queue Level:   %u buffers\n\n",
                               num_queues, total_queue_bytes, max_queue_depth);
    }

    /* Synchronization Metrics */
    gdouble drop_ratio = 0.0;
    guint64 jitter_us = 0, max_latency_us = 0;
    if (profiling_get_sync_metrics(ctx, &drop_ratio, &jitter_us, &max_latency_us)) {
        g_string_append_printf(report,
                               "Synchronization Metrics:\n"
                               "  Buffer Drop Ratio: %.2f%%\n"
                               "  Jitter:            %" G_GUINT64_FORMAT " µs\n"
                               "  Max Latency:       %" G_GUINT64_FORMAT " µs\n\n",
                               drop_ratio * 100.0, jitter_us, max_latency_us);
    }

    /* Frame timing samples */
    g_string_append_printf(report,
                           "Performance Samples Collected: %u\n"
                           "Frame Timings Recorded:        %u\n\n",
                           ctx->performance_samples->length, ctx->frame_timings->length);

    /* Recommendations */
    g_string_append_printf(report, "Optimization Recommendations:\n");
    if (avg_fps < 115.0) {
        g_string_append_printf(report,
                               "  ⚠ Average FPS (%.2f) is below 120 fps target\n"
                               "    - Check queue buffer sizes\n"
                               "    - Profile individual elements for bottlenecks\n"
                               "    - Consider GPU utilization\n",
                               avg_fps);
    } else if (std_dev > 5.0) {
        g_string_append_printf(report,
                               "  ⚠ High frame rate variance (std dev: %.2f)\n"
                               "    - Synchronization issues detected\n"
                               "    - Review queue latency settings\n",
                               std_dev);
    } else {
        g_string_append_printf(report, "  ✓ Pipeline performing well at target frame rate\n");
    }

    if (drop_ratio > 0.01) {
        g_string_append_printf(report,
                               "  ⚠ Buffer drop ratio (%.2f%%) indicates element under load\n"
                               "    - Increase queue max-size-buffers\n"
                               "    - Reduce pipeline complexity\n",
                               drop_ratio * 100.0);
    }

    g_string_append_printf(report, "\n========================================\n");

    return g_string_free(report, FALSE);
}

gboolean profiling_export_json(ProfilingContext *ctx, const char *filename)
{
    if (!ctx || !filename) {
        return FALSE;
    }

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        LOG_ERROR("Failed to open file for profiling export: %s", filename);
        return FALSE;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"profiling_session\": {\n");
    fprintf(fp, "    \"duration_us\": %" G_GUINT64_FORMAT ",\n",
            get_time_us() - ctx->profile_start_time_us);
    fprintf(fp, "    \"total_frames_recorded\": %u,\n", ctx->frame_timings->length);
    fprintf(fp, "    \"total_samples\": %u\n", ctx->performance_samples->length);
    fprintf(fp, "  },\n");

    /* Export FPS statistics */
    gdouble current_fps = 0.0, avg_fps = 0.0;
    profiling_get_fps_stats(ctx, &current_fps, &avg_fps, NULL, NULL, NULL);
    fprintf(fp, "  \"fps_statistics\": {\n");
    fprintf(fp, "    \"current_fps\": %.2f,\n", current_fps);
    fprintf(fp, "    \"average_fps\": %.2f\n", avg_fps);
    fprintf(fp, "  },\n");

    /* Export samples */
    fprintf(fp, "  \"samples\": [\n");
    GList *item = g_queue_peek_head_link(ctx->performance_samples);
    gboolean first = TRUE;
    while (item) {
        PerformanceSample *sample = (PerformanceSample *) item->data;
        if (!first)
            fprintf(fp, ",\n");
        fprintf(fp, "    { \"timestamp_us\": %" G_GUINT64_FORMAT ", \"fps\": %.2f }",
                sample->timestamp_us, sample->current_fps);
        first = FALSE;
        item = g_list_next(item);
    }
    fprintf(fp, "\n  ]\n");
    fprintf(fp, "}\n");

    fclose(fp);
    LOG_INFO("Profiling data exported to: %s", filename);
    return TRUE;
}

void profiling_context_free(ProfilingContext *ctx)
{
    if (!ctx)
        return;

    if (ctx->frame_timings) {
        g_queue_free_full(ctx->frame_timings, g_free);
    }
    if (ctx->performance_samples) {
        g_queue_free_full(ctx->performance_samples, g_free);
    }
    if (ctx->element_metrics) {
        g_hash_table_destroy(ctx->element_metrics);
    }

    g_free(ctx);
    LOG_DEBUG("Freed profiling context");
}
