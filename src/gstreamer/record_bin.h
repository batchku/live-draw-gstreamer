/**
 * @file record_bin.h
 * @brief GStreamer recording bin factory for capturing video frames to GPU buffers.
 *
 * Provides functionality to create and manage GStreamer "bins" (sub-pipelines) that
 * capture video frames from the live stream and store them in GPU memory ring buffers.
 *
 * Architecture:
 * - Each record bin contains: queue → capsfilter → fakesink
 * - A pad probe on the queue's sink pad intercepts frames
 * - Frames are written to a RingBuffer for storage and later playback
 * - Pad probes allow frame capture without modifying the pipeline element chain
 *
 * Signal-based control:
 * - Recording starts when the application sends a start signal
 * - Recording stops when the application sends a stop signal
 * - Probes remain attached to the bin even when recording is paused
 */

#ifndef RECORD_BIN_H
#define RECORD_BIN_H

#include "../recording/buffer_manager.h"
#include <gst/gst.h>

/**
 * RecordBin - A GStreamer bin for capturing video frames to a ring buffer.
 *
 * @bin: The GStreamer bin containing queue, capsfilter, fakesink
 * @queue: Queue element for buffering frames
 * @capsfilter: Capsfilter element for format negotiation
 * @fakesink: Fakesink element that consumes buffers after probe capture
 * @ring_buffer: Ring buffer to store captured frames (GPU memory)
 * @is_recording: Boolean indicating if frames are currently being captured
 * @key_number: Key number (1-9) for keyboard mapping
 * @probe_id: ID of the pad probe for unregistering later
 * @probe_pad: Pad where the probe is attached (weak reference)
 */
typedef struct {
    GstElement *bin;         /**< GStreamer bin container */
    GstElement *queue;       /**< Queue element */
    GstElement *capsfilter;  /**< Capsfilter for format negotiation */
    GstElement *fakesink;    /**< Fakesink for consuming buffers */
    RingBuffer *ring_buffer; /**< Ring buffer for frame storage */
    gboolean is_recording;   /**< Recording active flag */
    int key_number;          /**< Key number for this recording (1-9) */
    gulong probe_id;         /**< Probe ID for cleanup */
    GstPad *probe_pad;       /**< Pad where probe is attached */
    GstPad *tee_pad;         /**< Pad from tee element (for proper cleanup) */
} RecordBin;

/**
 * record_bin_create - Create a recording bin for capturing video frames.
 *
 * Constructs a GStreamer bin with queue, capsfilter, and fakesink elements.
 * Attaches a pad probe to the queue's sink pad for frame capture.
 * Allocates a ring buffer for storing captured frames.
 *
 * The bin is NOT added to the pipeline by this function.
 * The caller must add it to the pipeline using gst_bin_add().
 *
 * @param key_number  Key number (1-9) for keyboard mapping
 * @param max_frames  Maximum frames to store in ring buffer (e.g., 60 for 2 seconds @ 30fps)
 * @param caps        GStreamer caps describing frame format (can be NULL initially)
 *
 * @return            Pointer to RecordBin struct on success, NULL on failure
 *
 * Error handling:
 * - If element creation fails: logs error and returns NULL
 * - If pad probe attachment fails: logs error and returns NULL
 * - If ring buffer creation fails: logs error and returns NULL
 *
 * Memory ownership:
 * - Returned RecordBin must be freed with record_bin_cleanup()
 * - bin element must be unreferenced after removal from pipeline
 */
RecordBin *record_bin_create(int key_number, guint max_frames, GstCaps *caps);

/**
 * record_bin_start_recording - Enable frame capture for this recording bin.
 *
 * Sets the is_recording flag to TRUE, causing the pad probe to capture frames
 * into the ring buffer. Call this when the user presses a recording key.
 *
 * @param rbin  Record bin to start
 * @return      TRUE on success, FALSE if rbin is NULL
 *
 * Note: Recording can be started/stopped multiple times without recreating the bin.
 * Frames accumulate in the ring buffer across multiple start/stop cycles.
 */
gboolean record_bin_start_recording(RecordBin *rbin);

/**
 * record_bin_stop_recording - Disable frame capture for this recording bin.
 *
 * Sets the is_recording flag to FALSE, stopping frame capture into the ring buffer.
 * Call this when the user releases a recording key.
 * The ring buffer retains all captured frames for playback.
 *
 * @param rbin  Record bin to stop
 * @return      TRUE on success, FALSE if rbin is NULL
 */
gboolean record_bin_stop_recording(RecordBin *rbin);

/**
 * record_bin_is_recording - Check if this record bin is currently recording.
 *
 * @param rbin  Record bin to query
 * @return      TRUE if recording is active, FALSE otherwise
 */
gboolean record_bin_is_recording(RecordBin *rbin);

/**
 * record_bin_get_buffer - Retrieve the ring buffer from this record bin.
 *
 * Returns a reference to the ring buffer containing captured frames.
 * The buffer is owned by the RecordBin; do not free it.
 *
 * @param rbin  Record bin
 * @return      Pointer to RingBuffer, or NULL if rbin is NULL
 */
RingBuffer *record_bin_get_buffer(RecordBin *rbin);

/**
 * record_bin_reset - Clear the ring buffer and prepare for new recording.
 *
 * Empties the ring buffer, discarding all previously captured frames.
 * Used when restarting a recording after previous content.
 *
 * @param rbin  Record bin to reset
 * @return      TRUE on success, FALSE if rbin is NULL
 *
 * Note: This does NOT change the is_recording state.
 */
gboolean record_bin_reset(RecordBin *rbin);

/**
 * record_bin_cleanup - Free a recording bin and release resources.
 *
 * Removes the pad probe, unreferences all GStreamer elements,
 * cleans up the ring buffer, and frees the RecordBin struct.
 * After calling this, rbin is invalid.
 *
 * Error handling: Safe to call with NULL.
 *
 * @param rbin  Record bin to clean up (may be NULL)
 *
 * Note: The caller is responsible for removing the bin from the pipeline
 * before calling this function.
 */
void record_bin_cleanup(RecordBin *rbin);

#endif // RECORD_BIN_H
