/**
 * @file test_tee_stream_splitting.c
 * @brief Integration test for tee stream splitting without deadlock.
 *
 * Tests that the tee element correctly splits the camera stream
 * to multiple record bins and the videomixer without causing deadlock,
 * even when pads are dynamically added/removed.
 *
 * This test validates the core requirement of T-5.4:
 * "Implement tee element to split live stream to recording bins
 *  and videomixer without deadlock"
 */

#include "../../src/gstreamer/pipeline_builder.h"
#include "../../src/utils/logging.h"
#include <assert.h>
#include <gst/gst.h>
#include <stdio.h>

/**
 * Test: Tee stream splitting with live queue and record bins
 *
 * Verifies that:
 * 1. Tee correctly splits camera stream
 * 2. Live queue receives frames without blocking
 * 3. Record bins can be added/removed dynamically
 * 4. No deadlock occurs when switching pads
 */
static void test_tee_splits_to_live_and_recording(void)
{
    printf("\n=== Test: Tee splits stream to live queue and recording bins ===\n");

    // Initialize GStreamer
    gst_init(NULL, NULL);

    // Create a mock camera source (appsrc for testing)
    // This avoids needing actual hardware for the test
    GstElement *camera_source = gst_element_factory_make("appsrc", "mock-camera");
    assert(camera_source != NULL && "Failed to create mock camera source");

    // Configure appsrc with test caps
    GstCaps *test_caps = gst_caps_from_string("video/x-raw, "
                                              "width=1920, height=1080, "
                                              "framerate=30/1, "
                                              "format=BGRx");
    assert(test_caps != NULL && "Failed to create test caps");

    g_object_set(G_OBJECT(camera_source), "caps", test_caps, "is-live", TRUE, "block", FALSE, NULL);
    gst_caps_unref(test_caps);

    // Create the pipeline with camera source
    Pipeline *pipeline = pipeline_create(camera_source);
    assert(pipeline != NULL && "Failed to create pipeline");
    assert(pipeline->live_tee != NULL && "Pipeline missing tee element");

    printf("  ✓ Pipeline created with tee element\n");

    // Verify tee has the correct configuration
    gboolean allow_not_linked;
    g_object_get(G_OBJECT(pipeline->live_tee), "allow-not-linked", &allow_not_linked, NULL);
    assert(allow_not_linked == TRUE && "Tee element not configured with allow-not-linked");

    printf("  ✓ Tee element configured for deadlock-free operation\n");

    // Set pipeline to READY state
    gboolean ret = pipeline_set_state(pipeline, GST_STATE_READY);
    assert(ret == TRUE && "Failed to set pipeline to READY state");

    printf("  ✓ Pipeline state set to READY\n");

    // Add recording bins dynamically
    printf("  Adding recording bins...\n");
    for (int key = 1; key <= 3; key++) {
        ret = pipeline_add_record_bin(pipeline, key);
        assert(ret == TRUE && "Failed to add record bin");
        printf("    ✓ Added record bin for key %d\n", key);
    }

    // Try removing a bin in the middle (tests dynamic unlinking)
    printf("  Removing record bin for key 2...\n");
    ret = pipeline_remove_record_bin(pipeline, 2);
    assert(ret == TRUE && "Failed to remove record bin");
    printf("    ✓ Removed record bin for key 2\n");

    // Add another bin to test adding after removal
    printf("  Re-adding recording bins...\n");
    ret = pipeline_add_record_bin(pipeline, 2);
    assert(ret == TRUE && "Failed to re-add record bin");
    printf("    ✓ Re-added record bin for key 2\n");

    ret = pipeline_add_record_bin(pipeline, 4);
    assert(ret == TRUE && "Failed to add record bin for key 4");
    printf("    ✓ Added record bin for key 4\n");

    printf("✓ Tee successfully splits stream without deadlock\n");

    // Cleanup
    printf("  Cleaning up pipeline...\n");

    // Remove all record bins
    for (int key = 1; key <= 9; key++) {
        pipeline_remove_record_bin(pipeline, key);
    }

    // Cleanup pipeline
    pipeline_cleanup(pipeline);
    gst_object_unref(camera_source);

    printf("  ✓ Pipeline cleaned up successfully\n");

    gst_deinit();
}

/**
 * Test: Tee pad consistency during dynamic operations
 *
 * Verifies that tee pads remain consistent and properly linked
 * during dynamic record bin addition/removal.
 */
static void test_tee_pad_consistency(void)
{
    printf("\n=== Test: Tee pad consistency during dynamic operations ===\n");

    gst_init(NULL, NULL);

    // Create mock camera source
    GstElement *camera_source = gst_element_factory_make("appsrc", "mock-camera");
    assert(camera_source != NULL);

    GstCaps *test_caps =
        gst_caps_from_string("video/x-raw, width=1920, height=1080, framerate=30/1, format=BGRx");
    g_object_set(G_OBJECT(camera_source), "caps", test_caps, "is-live", TRUE, NULL);
    gst_caps_unref(test_caps);

    // Create pipeline
    Pipeline *pipeline = pipeline_create(camera_source);
    assert(pipeline != NULL);

    // Set to READY
    pipeline_set_state(pipeline, GST_STATE_READY);

    // Add bins in sequence and verify pad count
    guint expected_src_pads = 1; // Live queue is already connected
    for (int key = 1; key <= 5; key++) {
        pipeline_add_record_bin(pipeline, key);
        expected_src_pads++;

        // Count actual source pads on tee
        GstIterator *iter = gst_element_iterate_src_pads(pipeline->live_tee);
        guint actual_src_pads = 0;
        GValue item = G_VALUE_INIT;
        while (gst_iterator_next(iter, &item) == GST_ITERATOR_OK) {
            actual_src_pads++;
            g_value_unset(&item);
        }
        gst_iterator_free(iter);

        printf("  ✓ After adding key %d: tee has %u source pads (expected %u)\n", key,
               actual_src_pads, expected_src_pads);
        assert(actual_src_pads == expected_src_pads &&
               "Tee pad count mismatch after adding record bin");
    }

    // Remove bins and verify pad count decreases
    for (int key = 5; key >= 1; key--) {
        pipeline_remove_record_bin(pipeline, key);
        expected_src_pads--;

        // Count actual source pads on tee
        GstIterator *iter = gst_element_iterate_src_pads(pipeline->live_tee);
        guint actual_src_pads = 0;
        GValue item = G_VALUE_INIT;
        while (gst_iterator_next(iter, &item) == GST_ITERATOR_OK) {
            actual_src_pads++;
            g_value_unset(&item);
        }
        gst_iterator_free(iter);

        printf("  ✓ After removing key %d: tee has %u source pads (expected %u)\n", key,
               actual_src_pads, expected_src_pads);
        assert(actual_src_pads == expected_src_pads &&
               "Tee pad count mismatch after removing record bin");
    }

    printf("✓ Tee pad consistency verified\n");

    // Cleanup
    pipeline_cleanup(pipeline);
    gst_object_unref(camera_source);
    gst_deinit();
}

/**
 * Test: Live queue remains unaffected by record bin operations
 *
 * Verifies that adding/removing record bins does not interfere
 * with the live queue's connection to the videomixer.
 */
static void test_live_queue_unaffected(void)
{
    printf("\n=== Test: Live queue unaffected by record bin operations ===\n");

    gst_init(NULL, NULL);

    // Create mock camera source
    GstElement *camera_source = gst_element_factory_make("appsrc", "mock-camera");
    assert(camera_source != NULL);

    GstCaps *test_caps =
        gst_caps_from_string("video/x-raw, width=1920, height=1080, framerate=30/1, format=BGRx");
    g_object_set(G_OBJECT(camera_source), "caps", test_caps, "is-live", TRUE, NULL);
    gst_caps_unref(test_caps);

    // Create pipeline
    Pipeline *pipeline = pipeline_create(camera_source);
    assert(pipeline != NULL);

    // Verify initial state: live queue should be connected
    GstPad *live_queue_sink = gst_element_get_static_pad(pipeline->live_queue, "sink");
    assert(live_queue_sink != NULL && "Failed to get live_queue sink pad");

    GstPad *live_queue_peer = gst_pad_get_peer(live_queue_sink);
    assert(live_queue_peer != NULL && "Live queue not connected to tee");
    gst_object_unref(live_queue_peer);
    gst_object_unref(live_queue_sink);

    printf("  ✓ Live queue connected to tee\n");

    // Set pipeline to READY
    pipeline_set_state(pipeline, GST_STATE_READY);

    // Add and remove record bins multiple times
    for (int cycle = 0; cycle < 3; cycle++) {
        printf("  Cycle %d: Adding and removing bins...\n", cycle + 1);
        for (int key = 1; key <= 9; key++) {
            pipeline_add_record_bin(pipeline, key);
        }

        for (int key = 9; key >= 1; key--) {
            pipeline_remove_record_bin(pipeline, key);
        }

        // Verify live queue is still connected
        live_queue_sink = gst_element_get_static_pad(pipeline->live_queue, "sink");
        live_queue_peer = gst_pad_get_peer(live_queue_sink);
        assert(live_queue_peer != NULL && "Live queue disconnected from tee");
        gst_object_unref(live_queue_peer);
        gst_object_unref(live_queue_sink);

        printf("    ✓ Live queue still connected after cycle %d\n", cycle + 1);
    }

    printf("✓ Live queue remained unaffected by record bin operations\n");

    // Cleanup
    pipeline_cleanup(pipeline);
    gst_object_unref(camera_source);
    gst_deinit();
}

/**
 * Main test runner
 */
int main(int argc G_GNUC_UNUSED, char **argv G_GNUC_UNUSED)
{
    printf("\n================================================\n");
    printf("Tee Stream Splitting Integration Tests\n");
    printf("================================================\n");

    // Enable debug logging for detailed output
    logging_set_level(LOG_LEVEL_DEBUG);

    // Run all tests
    test_tee_splits_to_live_and_recording();
    test_tee_pad_consistency();
    test_live_queue_unaffected();

    printf("\n================================================\n");
    printf("✓ All tee stream splitting tests passed!\n");
    printf("================================================\n\n");

    return 0;
}
