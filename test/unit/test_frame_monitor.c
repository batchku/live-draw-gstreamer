#include "utils/frame_monitor.h"
#include "utils/logging.h"
#include <glib.h>
#include <gst/gst.h>

/**
 * Unit tests for frame_monitor module.
 *
 * Tests frame rate measurement, drop detection, validation,
 * and reporting functionality.
 */

/**
 * Test fixture setup
 */
static void setup(void)
{
    gst_init(NULL, NULL);
    logging_set_level(LOG_LEVEL_DEBUG);
}

/**
 * Test fixture teardown
 */
static void teardown(void)
{
    gst_deinit();
}

/**
 * Test: Create and cleanup monitor
 */
static void test_frame_monitor_create_cleanup(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    g_assert_cmpuint(frame_monitor_get_window_size(monitor), ==, 0);
    g_assert_false(frame_monitor_has_sufficient_data(monitor));

    frame_monitor_cleanup(monitor);
}

/**
 * Test: Record single frame
 */
static void test_frame_monitor_single_frame(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    gboolean result = frame_monitor_on_frame(monitor, 1000000000); /* 1 second in ns */
    g_assert_true(result);
    g_assert_cmpuint(frame_monitor_get_window_size(monitor), ==, 1);

    frame_monitor_cleanup(monitor);
}

/**
 * Test: Record sequence of frames at 120 fps
 */
static void test_frame_monitor_perfect_120fps(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    /* Simulate 120 fps for 1 second = 120 frames */
    GstClockTime interval_ns = GST_SECOND / 120;
    GstClockTime current_time = 0;

    for (guint i = 0; i < 120; i++) {
        gboolean result = frame_monitor_on_frame(monitor, current_time);
        g_assert_true(result);
        current_time += interval_ns;
    }

    g_assert_true(frame_monitor_get_window_size(monitor) <= FRAME_MONITOR_WINDOW_SIZE);

    /* Check statistics */
    FrameMonitorStats *stats = frame_monitor_get_stats(monitor);
    g_assert_nonnull(stats);
    g_assert_cmpuint(stats->total_frames, ==, 120);
    g_assert_cmpuint(stats->dropped_frames, ==, 0);

    /* Should be very close to 120 fps */
    g_assert_cmpfloat_with_epsilon(stats->average_fps, 120.0, 2.0);

    g_free(stats);
    frame_monitor_cleanup(monitor);
}

/**
 * Test: Detect dropped frame
 */
static void test_frame_monitor_detect_single_drop(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    GstClockTime interval_ns = GST_SECOND / 120; /* Expected ~8.33ms */
    GstClockTime current_time = 0;

    /* Record 50 normal frames */
    for (guint i = 0; i < 50; i++) {
        frame_monitor_on_frame(monitor, current_time);
        current_time += interval_ns;
    }

    /* Create a large gap (simulate one dropped frame: 2x expected interval) */
    current_time += interval_ns * 2;
    frame_monitor_on_frame(monitor, current_time);

    /* Record 50 more normal frames */
    for (guint i = 0; i < 50; i++) {
        current_time += interval_ns;
        frame_monitor_on_frame(monitor, current_time);
    }

    /* Check that drops were detected */
    FrameDropInfo *drops = frame_monitor_detect_drops(monitor);
    g_assert_nonnull(drops);
    g_assert_true(drops->has_drops);
    g_assert_cmpuint(drops->drop_count, >, 0);
    g_assert_cmpfloat(drops->drop_rate, >, 0.0);

    g_free(drops);
    frame_monitor_cleanup(monitor);
}

/**
 * Test: Validate perfect 120 fps
 */
static void test_frame_monitor_validate_perfect(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    GstClockTime interval_ns = GST_SECOND / 120;
    GstClockTime current_time = 0;

    /* Record 120 frames at perfect 120 fps */
    for (guint i = 0; i < 120; i++) {
        frame_monitor_on_frame(monitor, current_time);
        current_time += interval_ns;
    }

    FrameRateValidationResult result = frame_monitor_validate_framerate(monitor, 120, 2);

    g_assert_cmpint(result, ==, FRAME_RATE_VALID);

    frame_monitor_cleanup(monitor);
}

/**
 * Test: Validate low frame rate
 */
static void test_frame_monitor_validate_low(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    GstClockTime interval_ns = GST_SECOND / 100; /* 100 fps instead of 120 */
    GstClockTime current_time = 0;

    /* Record 100 frames at 100 fps */
    for (guint i = 0; i < 100; i++) {
        frame_monitor_on_frame(monitor, current_time);
        current_time += interval_ns;
    }

    FrameRateValidationResult result = frame_monitor_validate_framerate(monitor, 120, 2);

    g_assert_cmpint(result, ==, FRAME_RATE_LOW);

    frame_monitor_cleanup(monitor);
}

/**
 * Test: Validate high frame rate
 */
static void test_frame_monitor_validate_high(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    GstClockTime interval_ns = GST_SECOND / 144; /* 144 fps instead of 120 */
    GstClockTime current_time = 0;

    /* Record 144 frames at 144 fps */
    for (guint i = 0; i < 144; i++) {
        frame_monitor_on_frame(monitor, current_time);
        current_time += interval_ns;
    }

    FrameRateValidationResult result = frame_monitor_validate_framerate(monitor, 120, 2);

    g_assert_cmpint(result, ==, FRAME_RATE_HIGH);

    frame_monitor_cleanup(monitor);
}

/**
 * Test: Insufficient data validation
 */
static void test_frame_monitor_insufficient_data(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    /* Record only 5 frames */
    GstClockTime interval_ns = GST_SECOND / 120;
    GstClockTime current_time = 0;

    for (guint i = 0; i < 5; i++) {
        frame_monitor_on_frame(monitor, current_time);
        current_time += interval_ns;
    }

    FrameRateValidationResult result = frame_monitor_validate_framerate(monitor, 120, 2);

    g_assert_cmpint(result, ==, FRAME_RATE_INSUFFICIENT_DATA);

    frame_monitor_cleanup(monitor);
}

/**
 * Test: Reset monitor
 */
static void test_frame_monitor_reset(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    GstClockTime interval_ns = GST_SECOND / 120;
    GstClockTime current_time = 0;

    /* Record some frames */
    for (guint i = 0; i < 60; i++) {
        frame_monitor_on_frame(monitor, current_time);
        current_time += interval_ns;
    }

    g_assert_true(frame_monitor_get_window_size(monitor) > 0);

    /* Reset */
    frame_monitor_reset(monitor);

    g_assert_cmpuint(frame_monitor_get_window_size(monitor), ==, 0);
    g_assert_false(frame_monitor_has_sufficient_data(monitor));

    frame_monitor_cleanup(monitor);
}

/**
 * Test: Generate report
 */
static void test_frame_monitor_generate_report(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    GstClockTime interval_ns = GST_SECOND / 120;
    GstClockTime current_time = 0;

    /* Record 120 frames */
    for (guint i = 0; i < 120; i++) {
        frame_monitor_on_frame(monitor, current_time);
        current_time += interval_ns;
    }

    /* Generate report */
    gchar *report = frame_monitor_generate_report(monitor);
    g_assert_nonnull(report);

    /* Verify report contains expected content */
    g_assert_nonnull(g_strstr_len(report, -1, "Frame Rate Performance Report"));
    g_assert_nonnull(g_strstr_len(report, -1, "Total Frames"));
    g_assert_nonnull(g_strstr_len(report, -1, "Validation Status"));

    g_free(report);
    frame_monitor_cleanup(monitor);
}

/**
 * Test: Window size limitation
 */
static void test_frame_monitor_window_limit(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    GstClockTime interval_ns = GST_SECOND / 120;
    GstClockTime current_time = 0;

    /* Record more frames than window size */
    for (guint i = 0; i < FRAME_MONITOR_WINDOW_SIZE + 100; i++) {
        frame_monitor_on_frame(monitor, current_time);
        current_time += interval_ns;
    }

    /* Window should be capped at max size */
    guint64 window_size = frame_monitor_get_window_size(monitor);
    g_assert_cmpuint64(window_size, <=, FRAME_MONITOR_WINDOW_SIZE);

    frame_monitor_cleanup(monitor);
}

/**
 * Test: Unstable frame rate detection
 */
static void test_frame_monitor_unstable_fps(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    GstClockTime current_time = 0;
    GstClockTime base_interval = GST_SECOND / 120;

    /* Create unstable frame rate with large variations */
    for (guint i = 0; i < 100; i++) {
        frame_monitor_on_frame(monitor, current_time);

        /* Alternate between slow and fast frames */
        if (i % 2 == 0) {
            current_time += base_interval * 2; /* 60 fps */
        } else {
            current_time += base_interval / 2; /* 240 fps */
        }
    }

    FrameRateValidationResult result = frame_monitor_validate_framerate(monitor, 120, 2);

    /* Should detect unstable fps due to high std deviation */
    g_assert_cmpint(result, ==, FRAME_RATE_UNSTABLE);

    frame_monitor_cleanup(monitor);
}

/**
 * Test: Has sufficient data check
 */
static void test_frame_monitor_sufficient_data(void)
{
    FrameMonitor *monitor = frame_monitor_create();
    g_assert_nonnull(monitor);

    g_assert_false(frame_monitor_has_sufficient_data(monitor));

    GstClockTime interval_ns = GST_SECOND / 120;
    GstClockTime current_time = 0;

    /* Add frames until sufficient */
    for (guint i = 0; i < 31; i++) {
        frame_monitor_on_frame(monitor, current_time);
        current_time += interval_ns;
    }

    g_assert_true(frame_monitor_has_sufficient_data(monitor));

    frame_monitor_cleanup(monitor);
}

/**
 * Main test runner
 */
int main(int argc, char **argv)
{
    setup();

    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/FrameMonitor/CreateCleanup", test_frame_monitor_create_cleanup);
    g_test_add_func("/FrameMonitor/SingleFrame", test_frame_monitor_single_frame);
    g_test_add_func("/FrameMonitor/Perfect120fps", test_frame_monitor_perfect_120fps);
    g_test_add_func("/FrameMonitor/DetectDrop", test_frame_monitor_detect_single_drop);
    g_test_add_func("/FrameMonitor/ValidatePerfect", test_frame_monitor_validate_perfect);
    g_test_add_func("/FrameMonitor/ValidateLow", test_frame_monitor_validate_low);
    g_test_add_func("/FrameMonitor/ValidateHigh", test_frame_monitor_validate_high);
    g_test_add_func("/FrameMonitor/InsufficientData", test_frame_monitor_insufficient_data);
    g_test_add_func("/FrameMonitor/Reset", test_frame_monitor_reset);
    g_test_add_func("/FrameMonitor/GenerateReport", test_frame_monitor_generate_report);
    g_test_add_func("/FrameMonitor/WindowLimit", test_frame_monitor_window_limit);
    g_test_add_func("/FrameMonitor/UnstableFPS", test_frame_monitor_unstable_fps);
    g_test_add_func("/FrameMonitor/SufficientData", test_frame_monitor_sufficient_data);

    int result = g_test_run();

    teardown();
    return result;
}
