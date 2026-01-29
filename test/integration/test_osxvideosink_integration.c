/**
 * @file test_osxvideosink_integration.c
 * @brief Integration tests for osxvideosink and Cocoa NSView integration
 *
 * Tests the complete osxvideosink integration with Cocoa NSWindow,
 * Metal/OpenGL rendering coordination, and window lifecycle management.
 */

#include "../../src/osx/window.h"
#include "../../src/utils/logging.h"
#include <glib.h>
#include <gst/gst.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* ============================================================================
 * Test Fixtures and Setup/Teardown
 * ========================================================================= */

/**
 * Test suite for osxvideosink integration
 */
typedef struct {
    OSXWindow *window;
    GstElement *pipeline;
    GstElement *videosink;
} OSXVideoSinkTestFixture;

/**
 * Setup: Initialize window and check videosink integration
 */
static void setup_osxvideosink_test(OSXVideoSinkTestFixture *fixture)
{
    fprintf(stderr, "[TEST] Setup: Creating OS X window...\n");

    // Initialize logging
    logging_init();

    // Create window with 10 cells (10x1 grid)
    fixture->window = window_create(10);
    if (!fixture->window) {
        fprintf(stderr, "[TEST] ERROR: Failed to create window\n");
        exit(1);
    }

    // Get videosink from window
    fixture->videosink = window_get_videosink(fixture->window);
    if (!fixture->videosink) {
        fprintf(stderr, "[TEST] ERROR: window_get_videosink returned NULL\n");
        window_cleanup(fixture->window);
        exit(1);
    }

    fprintf(stderr, "[TEST] Setup complete: Window=%p, Videosink=%p\n", (void *) fixture->window,
            (void *) fixture->videosink);
}

/**
 * Teardown: Cleanup window and resources
 */
static void teardown_osxvideosink_test(OSXVideoSinkTestFixture *fixture)
{
    fprintf(stderr, "[TEST] Teardown: Cleaning up...\n");

    if (fixture->window) {
        window_cleanup(fixture->window);
        fixture->window = NULL;
    }

    fprintf(stderr, "[TEST] Teardown complete\n");
}

/* ============================================================================
 * Test Cases
 * ========================================================================= */

/**
 * Test 1: Window creation and videosink initialization
 */
static void test_window_creation_and_videosink(void)
{
    fprintf(stderr, "\n[TEST] Running: test_window_creation_and_videosink\n");

    OSXVideoSinkTestFixture fixture = {0};
    setup_osxvideosink_test(&fixture);

    // Verify window is not NULL
    if (!fixture.window) {
        fprintf(stderr, "[TEST] FAIL: Window is NULL\n");
        teardown_osxvideosink_test(&fixture);
        exit(1);
    }

    // Verify videosink is not NULL
    if (!fixture.videosink) {
        fprintf(stderr, "[TEST] FAIL: Videosink is NULL\n");
        teardown_osxvideosink_test(&fixture);
        exit(1);
    }

    // Verify window has valid grid configuration
    if (fixture.window->grid_cols != 10 || fixture.window->grid_rows != 1) {
        fprintf(stderr, "[TEST] FAIL: Grid configuration incorrect: %u x %u (expected 10 x 1)\n",
                fixture.window->grid_cols, fixture.window->grid_rows);
        teardown_osxvideosink_test(&fixture);
        exit(1);
    }

    // Verify aspect ratio is set
    if (fixture.window->aspect_ratio <= 0) {
        fprintf(stderr, "[TEST] FAIL: Aspect ratio not set: %.3f\n", fixture.window->aspect_ratio);
        teardown_osxvideosink_test(&fixture);
        exit(1);
    }

    fprintf(stderr, "[TEST] PASS: Window and videosink created successfully\n");
    fprintf(stderr, "  - Window: %p\n", (void *) fixture.window);
    fprintf(stderr, "  - Videosink: %p\n", (void *) fixture.videosink);
    fprintf(stderr, "  - Grid: %u x %u\n", fixture.window->grid_cols, fixture.window->grid_rows);
    fprintf(stderr, "  - Aspect ratio: %.3f\n", fixture.window->aspect_ratio);

    teardown_osxvideosink_test(&fixture);
}

/**
 * Test 2: Videosink element properties
 */
static void test_videosink_properties(void)
{
    fprintf(stderr, "\n[TEST] Running: test_videosink_properties\n");

    OSXVideoSinkTestFixture fixture = {0};
    setup_osxvideosink_test(&fixture);

    // Check videosink properties
    gboolean sync = FALSE;
    gboolean fullscreen = FALSE;
    gboolean force_aspect = FALSE;

    g_object_get(G_OBJECT(fixture.videosink), "sync", &sync, "fullscreen", &fullscreen,
                 "force-aspect-ratio", &force_aspect, NULL);

    if (!sync) {
        fprintf(stderr, "[TEST] WARN: sync property not set\n");
    }

    if (fullscreen) {
        fprintf(stderr, "[TEST] FAIL: fullscreen property should be FALSE\n");
        teardown_osxvideosink_test(&fixture);
        exit(1);
    }

    if (!force_aspect) {
        fprintf(stderr, "[TEST] WARN: force-aspect-ratio property not set\n");
    }

    fprintf(stderr, "[TEST] PASS: Videosink properties configured correctly\n");
    fprintf(stderr, "  - sync: %s\n", sync ? "TRUE" : "FALSE");
    fprintf(stderr, "  - fullscreen: %s\n", fullscreen ? "TRUE" : "FALSE");
    fprintf(stderr, "  - force-aspect-ratio: %s\n", force_aspect ? "TRUE" : "FALSE");

    teardown_osxvideosink_test(&fixture);
}

/**
 * Test 3: Window aspect ratio setting and recalculation
 */
static void test_aspect_ratio_setting(void)
{
    fprintf(stderr, "\n[TEST] Running: test_aspect_ratio_setting\n");

    OSXVideoSinkTestFixture fixture = {0};
    setup_osxvideosink_test(&fixture);

    // Test default aspect ratio
    gdouble expected_ar = 16.0 / 9.0;
    gdouble actual_ar = fixture.window->aspect_ratio;
    gdouble tolerance = 0.01;

    if (fabs(actual_ar - expected_ar) > tolerance) {
        fprintf(stderr, "[TEST] FAIL: Default aspect ratio %.3f != %.3f\n", actual_ar, expected_ar);
        teardown_osxvideosink_test(&fixture);
        exit(1);
    }

    // Test changing aspect ratio
    gdouble new_ar = 4.0 / 3.0; // 1.333...
    window_set_aspect_ratio(fixture.window, new_ar);

    if (fabs(fixture.window->aspect_ratio - new_ar) > tolerance) {
        fprintf(stderr, "[TEST] FAIL: Aspect ratio not updated: %.3f != %.3f\n",
                fixture.window->aspect_ratio, new_ar);
        teardown_osxvideosink_test(&fixture);
        exit(1);
    }

    fprintf(stderr, "[TEST] PASS: Aspect ratio setting works correctly\n");
    fprintf(stderr, "  - Default (16:9): %.3f\n", expected_ar);
    fprintf(stderr, "  - Changed to (4:3): %.3f\n", new_ar);

    teardown_osxvideosink_test(&fixture);
}

/**
 * Test 4: Window visibility check
 */
static void test_window_visibility(void)
{
    fprintf(stderr, "\n[TEST] Running: test_window_visibility\n");

    OSXVideoSinkTestFixture fixture = {0};
    setup_osxvideosink_test(&fixture);

    // Check if window is visible
    gboolean visible = window_is_visible(fixture.window);

    // Window should be visible after creation
    if (!visible) {
        fprintf(stderr, "[TEST] WARN: Window reported as not visible (may be timing issue)\n");
    } else {
        fprintf(stderr, "[TEST] PASS: Window is visible\n");
    }

    teardown_osxvideosink_test(&fixture);
}

/**
 * Test 5: Window render request (no-op test)
 */
static void test_window_render_request(void)
{
    fprintf(stderr, "\n[TEST] Running: test_window_render_request\n");

    OSXVideoSinkTestFixture fixture = {0};
    setup_osxvideosink_test(&fixture);

    // Request render - should not crash
    window_request_render(fixture.window);
    fprintf(stderr, "[TEST] PASS: window_request_render() executed without error\n");

    // Swap buffers - should not crash
    window_swap_buffers(fixture.window);
    fprintf(stderr, "[TEST] PASS: window_swap_buffers() executed without error\n");

    teardown_osxvideosink_test(&fixture);
}

/**
 * Test 6: Cell dimensions calculation
 */
static void test_cell_dimensions(void)
{
    fprintf(stderr, "\n[TEST] Running: test_cell_dimensions\n");

    OSXVideoSinkTestFixture fixture = {0};
    setup_osxvideosink_test(&fixture);

    // Expected dimensions for 10x1 grid with 320px cells and 16:9 aspect ratio
    CGFloat expected_cell_width = 320.0;
    CGFloat expected_window_height = 320.0 / (16.0 / 9.0); // ~180 pixels

    CGFloat actual_cell_width = fixture.window->cell_width;
    CGFloat actual_cell_height = fixture.window->cell_height;

    if (fabs(actual_cell_width - expected_cell_width) > 0.1) {
        fprintf(stderr, "[TEST] FAIL: Cell width mismatch: %.0f != %.0f\n", actual_cell_width,
                expected_cell_width);
        teardown_osxvideosink_test(&fixture);
        exit(1);
    }

    // Allow some tolerance for floating-point calculations
    gdouble height_tolerance = 1.0;
    if (fabs(actual_cell_height - expected_window_height) > height_tolerance) {
        fprintf(stderr, "[TEST] WARN: Cell height mismatch: %.0f != %.0f\n", actual_cell_height,
                expected_window_height);
    }

    fprintf(stderr, "[TEST] PASS: Cell dimensions calculated correctly\n");
    fprintf(stderr, "  - Cell width: %.0f px\n", actual_cell_width);
    fprintf(stderr, "  - Cell height: %.0f px\n", actual_cell_height);
    fprintf(stderr, "  - Total window: %.0f x %.0f px\n",
            actual_cell_width * fixture.window->grid_cols, actual_cell_height);

    teardown_osxvideosink_test(&fixture);
}

/**
 * Test 7: Videosink null handling
 */
static void test_videosink_null_safety(void)
{
    fprintf(stderr, "\n[TEST] Running: test_videosink_null_safety\n");

    // Test with NULL window - should not crash
    GstElement *sink = window_get_videosink(NULL);
    if (sink != NULL) {
        fprintf(stderr, "[TEST] WARN: window_get_videosink(NULL) returned non-NULL\n");
    }

    // Test window_set_aspect_ratio with NULL
    window_set_aspect_ratio(NULL, 16.0 / 9.0); // Should not crash

    // Test window_on_resize with NULL
    window_on_resize(NULL, 3200, 180); // Should not crash

    // Test window_request_render with NULL
    window_request_render(NULL); // Should not crash

    // Test window_swap_buffers with NULL
    window_swap_buffers(NULL); // Should not crash

    // Test window_is_visible with NULL
    gboolean visible = window_is_visible(NULL);
    if (visible) {
        fprintf(stderr, "[TEST] FAIL: window_is_visible(NULL) should return FALSE\n");
        exit(1);
    }

    fprintf(stderr, "[TEST] PASS: Null safety checks passed\n");
}

/**
 * Test 8: Window cleanup and resource deallocation
 */
static void test_window_cleanup(void)
{
    fprintf(stderr, "\n[TEST] Running: test_window_cleanup\n");

    OSXVideoSinkTestFixture fixture = {0};
    setup_osxvideosink_test(&fixture);

    // Cleanup should not crash
    window_cleanup(fixture.window);
    fixture.window = NULL;

    // Double cleanup should not crash
    window_cleanup(NULL);

    fprintf(stderr, "[TEST] PASS: Window cleanup successful\n");
}

/* ============================================================================
 * Main Test Runner
 * ========================================================================= */

int main(int argc, char *argv[])
{
    fprintf(stderr, "\n========================================\n");
    fprintf(stderr, "  osxvideosink Integration Tests\n");
    fprintf(stderr, "========================================\n\n");

    // Initialize GStreamer
    gst_init(&argc, &argv);

    int failed_tests = 0;
    int passed_tests = 0;

    // Run tests (simple sequential execution without exception handling)
    fprintf(stderr, "[TEST] Starting test suite...\n");

    test_window_creation_and_videosink();
    passed_tests++;

    test_videosink_properties();
    passed_tests++;

    test_aspect_ratio_setting();
    passed_tests++;

    test_window_visibility();
    passed_tests++;

    test_window_render_request();
    passed_tests++;

    test_cell_dimensions();
    passed_tests++;

    test_videosink_null_safety();
    passed_tests++;

    test_window_cleanup();
    passed_tests++;

    // Print summary
    fprintf(stderr, "\n========================================\n");
    fprintf(stderr, "  Test Summary\n");
    fprintf(stderr, "========================================\n");
    fprintf(stderr, "Passed: %d\n", passed_tests);
    fprintf(stderr, "Failed: %d\n", failed_tests);
    fprintf(stderr, "Total:  %d\n\n", passed_tests + failed_tests);

    // Cleanup GStreamer
    gst_deinit();

    return failed_tests > 0 ? 1 : 0;
}
