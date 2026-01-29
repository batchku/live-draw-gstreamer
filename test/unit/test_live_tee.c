/**
 * @file test_live_tee.c
 * @brief Unit tests for live_tee module (tee element configuration).
 *
 * Tests the tee element configuration and pad management functions
 * for deadlock-free stream splitting in the video looper pipeline.
 */

#include "../../src/gstreamer/live_tee.h"
#include "../../src/utils/logging.h"
#include <assert.h>
#include <gst/gst.h>
#include <stdio.h>

/**
 * Test: Tee element configuration
 *
 * Verifies that live_tee_configure() correctly sets properties
 * on the tee element for deadlock prevention.
 */
static void test_live_tee_configure(void)
{
    printf("\n=== Test: live_tee_configure ===\n");

    gst_init(NULL, NULL);

    // Create a tee element
    GstElement *tee = gst_element_factory_make("tee", "test-tee");
    assert(tee != NULL && "Failed to create tee element");

    // Configure the tee for deadlock prevention
    gboolean ret = live_tee_configure(tee);
    assert(ret == TRUE && "live_tee_configure returned FALSE");

    // Verify that allow-not-linked property was set
    gboolean allow_not_linked;
    g_object_get(G_OBJECT(tee), "allow-not-linked", &allow_not_linked, NULL);
    assert(allow_not_linked == TRUE && "allow-not-linked property not set to TRUE");

    printf("✓ Tee element configured successfully with allow-not-linked=TRUE\n");

    // Clean up
    gst_object_unref(tee);
    gst_deinit();
}

/**
 * Test: Tee element with NULL input
 *
 * Verifies error handling when passing NULL to live_tee_configure().
 */
static void test_live_tee_configure_null_input(void)
{
    printf("\n=== Test: live_tee_configure with NULL input ===\n");

    gst_init(NULL, NULL);

    // Try to configure with NULL
    gboolean ret = live_tee_configure(NULL);
    assert(ret == FALSE && "live_tee_configure should return FALSE for NULL input");

    printf("✓ Correctly rejected NULL tee element\n");

    gst_deinit();
}

/**
 * Test: Request pad from tee element
 *
 * Verifies that live_tee_request_pad() successfully requests output pads.
 */
static void test_live_tee_request_pad(void)
{
    printf("\n=== Test: live_tee_request_pad ===\n");

    gst_init(NULL, NULL);

    // Create a tee element
    GstElement *tee = gst_element_factory_make("tee", "test-tee");
    assert(tee != NULL && "Failed to create tee element");

    // Configure the tee
    live_tee_configure(tee);

    // Request pads for recording bins (keys 1-9)
    GstPad *pads[9];
    for (int i = 1; i <= 9; i++) {
        pads[i - 1] = live_tee_request_pad(tee, i);
        assert(pads[i - 1] != NULL && "Failed to request pad from tee");
        printf("  ✓ Requested pad for record bin %d (pad: %s)\n", i, GST_PAD_NAME(pads[i - 1]));
    }

    printf("✓ Successfully requested 9 output pads from tee element\n");

    // Clean up pads (release them back to tee)
    for (int i = 0; i < 9; i++) {
        gst_object_unref(pads[i]);
    }

    // Clean up
    gst_object_unref(tee);
    gst_deinit();
}

/**
 * Test: Release pad from tee element
 *
 * Verifies that live_tee_release_pad() correctly releases pads.
 */
static void test_live_tee_release_pad(void)
{
    printf("\n=== Test: live_tee_release_pad ===\n");

    gst_init(NULL, NULL);

    // Create a tee element
    GstElement *tee = gst_element_factory_make("tee", "test-tee");
    assert(tee != NULL && "Failed to create tee element");

    // Configure the tee
    live_tee_configure(tee);

    // Request a pad
    GstPad *tee_pad = live_tee_request_pad(tee, 1);
    assert(tee_pad != NULL && "Failed to request pad from tee");

    // Release the pad
    gboolean ret = live_tee_release_pad(tee, tee_pad);
    assert(ret == TRUE && "live_tee_release_pad returned FALSE");

    printf("✓ Successfully released pad from tee element\n");

    // Clean up
    gst_object_unref(tee);
    gst_deinit();
}

/**
 * Test: Multiple pad creation and release
 *
 * Verifies that multiple pads can be requested and released independently.
 */
static void test_live_tee_multiple_pads(void)
{
    printf("\n=== Test: Multiple pad creation and release ===\n");

    gst_init(NULL, NULL);

    // Create a tee element
    GstElement *tee = gst_element_factory_make("tee", "test-tee");
    assert(tee != NULL && "Failed to create tee element");

    // Configure the tee
    live_tee_configure(tee);

    // Request 3 pads
    GstPad *pad1 = live_tee_request_pad(tee, 1);
    GstPad *pad2 = live_tee_request_pad(tee, 2);
    GstPad *pad3 = live_tee_request_pad(tee, 3);

    assert(pad1 != NULL && pad2 != NULL && pad3 != NULL && "Failed to request pads");
    printf("  ✓ Created 3 pads: %s, %s, %s\n", GST_PAD_NAME(pad1), GST_PAD_NAME(pad2),
           GST_PAD_NAME(pad3));

    // Release pad 2 (middle one)
    gboolean ret = live_tee_release_pad(tee, pad2);
    assert(ret == TRUE && "Failed to release pad 2");
    printf("  ✓ Released pad 2\n");

    // Release pad 1
    ret = live_tee_release_pad(tee, pad1);
    assert(ret == TRUE && "Failed to release pad 1");
    printf("  ✓ Released pad 1\n");

    // Release pad 3
    ret = live_tee_release_pad(tee, pad3);
    assert(ret == TRUE && "Failed to release pad 3");
    printf("  ✓ Released pad 3\n");

    printf("✓ Successfully created and released 3 pads in various orders\n");

    // Clean up
    gst_object_unref(tee);
    gst_deinit();
}

/**
 * Test: Request pad with NULL tee
 *
 * Verifies error handling when requesting pad from NULL tee.
 */
static void test_live_tee_request_pad_null(void)
{
    printf("\n=== Test: live_tee_request_pad with NULL tee ===\n");

    gst_init(NULL, NULL);

    // Try to request pad from NULL tee
    GstPad *pad = live_tee_request_pad(NULL, 1);
    assert(pad == NULL && "live_tee_request_pad should return NULL for NULL tee");

    printf("✓ Correctly rejected NULL tee element\n");

    gst_deinit();
}

/**
 * Main test runner
 */
int main(int argc G_GNUC_UNUSED, char **argv G_GNUC_UNUSED)
{
    printf("\n================================================\n");
    printf("Live Tee Element Unit Tests\n");
    printf("================================================\n");

    // Enable debug logging
    logging_set_level(LOG_LEVEL_DEBUG);

    // Run all tests
    test_live_tee_configure();
    test_live_tee_configure_null_input();
    test_live_tee_request_pad();
    test_live_tee_release_pad();
    test_live_tee_multiple_pads();
    test_live_tee_request_pad_null();

    printf("\n================================================\n");
    printf("✓ All live_tee tests passed!\n");
    printf("================================================\n\n");

    return 0;
}
