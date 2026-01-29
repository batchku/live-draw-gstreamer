#ifndef GST_ELEMENTS_H
#define GST_ELEMENTS_H

#include <gst/gst.h>

/**
 * @file gst_elements.h
 * @brief GStreamer element creation helpers for recording and playback bins.
 *
 * This module provides factory functions for creating common GStreamer elements
 * used in record and playback bins (queues, capsfilters, fakesink) with
 * consistent configuration and error handling.
 *
 * All returned elements are referenced (g_object_ref) and must be unreferenced
 * by the caller using gst_object_unref().
 */

/**
 * Create a queue element for buffering video frames in GPU memory.
 *
 * Configures queue with:
 * - max-size-buffers: 30 frames (~1 second at 30fps input)
 * - max-size-bytes: unlimited (0)
 * - max-size-time: unlimited (0)
 * - leaky: none (don't drop buffers)
 *
 * Used for:
 * - Live feed queue in cell 1
 * - Buffer queue in record bins
 * - Playback queue before videomixer input
 *
 * @param name  Name for the queue element (must be unique in pipeline)
 * @return      Pointer to GstElement queue on success, NULL on failure
 *
 * Error handling:
 * - Returns NULL if element factory "queue" not found
 * - Returns NULL if element creation fails
 * Errors are logged via logging module.
 */
GstElement *gst_elements_create_queue(const char *name);

/**
 * Create a capsfilter element for format negotiation.
 *
 * Capsfilter is used to negotiate and filter caps at specific pipeline points:
 * - Video format (e.g., "video/x-raw, format=BGRx, width=1920, height=1080")
 * - Framerate (e.g., "framerate=30/1")
 *
 * Initial configuration:
 * - Caps: None (caller should set via g_object_set after creation)
 * - Direction: Negotiates bidirectional caps between upstream and downstream
 *
 * Used for:
 * - Live feed capsfilter for format spec
 * - Record bin output capsfilter
 * - Composite (final) capsfilter before osxvideosink
 *
 * @param name  Name for the capsfilter element (must be unique in pipeline)
 * @return      Pointer to GstElement capsfilter on success, NULL on failure
 *
 * Error handling:
 * - Returns NULL if element factory "capsfilter" not found
 * - Returns NULL if element creation fails
 * Errors are logged via logging module.
 *
 * Usage:
 * @code
 *   GstElement *caps = gst_elements_create_capsfilter("live_caps");
 *   if (caps) {
 *       GstCaps *gst_caps = gst_caps_from_string("video/x-raw,format=BGRx");
 *       g_object_set(G_OBJECT(caps), "caps", gst_caps, NULL);
 *       gst_caps_unref(gst_caps);
 *   }
 * @endcode
 */
GstElement *gst_elements_create_capsfilter(const char *name);

/**
 * Create a fakesink element for discarding buffers (used in recording bins).
 *
 * Fakesink is used as a sink element in recording bins to consume video buffers
 * while they are being captured to ring buffers via probe callbacks.
 *
 * Configuration:
 * - sync: FALSE (don't synchronize to clock; we handle frame timing via probes)
 * - silent: FALSE (log buffer arrival for debugging)
 * - dump: FALSE (don't dump buffer contents to stdout)
 *
 * Used for:
 * - Recording bin sink element (consumes buffers after probe capture)
 *
 * @param name  Name for the fakesink element (must be unique in pipeline)
 * @return      Pointer to GstElement fakesink on success, NULL on failure
 *
 * Error handling:
 * - Returns NULL if element factory "fakesink" not found
 * - Returns NULL if element creation fails
 * Errors are logged via logging module.
 *
 * @note Fakesink is a standard GStreamer debugging element; production pipelines
 *       typically use a real sink (videosink, filesink, etc.). In recording bins,
 *       fakesink is appropriate because the actual output (recorded buffers) is
 *       handled via pad probes and ring buffer management, not via the sink.
 */
GstElement *gst_elements_create_fakesink(const char *name);

#endif // GST_ELEMENTS_H
