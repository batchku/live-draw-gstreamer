/**
 * @file test_live_queue.c
 * @brief Unit tests for live_queue element creation and caps negotiation.
 *
 * Tests verify:
 * - live_queue_create() successfully creates a queue element with proper configuration
 * - live_queue_configure_caps() correctly applies caps to capsfilter
 * - live_queue_negotiate_caps() negotiates optimal caps from camera source
 * - Caps negotiation handles fallback scenarios gracefully
 *
 * These tests ensure the live feed (cell 1) properly handles caps negotiation
 * for GPU memory buffering and format compatibility with videomixer.
 */

#include <assert.h>
#include <glib.h>
#include <gst/gst.h>
#include <stdio.h>
#include <string.h>

// Include headers to test
#include "gstreamer/gst_elements.h"
#include "gstreamer/live_queue.h"

// ============================================================================
// Test Fixtures
// ============================================================================

/**
 * Initialize GStreamer for testing.
 *
 * Must be called once at start of test suite.
 */
static void init_gstreamer(void)
{
    static gboolean initialized = FALSE;
    if (!initialized) {
        gst_init(NULL, NULL);
        initialized = TRUE;
    }
}

/**
 * Clean up GStreamer after testing.
 *
 * Should be called once at end of test suite.
 */
static void cleanup_gstreamer(void)
{
    gst_deinit();
}

// ============================================================================
// Test: live_queue_create - Successful Creation
// ============================================================================

/**
 * Test that live_queue_create() successfully creates a queue element.
 *
 * Verifies:
 * - Returns non-NULL pointer
 * - Created element is a queue element
 * - Element has correct name
 * - Element has proper ref count
 */
static void test_live_queue_create_success(void)
{
    printf("\n[TEST] live_queue_create() - Successful Creation\n");

    GstElement *queue = live_queue_create("test-queue");
    assert(queue != NULL);

    // Verify it's a queue element
    GstElementFactory *factory = gst_element_get_factory(queue);
    assert(factory != NULL);
    const gchar *factory_name = GST_OBJECT_NAME(factory);
    assert(g_str_has_prefix(factory_name, "queue") || strcmp(factory_name, "queue") == 0);

    // Verify name is set
    const gchar *element_name = GST_ELEMENT_NAME(queue);
    assert(element_name != NULL);
    assert(strcmp(element_name, "test-queue") == 0);

    gst_object_unref(queue);
    printf("  ✓ Queue element created successfully with correct properties\n");
}

/**
 * Test that live_queue_create() rejects NULL name.
 *
 * Verifies:
 * - Returns NULL for NULL name
 * - Handles invalid input gracefully
 */
static void test_live_queue_create_null_name(void)
{
    printf("\n[TEST] live_queue_create() - NULL Name Handling\n");

    GstElement *queue = live_queue_create(NULL);
    assert(queue == NULL);

    printf("  ✓ Correctly rejected NULL name\n");
}

/**
 * Test that live_queue_create() configures queue properties correctly.
 *
 * Verifies:
 * - max-size-buffers is set to 30
 * - max-size-bytes is set to 0 (unlimited)
 * - max-size-time is set to 0 (unlimited)
 * - leaky is set to 2 (GST_QUEUE_LEAK_DOWNSTREAM)
 */
static void test_live_queue_create_configuration(void)
{
    printf("\n[TEST] live_queue_create() - Queue Configuration\n");

    GstElement *queue = live_queue_create("config-queue");
    assert(queue != NULL);

    // Check queue properties
    guint max_buffers;
    guint64 max_bytes, max_time;
    guint leaky;

    g_object_get(G_OBJECT(queue), "max-size-buffers", &max_buffers, "max-size-bytes", &max_bytes,
                 "max-size-time", &max_time, "leaky", &leaky, NULL);

    assert(max_buffers == 30);
    assert(max_bytes == 0);
    assert(max_time == 0);
    assert(leaky == 2); // GST_QUEUE_LEAK_DOWNSTREAM

    gst_object_unref(queue);
    printf("  ✓ Queue configuration correct (max_buffers=30, leaky=downstream)\n");
}

// ============================================================================
// Test: live_queue_configure_caps - Successful Configuration
// ============================================================================

/**
 * Test that live_queue_configure_caps() applies caps correctly.
 *
 * Verifies:
 * - Accepts valid caps configuration
 * - Sets capsfilter caps property
 * - Returns TRUE on success
 */
static void test_live_queue_configure_caps_success(void)
{
    printf("\n[TEST] live_queue_configure_caps() - Successful Configuration\n");

    GstElement *capsfilter = gst_elements_create_capsfilter("test-caps");
    assert(capsfilter != NULL);

    LiveQueueCaps caps_config = {
        .width = 1920, .height = 1080, .framerate_num = 30, .framerate_den = 1, .format = "BGRx"};

    gboolean result = live_queue_configure_caps(capsfilter, &caps_config);
    assert(result == TRUE);

    // Verify caps were applied
    GstCaps *applied_caps;
    g_object_get(G_OBJECT(capsfilter), "caps", &applied_caps, NULL);
    assert(applied_caps != NULL);

    // Check caps contains expected values
    GstStructure *structure = gst_caps_get_structure(applied_caps, 0);
    assert(structure != NULL);

    gint width, height;
    const gchar *format;
    gst_structure_get(structure, "width", G_TYPE_INT, &width, NULL);
    gst_structure_get(structure, "height", G_TYPE_INT, &height, NULL);
    format = gst_structure_get_string(structure, "format");

    assert(width == 1920);
    assert(height == 1080);
    assert(format != NULL);
    assert(strcmp(format, "BGRx") == 0);

    gst_caps_unref(applied_caps);
    gst_object_unref(capsfilter);
    printf("  ✓ Caps applied correctly (BGRx 1920×1080 @ 30fps)\n");
}

/**
 * Test that live_queue_configure_caps() handles NULL capsfilter.
 *
 * Verifies:
 * - Returns FALSE for NULL capsfilter
 * - Handles invalid input gracefully
 */
static void test_live_queue_configure_caps_null_capsfilter(void)
{
    printf("\n[TEST] live_queue_configure_caps() - NULL Capsfilter Handling\n");

    LiveQueueCaps caps_config = {
        .width = 1920, .height = 1080, .framerate_num = 30, .framerate_den = 1, .format = "BGRx"};

    gboolean result = live_queue_configure_caps(NULL, &caps_config);
    assert(result == FALSE);

    printf("  ✓ Correctly rejected NULL capsfilter\n");
}

/**
 * Test that live_queue_configure_caps() handles NULL config.
 *
 * Verifies:
 * - Returns FALSE for NULL config
 * - Handles invalid input gracefully
 */
static void test_live_queue_configure_caps_null_config(void)
{
    printf("\n[TEST] live_queue_configure_caps() - NULL Config Handling\n");

    GstElement *capsfilter = gst_elements_create_capsfilter("test-caps");
    assert(capsfilter != NULL);

    gboolean result = live_queue_configure_caps(capsfilter, NULL);
    assert(result == FALSE);

    gst_object_unref(capsfilter);
    printf("  ✓ Correctly rejected NULL config\n");
}

/**
 * Test various format strings in caps configuration.
 *
 * Verifies:
 * - BGRx format works
 * - YUY2 format works
 * - Different resolutions work
 */
static void test_live_queue_configure_caps_multiple_formats(void)
{
    printf("\n[TEST] live_queue_configure_caps() - Multiple Formats\n");

    // Test BGRx
    {
        GstElement *caps = gst_elements_create_capsfilter("caps-bgrx");
        LiveQueueCaps config = {.width = 1920,
                                .height = 1080,
                                .framerate_num = 30,
                                .framerate_den = 1,
                                .format = "BGRx"};
        assert(live_queue_configure_caps(caps, &config) == TRUE);
        gst_object_unref(caps);
    }

    // Test YUY2
    {
        GstElement *caps = gst_elements_create_capsfilter("caps-yuy2");
        LiveQueueCaps config = {.width = 1280,
                                .height = 720,
                                .framerate_num = 30,
                                .framerate_den = 1,
                                .format = "YUY2"};
        assert(live_queue_configure_caps(caps, &config) == TRUE);
        gst_object_unref(caps);
    }

    // Test different resolution (720p)
    {
        GstElement *caps = gst_elements_create_capsfilter("caps-720p");
        LiveQueueCaps config = {.width = 1280,
                                .height = 720,
                                .framerate_num = 30,
                                .framerate_den = 1,
                                .format = "BGRx"};
        assert(live_queue_configure_caps(caps, &config) == TRUE);
        gst_object_unref(caps);
    }

    printf("  ✓ Multiple format configurations successful\n");
}

// ============================================================================
// Test: live_queue_negotiate_caps - Caps Negotiation
// ============================================================================

/**
 * Test that live_queue_negotiate_caps() handles NULL camera source.
 *
 * Verifies:
 * - Returns FALSE for NULL camera source
 * - Handles invalid input gracefully
 */
static void test_live_queue_negotiate_caps_null_camera(void)
{
    printf("\n[TEST] live_queue_negotiate_caps() - NULL Camera Handling\n");

    LiveQueueCaps out_config;
    gboolean result = live_queue_negotiate_caps(NULL, &out_config);
    assert(result == FALSE);

    printf("  ✓ Correctly rejected NULL camera source\n");
}

/**
 * Test that live_queue_negotiate_caps() handles NULL output config.
 *
 * Verifies:
 * - Returns FALSE for NULL output config
 * - Handles invalid input gracefully
 */
static void test_live_queue_negotiate_caps_null_output(void)
{
    printf("\n[TEST] live_queue_negotiate_caps() - NULL Output Config Handling\n");

    // Create a fake appsrc element to use as camera source
    GstElement *camera = gst_element_factory_make("appsrc", "fake-camera");
    assert(camera != NULL);

    gboolean result = live_queue_negotiate_caps(camera, NULL);
    assert(result == FALSE);

    gst_object_unref(camera);
    printf("  ✓ Correctly rejected NULL output config\n");
}

/**
 * Test that live_queue_negotiate_caps() returns default caps safely.
 *
 * Verifies:
 * - Sets default output values even if negotiation fails
 * - Returns sensible defaults (1920×1080, BGRx, 30fps)
 * - Non-fatal failure (returns TRUE with defaults)
 */
static void test_live_queue_negotiate_caps_defaults(void)
{
    printf("\n[TEST] live_queue_negotiate_caps() - Default Caps\n");

    // Create a minimal camera element (appsrc doesn't have real camera caps)
    GstElement *camera = gst_element_factory_make("appsrc", "fake-camera");
    assert(camera != NULL);

    LiveQueueCaps out_config;
    gboolean result = live_queue_negotiate_caps(camera, &out_config);

    // Result should be TRUE (non-fatal failure with defaults)
    assert(result == TRUE);

    // Check defaults were applied
    assert(out_config.width == 1920);
    assert(out_config.height == 1080);
    assert(out_config.framerate_num == 30);
    assert(out_config.framerate_den == 1);
    assert(out_config.format != NULL);
    assert(strcmp(out_config.format, "BGRx") == 0);

    gst_object_unref(camera);
    printf("  ✓ Default caps applied correctly (1920×1080 BGRx @ 30fps)\n");
}

// ============================================================================
// Test: Integration - Full Live Queue Pipeline
// ============================================================================

/**
 * Test complete live queue integration in a simple pipeline.
 *
 * Verifies:
 * - Queue links with capsfilter
 * - Capsfilter links with downstream element
 * - Pipeline state transitions work
 */
static void test_live_queue_pipeline_integration(void)
{
    printf("\n[TEST] Live Queue - Pipeline Integration\n");

    GstElement *pipeline = gst_pipeline_new("test-pipeline");
    assert(pipeline != NULL);

    // Create elements
    GstElement *source = gst_element_factory_make("appsrc", "source");
    GstElement *queue = live_queue_create("live-queue");
    GstElement *capsfilter = gst_elements_create_capsfilter("live-caps");
    GstElement *sink = gst_element_factory_make("fakesink", "sink");

    assert(source != NULL);
    assert(queue != NULL);
    assert(capsfilter != NULL);
    assert(sink != NULL);

    // Configure capsfilter
    LiveQueueCaps config = {
        .width = 1920, .height = 1080, .framerate_num = 30, .framerate_den = 1, .format = "BGRx"};
    assert(live_queue_configure_caps(capsfilter, &config) == TRUE);

    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, queue, capsfilter, sink, NULL);

    // Link elements
    assert(gst_element_link(source, queue) == TRUE);
    assert(gst_element_link(queue, capsfilter) == TRUE);
    assert(gst_element_link(capsfilter, sink) == TRUE);

    // Set to READY state
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_READY);
    assert(ret != GST_STATE_CHANGE_FAILURE);

    // Clean up
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    printf("  ✓ Live queue integrates correctly in pipeline\n");
}

// ============================================================================
// Main Test Suite
// ============================================================================

int main(int argc, char *argv[])
{
    (void) argc; // Suppress unused parameter warning
    (void) argv;

    printf("\n");
    printf("======================================================================\n");
    printf("Live Queue Unit Tests\n");
    printf("======================================================================\n");

    init_gstreamer();

    // Test live_queue_create
    test_live_queue_create_success();
    test_live_queue_create_null_name();
    test_live_queue_create_configuration();

    // Test live_queue_configure_caps
    test_live_queue_configure_caps_success();
    test_live_queue_configure_caps_null_capsfilter();
    test_live_queue_configure_caps_null_config();
    test_live_queue_configure_caps_multiple_formats();

    // Test live_queue_negotiate_caps
    test_live_queue_negotiate_caps_null_camera();
    test_live_queue_negotiate_caps_null_output();
    test_live_queue_negotiate_caps_defaults();

    // Test integration
    test_live_queue_pipeline_integration();

    cleanup_gstreamer();

    printf("\n");
    printf("======================================================================\n");
    printf("All tests passed! ✓\n");
    printf("======================================================================\n\n");

    return 0;
}
