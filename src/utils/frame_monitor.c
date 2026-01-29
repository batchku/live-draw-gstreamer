#include "frame_monitor.h"
#include "logging.h"
#include <math.h>
#include <string.h>
#include <time.h>

/**
 * Internal frame monitor context
 */
struct FrameMonitor {
    GQueue *frame_times;    /* Queue of GstClockTime timestamps */
    guint64 total_frames;   /* Total frames since creation */
    guint64 dropped_frames; /* Total dropped frames detected */
    guint window_size;      /* Maximum frames in window */
    guint64 last_timestamp; /* Previous frame timestamp */
    gboolean first_frame;   /* True until first frame received */
    GMutex lock;            /* Thread-safety lock */
};

/**
 * Create a new frame monitor.
 */
FrameMonitor *frame_monitor_create(void)
{
    FrameMonitor *monitor = g_new0(FrameMonitor, 1);
    if (!monitor) {
        LOG_ERROR("Failed to allocate frame monitor");
        return NULL;
    }

    monitor->frame_times = g_queue_new();
    monitor->window_size = FRAME_MONITOR_WINDOW_SIZE;
    monitor->total_frames = 0;
    monitor->dropped_frames = 0;
    monitor->last_timestamp = 0;
    monitor->first_frame = TRUE;
    g_mutex_init(&monitor->lock);

    LOG_DEBUG("Frame monitor created (window=%u frames, target=%u fps, tolerance=%u fps)",
              monitor->window_size, FRAME_MONITOR_TARGET_FPS, FRAME_MONITOR_TOLERANCE_FPS);

    return monitor;
}

/**
 * Record a frame received event.
 *
 * Detects drops by analyzing timestamp gaps. A drop is detected when
 * the gap between frames exceeds 1.5x the expected frame duration.
 */
gboolean frame_monitor_on_frame(FrameMonitor *monitor, GstClockTime timestamp)
{
    if (!monitor) {
        return FALSE;
    }

    g_mutex_lock(&monitor->lock);

    monitor->total_frames++;

    /* Detect frame drops on second and subsequent frames */
    if (!monitor->first_frame && monitor->last_timestamp > 0) {
        /* Expected duration between frames at 120 fps: ~8.33ms = 8,333,333 ns */
        GstClockTime expected_duration = GST_SECOND / FRAME_MONITOR_TARGET_FPS;

        /* Allow 1.5x tolerance for adaptive threshold */
        GstClockTime max_acceptable_gap = (expected_duration * 3) / 2;

        GstClockTime time_gap = timestamp - monitor->last_timestamp;

        if (time_gap > max_acceptable_gap && time_gap > 0) {
            /* Calculate number of missing frames based on gap */
            guint64 missing_frames = (time_gap / expected_duration) - 1;
            if (missing_frames > 0) {
                monitor->dropped_frames += missing_frames;
                LOG_WARNING("Frame drop detected: gap=%" GST_TIME_FORMAT
                            " (expected ~%" GST_TIME_FORMAT "), missing frames=%" G_GUINT64_FORMAT,
                            GST_TIME_ARGS(time_gap), GST_TIME_ARGS(expected_duration),
                            missing_frames);
            }
        }
    }

    monitor->last_timestamp = timestamp;
    if (monitor->first_frame) {
        monitor->first_frame = FALSE;
    }

    /* Store timestamp in window queue, maintain window size */
    guint64 *time_ptr = g_new(guint64, 1);
    *time_ptr = (guint64) timestamp;
    g_queue_push_tail(monitor->frame_times, time_ptr);

    if (g_queue_get_length(monitor->frame_times) > monitor->window_size) {
        guint64 *old_time = g_queue_pop_head(monitor->frame_times);
        g_free(old_time);
    }

    g_mutex_unlock(&monitor->lock);
    return TRUE;
}

/**
 * Get current frame rate statistics.
 *
 * Calculates fps from window timestamps, including min/max and std deviation.
 */
FrameMonitorStats *frame_monitor_get_stats(FrameMonitor *monitor)
{
    if (!monitor) {
        return NULL;
    }

    FrameMonitorStats *stats = g_new0(FrameMonitorStats, 1);
    if (!stats) {
        return NULL;
    }

    g_mutex_lock(&monitor->lock);

    stats->total_frames = monitor->total_frames;
    stats->dropped_frames = monitor->dropped_frames;

    guint window_frames = g_queue_get_length(monitor->frame_times);

    if (window_frames < 2) {
        stats->current_fps = 0.0;
        stats->average_fps = 0.0;
        stats->fps_min = 0.0;
        stats->fps_max = 0.0;
        stats->fps_std_dev = 0.0;
        g_mutex_unlock(&monitor->lock);
        return stats;
    }

    /* Get first and last timestamps in window */
    guint64 *first_time = (guint64 *) g_queue_peek_head(monitor->frame_times);
    guint64 *last_time = (guint64 *) g_queue_peek_tail(monitor->frame_times);

    if (first_time && last_time && *last_time > *first_time) {
        GstClockTime window_duration_ns = *last_time - *first_time;
        stats->session_duration_ns = window_duration_ns;

        /* Calculate average fps over window */
        gdouble window_duration_s = (gdouble) window_duration_ns / (gdouble) GST_SECOND;
        if (window_duration_s > 0) {
            stats->average_fps = (gdouble) (window_frames - 1) / window_duration_s;
        }

        /* Calculate per-frame intervals for current fps, min/max */
        GList *link = g_queue_peek_tail_link(monitor->frame_times);
        if (link && link->prev) {
            guint64 *prev_time = (guint64 *) link->prev->data;
            if (prev_time && *last_time > *prev_time) {
                GstClockTime interval_ns = *last_time - *prev_time;
                gdouble interval_s = (gdouble) interval_ns / (gdouble) GST_SECOND;
                if (interval_s > 0) {
                    stats->current_fps = 1.0 / interval_s;
                }
            }
        }

        /* Calculate min/max fps and std deviation */
        gdouble *intervals = g_new(gdouble, window_frames - 1);
        if (intervals) {
            gdouble sum_fps = 0.0;
            guint valid_intervals = 0;

            GList *iter = g_queue_peek_head_link(monitor->frame_times);
            guint64 *prev_ts = NULL;

            for (guint i = 0; i < window_frames && iter; i++) {
                guint64 *curr_ts = (guint64 *) iter->data;

                if (prev_ts && *curr_ts > *prev_ts) {
                    GstClockTime interval_ns = *curr_ts - *prev_ts;
                    gdouble interval_s = (gdouble) interval_ns / (gdouble) GST_SECOND;

                    if (interval_s > 0) {
                        intervals[valid_intervals] = 1.0 / interval_s;
                        sum_fps += intervals[valid_intervals];
                        valid_intervals++;
                    }
                }

                prev_ts = curr_ts;
                iter = g_list_next(iter);
            }

            if (valid_intervals > 0) {
                /* Calculate mean */
                gdouble mean_fps = sum_fps / (gdouble) valid_intervals;

                /* Calculate std deviation */
                gdouble sum_sq_dev = 0.0;
                for (guint i = 0; i < valid_intervals; i++) {
                    gdouble dev = intervals[i] - mean_fps;
                    sum_sq_dev += dev * dev;
                }
                stats->fps_std_dev = sqrt(sum_sq_dev / (gdouble) valid_intervals);

                /* Find min/max */
                stats->fps_min = intervals[0];
                stats->fps_max = intervals[0];
                for (guint i = 1; i < valid_intervals; i++) {
                    if (intervals[i] < stats->fps_min) {
                        stats->fps_min = intervals[i];
                    }
                    if (intervals[i] > stats->fps_max) {
                        stats->fps_max = intervals[i];
                    }
                }
            }

            g_free(intervals);
        }
    }

    if (monitor->total_frames > 0) {
        stats->first_frame_timestamp = monitor->total_frames;
    }
    stats->last_frame_timestamp = monitor->last_timestamp;

    g_mutex_unlock(&monitor->lock);
    return stats;
}

/**
 * Detect dropped frames in the monitoring window.
 */
FrameDropInfo *frame_monitor_detect_drops(FrameMonitor *monitor)
{
    if (!monitor) {
        return NULL;
    }

    FrameDropInfo *info = g_new0(FrameDropInfo, 1);
    if (!info) {
        return NULL;
    }

    g_mutex_lock(&monitor->lock);

    info->has_drops = FALSE;
    info->drop_count = 0;
    info->drop_rate = 0.0;
    info->largest_drop_gap_ns = 0;
    info->drop_start_timestamp = 0;

    guint window_frames = g_queue_get_length(monitor->frame_times);

    if (window_frames < 2) {
        g_mutex_unlock(&monitor->lock);
        return info;
    }

    /* Expected frame interval at 120 fps */
    GstClockTime expected_interval_ns = GST_SECOND / FRAME_MONITOR_TARGET_FPS;
    GstClockTime max_acceptable_interval = (expected_interval_ns * 3) / 2;

    guint drop_events = 0;
    GstClockTime largest_gap = 0;
    GstClockTime drop_start_ts = 0;

    GList *iter = g_queue_peek_head_link(monitor->frame_times);
    guint64 *prev_ts = NULL;

    for (guint i = 0; i < window_frames && iter; i++) {
        guint64 *curr_ts = (guint64 *) iter->data;

        if (prev_ts && *curr_ts > *prev_ts) {
            GstClockTime gap = *curr_ts - *prev_ts;

            if (gap > max_acceptable_interval) {
                if (drop_events == 0) {
                    drop_start_ts = *prev_ts;
                }
                drop_events++;
                if (gap > largest_gap) {
                    largest_gap = gap;
                }
            }
        }

        prev_ts = curr_ts;
        iter = g_list_next(iter);
    }

    if (drop_events > 0) {
        info->has_drops = TRUE;
        info->drop_count = drop_events;
        info->largest_drop_gap_ns = largest_gap;
        info->drop_start_timestamp = drop_start_ts;

        if (window_frames > 1) {
            info->drop_rate = ((gdouble) drop_events / (gdouble) (window_frames - 1)) * 100.0;
        }
    }

    g_mutex_unlock(&monitor->lock);
    return info;
}

/**
 * Validate current frame rate against target.
 */
FrameRateValidationResult frame_monitor_validate_framerate(FrameMonitor *monitor, guint target_fps,
                                                           guint tolerance_fps)
{
    if (!monitor) {
        return FRAME_RATE_INSUFFICIENT_DATA;
    }

    FrameMonitorStats *stats = frame_monitor_get_stats(monitor);
    if (!stats) {
        return FRAME_RATE_INSUFFICIENT_DATA;
    }

    FrameRateValidationResult result = FRAME_RATE_VALID;

    /* Check if we have sufficient data */
    if (g_queue_get_length(monitor->frame_times) < 30) { /* ~0.25 seconds at 120 fps */
        result = FRAME_RATE_INSUFFICIENT_DATA;
        goto cleanup;
    }

    gdouble min_fps = (gdouble) target_fps - (gdouble) tolerance_fps;
    gdouble max_fps = (gdouble) target_fps + (gdouble) tolerance_fps;

    /* Check average frame rate */
    if (stats->average_fps < min_fps) {
        result = FRAME_RATE_LOW;
        goto cleanup;
    }

    if (stats->average_fps > max_fps) {
        result = FRAME_RATE_HIGH;
        goto cleanup;
    }

    /* Check variance/stability (std dev should be < 10% of target) */
    if (stats->fps_std_dev > ((gdouble) target_fps * 0.1)) {
        result = FRAME_RATE_UNSTABLE;
        goto cleanup;
    }

cleanup:
    g_free(stats);
    return result;
}

/**
 * Get validation result description.
 */
const gchar *frame_monitor_validation_string(FrameRateValidationResult result)
{
    switch (result) {
    case FRAME_RATE_VALID:
        return "VALID";
    case FRAME_RATE_LOW:
        return "LOW (below minimum)";
    case FRAME_RATE_HIGH:
        return "HIGH (above maximum)";
    case FRAME_RATE_UNSTABLE:
        return "UNSTABLE (high variance)";
    case FRAME_RATE_INSUFFICIENT_DATA:
        return "INSUFFICIENT_DATA";
    default:
        return "UNKNOWN";
    }
}

/**
 * Log frame rate statistics.
 */
void frame_monitor_log_stats(FrameMonitor *monitor, gboolean include_detailed)
{
    if (!monitor) {
        return;
    }

    FrameMonitorStats *stats = frame_monitor_get_stats(monitor);
    if (!stats) {
        LOG_ERROR("Failed to retrieve frame rate statistics");
        return;
    }

    if (include_detailed) {
        LOG_INFO("Frame Rate Statistics: avg=%.1f fps, current=%.1f fps, "
                 "min=%.1f fps, max=%.1f fps, stddev=%.1f, "
                 "total_frames=%" G_GUINT64_FORMAT ", dropped=%" G_GUINT64_FORMAT,
                 stats->average_fps, stats->current_fps, stats->fps_min, stats->fps_max,
                 stats->fps_std_dev, stats->total_frames, stats->dropped_frames);
    } else {
        LOG_INFO("Frame Rate: %.1f fps (avg), %.1f fps (current), "
                 "%" G_GUINT64_FORMAT " frames, %" G_GUINT64_FORMAT " dropped",
                 stats->average_fps, stats->current_fps, stats->total_frames,
                 stats->dropped_frames);
    }

    g_free(stats);
}

/**
 * Log detected frame drops.
 */
void frame_monitor_log_drops(FrameMonitor *monitor)
{
    if (!monitor) {
        return;
    }

    FrameDropInfo *drops = frame_monitor_detect_drops(monitor);
    if (!drops) {
        return;
    }

    if (drops->has_drops) {
        LOG_WARNING("Frame drops detected: count=%" G_GUINT64_FORMAT
                    ", rate=%.2f%%, largest_gap=%" GST_TIME_FORMAT,
                    drops->drop_count, drops->drop_rate, GST_TIME_ARGS(drops->largest_drop_gap_ns));
    } else {
        LOG_DEBUG("No frame drops detected in monitoring window");
    }

    g_free(drops);
}

/**
 * Reset monitoring statistics.
 */
void frame_monitor_reset(FrameMonitor *monitor)
{
    if (!monitor) {
        return;
    }

    g_mutex_lock(&monitor->lock);

    /* Clear frame times queue */
    while (!g_queue_is_empty(monitor->frame_times)) {
        guint64 *ts = g_queue_pop_head(monitor->frame_times);
        g_free(ts);
    }

    monitor->total_frames = 0;
    monitor->dropped_frames = 0;
    monitor->last_timestamp = 0;
    monitor->first_frame = TRUE;

    g_mutex_unlock(&monitor->lock);
    LOG_DEBUG("Frame monitor reset");
}

/**
 * Get the size of the current measurement window.
 */
guint64 frame_monitor_get_window_size(FrameMonitor *monitor)
{
    if (!monitor) {
        return 0;
    }

    g_mutex_lock(&monitor->lock);
    guint64 size = (guint64) g_queue_get_length(monitor->frame_times);
    g_mutex_unlock(&monitor->lock);

    return size;
}

/**
 * Check if monitor has sufficient data for analysis.
 */
gboolean frame_monitor_has_sufficient_data(FrameMonitor *monitor)
{
    if (!monitor) {
        return FALSE;
    }

    /* Require at least 30 frames (~0.25 seconds at 120 fps) */
    guint64 window_size = frame_monitor_get_window_size(monitor);
    return window_size >= 30;
}

/**
 * Generate a performance report.
 */
gchar *frame_monitor_generate_report(FrameMonitor *monitor)
{
    if (!monitor) {
        return g_strdup("Error: monitor is NULL\n");
    }

    FrameMonitorStats *stats = frame_monitor_get_stats(monitor);
    FrameDropInfo *drops = frame_monitor_detect_drops(monitor);
    FrameRateValidationResult validation = frame_monitor_validate_framerate(
        monitor, FRAME_MONITOR_TARGET_FPS, FRAME_MONITOR_TOLERANCE_FPS);

    if (!stats || !drops) {
        g_free(stats);
        g_free(drops);
        return g_strdup("Error: failed to gather statistics\n");
    }

    gchar buffer[2048];
    gint len = 0;

    len +=
        g_snprintf(buffer + len, sizeof(buffer) - len, "=== Frame Rate Performance Report ===\n");
    len += g_snprintf(buffer + len, sizeof(buffer) - len, "Target Frame Rate:    %u ± %u fps\n",
                      FRAME_MONITOR_TARGET_FPS, FRAME_MONITOR_TOLERANCE_FPS);
    len += g_snprintf(buffer + len, sizeof(buffer) - len, "Validation Status:    %s\n",
                      frame_monitor_validation_string(validation));
    len += g_snprintf(buffer + len, sizeof(buffer) - len, "\nMeasured Statistics:\n");
    len += g_snprintf(buffer + len, sizeof(buffer) - len, "  Average FPS:        %.2f\n",
                      stats->average_fps);
    len += g_snprintf(buffer + len, sizeof(buffer) - len, "  Current FPS:        %.2f\n",
                      stats->current_fps);
    len += g_snprintf(buffer + len, sizeof(buffer) - len, "  FPS Min/Max:        %.2f / %.2f\n",
                      stats->fps_min, stats->fps_max);
    len += g_snprintf(buffer + len, sizeof(buffer) - len, "  Standard Deviation: %.2f\n",
                      stats->fps_std_dev);
    len += g_snprintf(buffer + len, sizeof(buffer) - len, "\nFrame Count:\n");
    len += g_snprintf(buffer + len, sizeof(buffer) - len,
                      "  Total Frames:       %" G_GUINT64_FORMAT "\n", stats->total_frames);
    len += g_snprintf(buffer + len, sizeof(buffer) - len,
                      "  Dropped Frames:     %" G_GUINT64_FORMAT "\n", stats->dropped_frames);

    if (stats->total_frames > 0) {
        gdouble drop_percent =
            ((gdouble) stats->dropped_frames / (gdouble) stats->total_frames) * 100.0;
        len += g_snprintf(buffer + len, sizeof(buffer) - len, "  Drop Rate:          %.2f%%\n",
                          drop_percent);
    }

    len += g_snprintf(buffer + len, sizeof(buffer) - len, "\nDropped Frame Events:\n");
    len += g_snprintf(buffer + len, sizeof(buffer) - len, "  Detected:           %s\n",
                      drops->has_drops ? "YES" : "NO");
    len += g_snprintf(buffer + len, sizeof(buffer) - len,
                      "  Drop Events:        %" G_GUINT64_FORMAT "\n", drops->drop_count);
    len += g_snprintf(buffer + len, sizeof(buffer) - len, "  Drop Rate:          %.2f%%\n",
                      drops->drop_rate);
    len +=
        g_snprintf(buffer + len, sizeof(buffer) - len,
                   "  Largest Gap:        %" G_GUINT64_FORMAT " ns\n", drops->largest_drop_gap_ns);

    len += g_snprintf(buffer + len, sizeof(buffer) - len,
                      "\nSession Duration:     %" G_GUINT64_FORMAT " ns (%.2f sec)\n",
                      stats->session_duration_ns,
                      (gdouble) stats->session_duration_ns / (gdouble) GST_SECOND);
    len +=
        g_snprintf(buffer + len, sizeof(buffer) - len, "=====================================\n");

    g_free(stats);
    g_free(drops);

    return g_strndup(buffer, len);
}

/**
 * Cleanup and free frame monitor.
 */
void frame_monitor_cleanup(FrameMonitor *monitor)
{
    if (!monitor) {
        return;
    }

    g_mutex_lock(&monitor->lock);

    /* Clear frame times queue */
    while (!g_queue_is_empty(monitor->frame_times)) {
        guint64 *ts = g_queue_pop_head(monitor->frame_times);
        g_free(ts);
    }
    g_queue_free(monitor->frame_times);

    g_mutex_unlock(&monitor->lock);
    g_mutex_clear(&monitor->lock);

    g_free(monitor);
    LOG_DEBUG("Frame monitor cleaned up");
}
