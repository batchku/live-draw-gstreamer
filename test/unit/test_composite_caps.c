#include <gst/gst.h>
#include <stdio.h>
#include <string.h>

// Include the module we're testing
#include "gstreamer/composite_caps.h"

/**
 * Test 1: Basic composite_caps configuration with valid parameters
 *
 * This test verifies that the composite_caps element can be properly
 * configured with standard grid dimensions and frame rate.
 *
 * Expected: Function returns TRUE and caps are set
 */
static void test_composite_caps_valid_config(void)
{
    fprintf(stdout, "[TEST] test_composite_caps_valid_config\n");

    // Create a capsfilter element
    GstElement *caps_elem = gst_element_factory_make("capsfilter", "test-caps");
    if (!caps_elem) {
        fprintf(stderr, "FAIL: Failed to create capsfilter element\n");
        return;
    }

    // Configure with standard grid parameters
    gboolean result = composite_caps_configure(caps_elem,
                                               3200, // grid_width (10 cells × 320px)
                                               1080, // grid_height (aspect ratio-dependent)
                                               120,  // framerate_num (120 fps)
                                               1);   // framerate_den

    if (result != TRUE) {
        fprintf(stderr, "FAIL: composite_caps_configure returned FALSE\n");
        gst_object_unref(caps_elem);
        return;
    }

    // Verify that caps were set on the element
    GstCaps *caps = NULL;
    g_object_get(caps_elem, "caps", &caps, NULL);
    if (!caps) {
        fprintf(stderr, "FAIL: No caps were set on capsfilter\n");
        gst_object_unref(caps_elem);
        return;
    }

    // Verify caps contain our expected format
    gchar *caps_str = gst_caps_to_string(caps);
    if (!caps_str || strlen(caps_str) == 0) {
        fprintf(stderr, "FAIL: Caps string is empty\n");
        if (caps_str)
            g_free(caps_str);
        gst_caps_unref(caps);
        gst_object_unref(caps_elem);
        return;
    }

    fprintf(stdout, "PASS: Caps configured: %s\n", caps_str);

    // Verify caps contain required fields
    if (!strstr(caps_str, "width") || !strstr(caps_str, "height") ||
        !strstr(caps_str, "framerate")) {
        fprintf(stderr, "FAIL: Caps missing required fields (width, height, framerate)\n");
        g_free(caps_str);
        gst_caps_unref(caps);
        gst_object_unref(caps_elem);
        return;
    }

    fprintf(stdout, "PASS: Caps contain all required fields\n");

    g_free(caps_str);
    gst_caps_unref(caps);
    gst_object_unref(caps_elem);
}

/**
 * Test 2: composite_caps with NULL element should return FALSE
 *
 * This test verifies error handling when NULL is passed as the element.
 *
 * Expected: Function returns FALSE without crashing
 */
static void test_composite_caps_null_element(void)
{
    fprintf(stdout, "[TEST] test_composite_caps_null_element\n");

    // Call with NULL element
    gboolean result = composite_caps_configure(NULL, 3200, 1080, 120, 1);

    if (result != FALSE) {
        fprintf(stderr, "FAIL: Function should return FALSE for NULL element\n");
        return;
    }

    fprintf(stdout, "PASS: Correctly returned FALSE for NULL element\n");
}

/**
 * Test 3: composite_caps with invalid width
 *
 * This test verifies error handling for invalid grid dimensions.
 *
 * Expected: Function returns FALSE
 */
static void test_composite_caps_invalid_width(void)
{
    fprintf(stdout, "[TEST] test_composite_caps_invalid_width\n");

    // Initialize for this test
    static gboolean gst_initialized = FALSE;
    if (!gst_initialized) {
        gst_init(NULL, NULL);
        gst_initialized = TRUE;
    }

    GstElement *caps_elem = gst_element_factory_make("capsfilter", "test-caps");
    if (!caps_elem) {
        fprintf(stderr, "FAIL: Failed to create capsfilter element\n");
        return;
    }

    // Call with invalid width (0)
    gboolean result = composite_caps_configure(caps_elem, 0, 1080, 120, 1);

    if (result != FALSE) {
        fprintf(stderr, "FAIL: Function should return FALSE for width=0\n");
        gst_object_unref(caps_elem);
        return;
    }

    fprintf(stdout, "PASS: Correctly returned FALSE for invalid width\n");

    // Also test negative width
    result = composite_caps_configure(caps_elem, -100, 1080, 120, 1);
    if (result != FALSE) {
        fprintf(stderr, "FAIL: Function should return FALSE for negative width\n");
        gst_object_unref(caps_elem);
        return;
    }

    fprintf(stdout, "PASS: Correctly returned FALSE for negative width\n");
    gst_object_unref(caps_elem);
}

/**
 * Test 4: composite_caps with invalid height
 *
 * This test verifies error handling for invalid height.
 *
 * Expected: Function returns FALSE
 */
static void test_composite_caps_invalid_height(void)
{
    fprintf(stdout, "[TEST] test_composite_caps_invalid_height\n");

    GstElement *caps_elem = gst_element_factory_make("capsfilter", "test-caps");
    if (!caps_elem) {
        fprintf(stderr, "FAIL: Failed to create capsfilter element\n");
        return;
    }

    // Call with invalid height (0)
    gboolean result = composite_caps_configure(caps_elem, 3200, 0, 120, 1);

    if (result != FALSE) {
        fprintf(stderr, "FAIL: Function should return FALSE for height=0\n");
        gst_object_unref(caps_elem);
        return;
    }

    fprintf(stdout, "PASS: Correctly returned FALSE for invalid height\n");
    gst_object_unref(caps_elem);
}

/**
 * Test 5: composite_caps with invalid frame rate
 *
 * This test verifies error handling for invalid frame rate parameters.
 *
 * Expected: Function returns FALSE
 */
static void test_composite_caps_invalid_framerate(void)
{
    fprintf(stdout, "[TEST] test_composite_caps_invalid_framerate\n");

    GstElement *caps_elem = gst_element_factory_make("capsfilter", "test-caps");
    if (!caps_elem) {
        fprintf(stderr, "FAIL: Failed to create capsfilter element\n");
        return;
    }

    // Test with 0 numerator
    gboolean result = composite_caps_configure(caps_elem, 3200, 1080, 0, 1);
    if (result != FALSE) {
        fprintf(stderr, "FAIL: Function should return FALSE for framerate_num=0\n");
        gst_object_unref(caps_elem);
        return;
    }

    fprintf(stdout, "PASS: Correctly returned FALSE for framerate_num=0\n");

    // Test with 0 denominator
    result = composite_caps_configure(caps_elem, 3200, 1080, 120, 0);
    if (result != FALSE) {
        fprintf(stderr, "FAIL: Function should return FALSE for framerate_den=0\n");
        gst_object_unref(caps_elem);
        return;
    }

    fprintf(stdout, "PASS: Correctly returned FALSE for framerate_den=0\n");

    // Test with negative numerator
    result = composite_caps_configure(caps_elem, 3200, 1080, -120, 1);
    if (result != FALSE) {
        fprintf(stderr, "FAIL: Function should return FALSE for negative framerate_num\n");
        gst_object_unref(caps_elem);
        return;
    }

    fprintf(stdout, "PASS: Correctly returned FALSE for negative framerate_num\n");
    gst_object_unref(caps_elem);
}

/**
 * Test 6: Verify BGRx format in caps
 *
 * This test verifies that the primary format in the caps is BGRx,
 * which is optimal for macOS Metal/OpenGL rendering.
 *
 * Expected: Caps contain BGRx as the first format
 */
static void test_composite_caps_bgrx_format(void)
{
    fprintf(stdout, "[TEST] test_composite_caps_bgrx_format\n");

    GstElement *caps_elem = gst_element_factory_make("capsfilter", "test-caps");
    if (!caps_elem) {
        fprintf(stderr, "FAIL: Failed to create capsfilter element\n");
        return;
    }

    gboolean result = composite_caps_configure(caps_elem, 3200, 1080, 120, 1);
    if (result != TRUE) {
        fprintf(stderr, "FAIL: composite_caps_configure returned FALSE\n");
        gst_object_unref(caps_elem);
        return;
    }

    GstCaps *caps = NULL;
    g_object_get(caps_elem, "caps", &caps, NULL);
    if (!caps) {
        fprintf(stderr, "FAIL: No caps were set\n");
        gst_object_unref(caps_elem);
        return;
    }

    gchar *caps_str = gst_caps_to_string(caps);
    if (!caps_str) {
        fprintf(stderr, "FAIL: Failed to convert caps to string\n");
        gst_caps_unref(caps);
        gst_object_unref(caps_elem);
        return;
    }

    // Check for BGRx format
    if (!strstr(caps_str, "BGRx")) {
        fprintf(stderr, "FAIL: Caps do not contain BGRx format\n");
        fprintf(stderr, "Caps string: %s\n", caps_str);
        g_free(caps_str);
        gst_caps_unref(caps);
        gst_object_unref(caps_elem);
        return;
    }

    fprintf(stdout, "PASS: BGRx format found in caps\n");

    g_free(caps_str);
    gst_caps_unref(caps);
    gst_object_unref(caps_elem);
}

/**
 * Main test runner
 */
int main(int argc, char *argv[])
{
    fprintf(stdout, "=== Composite Caps Unit Tests ===\n\n");

    // Initialize GStreamer once
    gst_init(&argc, &argv);

    // Run all tests
    test_composite_caps_valid_config();
    fprintf(stdout, "\n");

    test_composite_caps_null_element();
    fprintf(stdout, "\n");

    test_composite_caps_invalid_width();
    fprintf(stdout, "\n");

    test_composite_caps_invalid_height();
    fprintf(stdout, "\n");

    test_composite_caps_invalid_framerate();
    fprintf(stdout, "\n");

    test_composite_caps_bgrx_format();
    fprintf(stdout, "\n");

    // Deinit GStreamer once at the end
    gst_deinit();

    fprintf(stdout, "=== All tests completed ===\n");
    return 0;
}
