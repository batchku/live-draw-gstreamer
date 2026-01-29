/**
 * @file live_tee.h
 * @brief Live stream tee element configuration for deadlock-free stream splitting.
 *
 * This module provides specialized configuration for the live_tee element
 * that splits the camera feed to multiple destinations without deadlock:
 * - One branch to live_queue → videomixer (for cell 1 display)
 * - Nine branches to record bins (for recording video loops)
 *
 * Deadlock Prevention:
 * - The tee element is configured with allow-not-linked=true
 * - This allows output pads to be unlinked without blocking input
 * - Recording bins are dynamically added/removed without pipeline deadlock
 * - The live queue branch uses a leaky queue to handle backpressure
 */

#ifndef LIVE_TEE_H
#define LIVE_TEE_H

#include <gst/gst.h>

/**
 * Configure tee element for deadlock-free stream splitting.
 *
 * Sets up the tee element properties to:
 * - allow-not-linked = true (allow unlinked pads without blocking)
 * - has-chain = true (allow synchronization on input side)
 *
 * These properties ensure that:
 * 1. The live queue branch can buffer frames independently
 * 2. Recording bins can be added/removed without deadlock
 * 3. One slow consumer doesn't block others
 *
 * @param tee_element   The tee element created by gst_element_factory_make("tee", ...)
 * @return              TRUE if configured successfully, FALSE on failure
 *
 * Error handling:
 * - Returns FALSE if tee_element is NULL
 * - Logs warning if properties are not available (older GStreamer versions)
 *
 * Note: Call this function immediately after creating the tee element
 * and before adding it to the pipeline.
 */
gboolean live_tee_configure(GstElement *tee_element);

/**
 * Request a new output pad from the tee element for a record bin.
 *
 * Creates a new source pad on the tee element for connecting to a record bin.
 * Each record bin gets its own tee output pad, allowing independent control
 * of frame flow to each recording destination.
 *
 * @param tee_element   The tee element
 * @param record_bin_id Unique identifier for logging (typically key number 1-9)
 * @return              Pointer to requested GstPad, or NULL on failure
 *
 * Error handling:
 * - Returns NULL if tee_element is NULL
 * - Returns NULL if pad request fails
 * - Logs error if request fails
 *
 * Ownership: The returned pad is owned by the tee element. The caller
 * must not unreference it directly. Instead, use gst_pad_unlink() when
 * disconnecting, and the pad will be released by the tee element.
 *
 * Note: Multiple pads can be requested from the same tee element.
 * Each pad will automatically generate a unique name (e.g., src_0, src_1, etc).
 */
GstPad *live_tee_request_pad(GstElement *tee_element, int record_bin_id);

/**
 * Release an output pad from the tee element.
 *
 * Releases a previously requested tee output pad. This is called when
 * a record bin is removed from the pipeline.
 *
 * @param tee_element   The tee element
 * @param tee_pad       The pad to release (previously returned by live_tee_request_pad)
 * @return              TRUE if released successfully, FALSE on failure
 *
 * Error handling:
 * - Returns FALSE if tee_element or tee_pad is NULL
 * - Returns FALSE if pad release fails
 * - Logs error if release fails
 *
 * Note: The tee_pad must be unlinked from any downstream elements
 * before calling this function. If still linked, GStreamer will
 * automatically unlink it.
 */
gboolean live_tee_release_pad(GstElement *tee_element, GstPad *tee_pad);

#endif // LIVE_TEE_H
