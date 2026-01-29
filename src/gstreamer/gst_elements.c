/**
 * @file gst_elements.c
 * @brief GStreamer element creation helpers for recording and playback bins.
 *
 * This module implements factory functions for creating commonly-used GStreamer
 * elements with consistent configuration and error handling. Elements created
 * here are used throughout the pipeline:
 *
 * - Queues: Buffer video frames in GPU memory, decouple source and sink timing
 * - Capsfilters: Negotiate video format (resolution, colorspace, framerate)
 * - Fakesink: Consume buffers in recording bins (actual output via probe callbacks)
 *
 * All functions follow the pattern:
 * 1. Create element from factory
 * 2. Apply default configuration
 * 3. Log any errors and return NULL on failure
 * 4. Return reference to element on success
 */

#include "gst_elements.h"
#include "../utils/logging.h"

/**
 * Create a queue element for buffering video frames in GPU memory.
 *
 * Queue is configured with reasonable defaults for video processing:
 * - max-size-buffers: 30 frames (approximately 1 second at 30fps input)
 * - max-size-bytes: 0 (unlimited; relies on buffer count limit)
 * - max-size-time: 0 (unlimited; relies on buffer count limit)
 * - leaky: none (QUEUE_NO_LEAK; don't drop buffers)
 *
 * This configuration allows frames to accumulate in GPU memory without
 * forced dropping, while preventing unbounded growth.
 *
 * @param name  Unique name for the queue element (e.g., "live_queue", "record_queue_1")
 * @return      Pointer to newly created GstElement queue on success,
 *              NULL if element factory not found or creation fails
 *
 * @note The returned element has ref count 1 and must be unreferenced
 *       by the caller using gst_object_unref() when no longer needed.
 *
 * @note Error messages are logged to stderr via the logging module.
 */
GstElement *gst_elements_create_queue(const char *name)
{
    g_return_val_if_fail(name != NULL, NULL);

    GstElement *queue = gst_element_factory_make("queue", name);
    if (!queue) {
        LOG_ERROR("Failed to create queue element '%s': element factory not found", name);
        return NULL;
    }

    /* Configure queue for GPU video buffering */
    g_object_set(G_OBJECT(queue), "max-size-buffers", 30, /* 1 second at 30fps */
                 "max-size-bytes", 0, /* Unlimited bytes; rely on buffer count */
                 "max-size-time", 0,  /* Unlimited time; rely on buffer count */
                 "leaky", 0,          /* QUEUE_NO_LEAK: don't drop buffers */
                 NULL);

    LOG_DEBUG("Created queue element '%s' (max 30 buffers)", name);
    return queue;
}

/**
 * Create a capsfilter element for video format negotiation.
 *
 * Capsfilter is used to:
 * - Specify exact video format at pipeline points (resolution, colorspace, framerate)
 * - Negotiate caps between upstream and downstream elements
 * - Convert between incompatible formats via GStreamer's negotiation
 *
 * Initial state: No caps set. Caller should set caps using g_object_set()
 * with "caps" property and a GstCaps* object.
 *
 * @param name  Unique name for the capsfilter element (e.g., "live_caps", "composite_caps")
 * @return      Pointer to newly created GstElement capsfilter on success,
 *              NULL if element factory not found or creation fails
 *
 * @note The returned element has ref count 1 and must be unreferenced
 *       by the caller using gst_object_unref() when no longer needed.
 *
 * @note Caps should be set immediately after creation:
 *       GstCaps *caps = gst_caps_from_string("video/x-raw,format=BGRx,width=1920");
 *       g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);
 *       gst_caps_unref(caps);
 *
 * @note Error messages are logged to stderr via the logging module.
 */
GstElement *gst_elements_create_capsfilter(const char *name)
{
    g_return_val_if_fail(name != NULL, NULL);

    GstElement *capsfilter = gst_element_factory_make("capsfilter", name);
    if (!capsfilter) {
        LOG_ERROR("Failed to create capsfilter element '%s': element factory not found", name);
        return NULL;
    }

    LOG_DEBUG("Created capsfilter element '%s'", name);
    return capsfilter;
}

/**
 * Create a fakesink element for discarding buffers (used in recording bins).
 *
 * Fakesink is a debugging sink that consumes buffers without doing anything with them.
 * In recording bins, fakesink serves as the final sink element, consuming buffers
 * after they have been captured via pad probes into ring buffers.
 *
 * Configuration:
 * - sync: FALSE (don't synchronize to clock; frame timing handled by probe)
 * - silent: FALSE (print buffer info for debugging)
 * - dump: FALSE (don't dump buffer contents to stdout)
 *
 * @param name  Unique name for the fakesink element (e.g., "record_sink_1")
 * @return      Pointer to newly created GstElement fakesink on success,
 *              NULL if element factory not found or creation fails
 *
 * @note The returned element has ref count 1 and must be unreferenced
 *       by the caller using gst_object_unref() when no longer needed.
 *
 * @note Fakesink is appropriate for recording bins because:
 *       - Actual video data is captured via pad probes to ring buffer
 *       - Fakesink just consumes buffers after probe processing
 *       - No real output stream (no filesink, videosink, etc.)
 *
 * @note Error messages are logged to stderr via the logging module.
 */
GstElement *gst_elements_create_fakesink(const char *name)
{
    g_return_val_if_fail(name != NULL, NULL);

    GstElement *fakesink = gst_element_factory_make("fakesink", name);
    if (!fakesink) {
        LOG_ERROR("Failed to create fakesink element '%s': element factory not found", name);
        return NULL;
    }

    /* Configure fakesink for recording bin output */
    g_object_set(G_OBJECT(fakesink), "sync",
                 FALSE,           /* Don't synchronize to clock; probe handles timing */
                 "silent", FALSE, /* Print buffer info for debugging */
                 "dump", FALSE,   /* Don't dump buffer contents to stdout */
                 NULL);

    LOG_DEBUG("Created fakesink element '%s'", name);
    return fakesink;
}
