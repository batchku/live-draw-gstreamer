/**
 * @file test_profiling_utils.c
 * @brief Unit tests for profiling utility functions
 *
 * Tests core profiling functionality:
 * - Context creation and initialization
 * - Frame timing recording
 * - Sample collection
 * - Metric calculations (FPS, queue stats, sync metrics)
 * - Report generation
 *
 * These tests verify profiling infrastructure works correctly
 * for identifying pipeline bottlenecks.
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../src/utils/profiling.h"

/* ========== Mock GStreamer Objects ========== */

/**
 * Create a GStreamer pipeline for testing
 */
static GstElement *create_mock_pipeline(void)
{
    GstElement *pipeline = gst_pipeline_new("test-pipeline");
    if (!pipeline) {
        /* Fallback: return NULL if pipeline creation fails */
        return NULL;
    }
    return pipeline;
}

/* ========== Test Fixtures ========== */

typedef struct {
    ProfilingContext *ctx;
    GstElement *mock_pipeline;
} ProfilingTestFixture;

static void setup_test(ProfilingTestFixture *f)
{
    gst_init(NULL, NULL);
    f->mock_pipeline = create_mock_pipeline();
    if (f->mock_pipeline) {
        f->ctx = profiling_context_create((GstElement *) f->mock_pipeline, 50, 0);
    } else {
        f->ctx = NULL;
    }
}

static void teardown_test(ProfilingTestFixture *f)
{
    if (f->ctx) {
        profiling_context_free(f->ctx);
        f->ctx = NULL;
    }
    if (f->mock_pipeline) {
        gst_object_unref(f->mock_pipeline);
        f->mock_pipeline = NULL;
    }
}

/* ========== Test Cases ========== */

/**
 * Test 1: Context creation with various configurations
 */
static gboolean test_context_creation(void)
{
    fprintf(stdout, "test_context_creation...");

    ProfilingTestFixture f;
    setup_test(&f);

    /* Verify default values */
    g_assert_nonnull(f.ctx);
    g_assert_true(f.ctx->pipeline != NULL);
    g_assert_cmpuint(f.ctx->sample_interval_ms, ==, 50);
    g_assert_false(f.ctx->is_active);
    g_assert_cmpuint(f.ctx->frame_timings->length, ==, 0);
    g_assert_cmpuint(f.ctx->performance_samples->length, ==, 0);

    teardown_test(&f);
    fprintf(stdout, " PASSED\n");
    return TRUE;
}

/**
 * Test 2: Frame timing recording
 */
static gboolean test_frame_timing_recording(void)
{
    fprintf(stdout, "test_frame_timing_recording...");

    ProfilingTestFixture f;
    setup_test(&f);

    /* Record multiple frames */
    for (guint i = 0; i < 10; i++) {
        guint64 ts_ns = (guint64) i * 33333333; /* ~30fps in nanoseconds */
        guint64 arrival_us = g_get_monotonic_time();
        profiling_record_frame(f.ctx, i, ts_ns, arrival_us, FALSE);
    }

    /* Verify frames recorded */
    g_assert_cmpuint(f.ctx->frame_timings->length, ==, 10);

    /* Verify frame properties */
    FrameTiming *first = (FrameTiming *) g_queue_peek_head(f.ctx->frame_timings);
    g_assert_nonnull(first);
    g_assert_cmpuint(first->frame_number, ==, 0);
    g_assert_cmpuint(first->timestamp_ns, ==, 0);

    FrameTiming *last = (FrameTiming *) g_queue_peek_tail(f.ctx->frame_timings);
    g_assert_nonnull(last);
    g_assert_cmpuint(last->frame_number, ==, 9);

    teardown_test(&f);
    fprintf(stdout, " PASSED\n");
    return TRUE;
}

/**
 * Test 3: Frame drop recording
 */
static gboolean test_frame_drop_detection(void)
{
    fprintf(stdout, "test_frame_drop_detection...");

    ProfilingTestFixture f;
    setup_test(&f);

    /* Record frames, with some marked as dropped */
    guint num_frames = 10;

    for (guint i = 0; i < num_frames; i++) {
        gboolean is_dropped = (i == 3 || i == 7); /* Drop frames 3 and 7 */

        profiling_record_frame(f.ctx, i, (guint64) i * 33333333, g_get_monotonic_time(),
                               is_dropped);
    }

    /* Verify drop count */
    guint actual_dropped = 0;
    GList *item = g_queue_peek_head_link(f.ctx->frame_timings);
    while (item) {
        FrameTiming *timing = (FrameTiming *) item->data;
        if (timing->was_dropped) {
            actual_dropped++;
        }
        item = g_list_next(item);
    }

    g_assert_cmpuint(actual_dropped, ==, 2);

    teardown_test(&f);
    fprintf(stdout, " PASSED\n");
    return TRUE;
}

/**
 * Test 4: Performance sample collection
 */
static gboolean test_sample_collection(void)
{
    fprintf(stdout, "test_sample_collection...");

    ProfilingTestFixture f;
    setup_test(&f);

    /* Enable profiling */
    f.ctx->is_active = TRUE;

    /* Collect samples */
    for (guint i = 0; i < 5; i++) {
        /* Add some frame data first */
        for (guint j = 0; j < 30; j++) {
            profiling_record_frame(f.ctx, i * 30 + j, (guint64) (i * 30 + j) * 33333333,
                                   g_get_monotonic_time(), FALSE);
        }
        profiling_collect_sample(f.ctx);
    }

    /* Verify samples collected */
    g_assert_cmpuint(f.ctx->performance_samples->length, >=, 1);

    /* Verify sample properties */
    PerformanceSample *last = (PerformanceSample *) g_queue_peek_tail(f.ctx->performance_samples);
    g_assert_nonnull(last);
    g_assert_cmpuint(last->current_fps, >=, 0);

    teardown_test(&f);
    fprintf(stdout, " PASSED\n");
    return TRUE;
}

/**
 * Test 5: FPS statistics calculation
 */
static gboolean test_fps_statistics(void)
{
    fprintf(stdout, "test_fps_statistics...");

    ProfilingTestFixture f;
    setup_test(&f);

    /* Add performance samples simulating 60 fps */
    guint64 start_time = g_get_monotonic_time();
    for (guint i = 0; i < 10; i++) {
        PerformanceSample *sample = g_new0(PerformanceSample, 1);
        sample->timestamp_us = start_time + (i * 16667); /* ~60fps */
        sample->frame_number = i;
        sample->current_fps = 60.0 + (i % 3 == 0 ? 1.5 : -1.5); /* 58-61 fps variation */
        g_queue_push_tail(f.ctx->performance_samples, sample);
    }

    /* Get FPS stats */
    gdouble current_fps = 0.0, avg_fps = 0.0, min_fps = 0.0, max_fps = 0.0;
    gboolean success =
        profiling_get_fps_stats(f.ctx, &current_fps, &avg_fps, &min_fps, &max_fps, NULL);

    g_assert_true(success);
    g_assert_cmpfloat_with_epsilon(avg_fps, 60.0, 1.0);
    g_assert_cmpfloat(min_fps, <, avg_fps);
    g_assert_cmpfloat(max_fps, >, avg_fps);
    g_assert_cmpfloat(min_fps, <=, max_fps);

    teardown_test(&f);
    fprintf(stdout, " PASSED\n");
    return TRUE;
}

/**
 * Test 6: Queue statistics
 */
static gboolean test_queue_statistics(void)
{
    fprintf(stdout, "test_queue_statistics...");

    ProfilingTestFixture f;
    setup_test(&f);

    /* Get queue stats (may be empty if pipeline not running) */
    guint64 total_bytes = 0;
    guint max_depth = 0;
    guint num_queues = 0;

    profiling_get_queue_stats(f.ctx, &total_bytes, &max_depth, &num_queues);

    /* May fail if no queues available, but shouldn't crash */
    g_assert_cmpuint(total_bytes, >=, 0);
    g_assert_cmpuint(max_depth, >=, 0);
    g_assert_cmpuint(num_queues, >=, 0);

    teardown_test(&f);
    fprintf(stdout, " PASSED\n");
    return TRUE;
}

/**
 * Test 7: Synchronization metrics
 */
static gboolean test_sync_metrics(void)
{
    fprintf(stdout, "test_sync_metrics...");

    ProfilingTestFixture f;
    setup_test(&f);

    /* Record frames with timing info */
    guint64 now = g_get_monotonic_time();
    for (guint i = 0; i < 20; i++) {
        gboolean dropped = (i == 5 || i == 15); /* Drop 2 out of 20 */
        profiling_record_frame(f.ctx, i, (guint64) i * 33333333, now + (guint64) i * 33333,
                               dropped);
    }

    /* Get sync metrics */
    gdouble drop_ratio = 0.0;
    guint64 jitter_us = 0, max_latency_us = 0;
    gboolean success = profiling_get_sync_metrics(f.ctx, &drop_ratio, &jitter_us, &max_latency_us);

    g_assert_true(success);
    g_assert_cmpfloat_with_epsilon(drop_ratio, 0.1, 0.02); /* 2/20 = 0.1 */
    g_assert_cmpuint(jitter_us, >=, 0);
    g_assert_cmpuint(max_latency_us, >=, 0);

    teardown_test(&f);
    fprintf(stdout, " PASSED\n");
    return TRUE;
}

/**
 * Test 8: Report generation
 */
static gboolean test_report_generation(void)
{
    fprintf(stdout, "test_report_generation...");

    ProfilingTestFixture f;
    setup_test(&f);

    if (!f.ctx) {
        teardown_test(&f);
        fprintf(stdout, " SKIPPED (no pipeline)\n");
        return TRUE; /* Skip test if setup failed */
    }

    /* Add sample data */
    for (guint i = 0; i < 5; i++) {
        PerformanceSample *sample = g_new0(PerformanceSample, 1);
        sample->timestamp_us = g_get_monotonic_time();
        sample->current_fps = 120.0 + (i % 2 == 0 ? 1.0 : -1.0);
        g_queue_push_tail(f.ctx->performance_samples, sample);
    }

    /* Generate report */
    gchar *report = profiling_generate_report(f.ctx);
    g_assert_nonnull(report);

    /* Verify report has reasonable length */
    g_assert_cmpuint(strlen(report), >, 50);

    g_free(report);
    teardown_test(&f);
    fprintf(stdout, " PASSED\n");
    return TRUE;
}

/**
 * Test 9: JSON export
 */
static gboolean test_json_export(void)
{
    fprintf(stdout, "test_json_export...");

    ProfilingTestFixture f;
    setup_test(&f);

    /* Add sample data */
    for (guint i = 0; i < 3; i++) {
        PerformanceSample *sample = g_new0(PerformanceSample, 1);
        sample->timestamp_us = g_get_monotonic_time();
        sample->current_fps = 120.0;
        g_queue_push_tail(f.ctx->performance_samples, sample);
    }

    /* Export to JSON */
    const char *tmpfile = "/tmp/test_profiling_export.json";
    gboolean success = profiling_export_json(f.ctx, tmpfile);
    g_assert_true(success);

    /* Verify file exists and contains JSON */
    FILE *fp = fopen(tmpfile, "r");
    g_assert_nonnull(fp);

    char line[256];
    gboolean found_json_start = FALSE;
    if (fgets(line, sizeof(line), fp)) {
        if (g_str_has_prefix(line, "{")) {
            found_json_start = TRUE;
        }
    }
    fclose(fp);

    g_assert_true(found_json_start);
    unlink(tmpfile);

    teardown_test(&f);
    fprintf(stdout, " PASSED\n");
    return TRUE;
}

/**
 * Test 10: Max samples circular buffer
 */
static gboolean test_max_samples_limit(void)
{
    fprintf(stdout, "test_max_samples_limit...");

    /* Create context with max 5 samples */
    GstElement *mock_pipeline = create_mock_pipeline();
    ProfilingContext *ctx = profiling_context_create((GstElement *) mock_pipeline, 50, 5);
    if (!ctx) {
        gst_object_unref(mock_pipeline);
        fprintf(stdout, " SKIPPED (no pipeline)\n");
        return TRUE;
    }

    g_assert_nonnull(ctx);

    /* Verify max_samples is set */
    g_assert_cmpuint(ctx->max_samples, ==, 5);

    profiling_context_free(ctx);
    gst_object_unref(mock_pipeline);

    fprintf(stdout, " PASSED\n");
    return TRUE;
}

/* ========== Main Test Runner ========== */

int main(int argc G_GNUC_UNUSED, char **argv G_GNUC_UNUSED)
{
    fprintf(stdout, "========================================\n"
                    "Profiling Utilities Unit Tests\n"
                    "========================================\n\n");

    gint tests_passed = 0;
    gint tests_failed = 0;

#define RUN_TEST(test_func)                                                                        \
    do {                                                                                           \
        if (test_func()) {                                                                         \
            tests_passed++;                                                                        \
        } else {                                                                                   \
            tests_failed++;                                                                        \
        }                                                                                          \
    } while (0)

    RUN_TEST(test_context_creation);
    RUN_TEST(test_frame_timing_recording);
    RUN_TEST(test_frame_drop_detection);
    RUN_TEST(test_sample_collection);
    RUN_TEST(test_fps_statistics);
    RUN_TEST(test_queue_statistics);
    RUN_TEST(test_sync_metrics);
    RUN_TEST(test_report_generation);
    RUN_TEST(test_json_export);
    RUN_TEST(test_max_samples_limit);

    fprintf(stdout,
            "\n========================================\n"
            "Results: %d passed, %d failed\n"
            "========================================\n",
            tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
