/**
 * @file playback_bin.h
 * @brief GStreamer playback bin factory for emitting recorded video frames.
 *
 * Provides functionality to create and manage GStreamer "bins" (sub-pipelines) that
 * emit video frames from a PlaybackLoop (with palindrome playback) to the videomixer.
 *
 * Architecture:
 * - Each playback bin contains: appsrc → queue → output pad
 * - The appsrc element pulls frames from a PlaybackLoop via callbacks
 * - Frames are advanced through palindrome playback on each pull
 * - Queue buffers frames before delivery to videomixer
 * - Pad probes track playback state for debugging
 *
 * Playback lifecycle:
 * - Created when a recording completes and playback should begin
 * - Runs continuously, cycling through palindrome sequence
 * - Removed when user starts a new recording in the same cell
 */

#ifndef PLAYBACK_BIN_H
#define PLAYBACK_BIN_H

#include "playback_manager.h"
#include "../app/app_config.h"
#include <gst/gst.h>

/**
 * PlaybackBin - A GStreamer bin for emitting video frames from a palindrome loop.
 *
 * @bin: The GStreamer bin containing appsrc and queue
 * @appsrc: Application source element that pulls frames from PlaybackLoop
 * @queue: Queue element for buffering frames
 * @playback_loop: PlaybackLoop providing frames with palindrome state management
 * @cell_number: Layer number (1-50) for this playback
 * @frame_count: Total frames emitted (for statistics)
 * @is_active: Boolean indicating if playback is currently running
 */
typedef struct {
    GstElement *bin;             /**< GStreamer bin container */
    GstElement *appsrc;          /**< Application source element */
    GstElement *queue;           /**< Queue element for buffering */
    PlaybackLoop *playback_loop; /**< Palindrome playback state */
    int cell_number;             /**< Layer (1-50) for this playback */
    guint frame_count;           /**< Total frames emitted */
    gboolean is_active;          /**< Playback active flag */
    GstClockTime next_pts;       /**< Monotonic PTS for playback */
    GstClockTime frame_duration; /**< Frame duration for playback */
} PlaybackBin;

/**
 * playback_bin_create - Create a playback bin for emitting video frames from a loop.
 *
 * Constructs a GStreamer bin with appsrc and queue elements.
 * Initializes a PlaybackLoop from the provided RingBuffer.
 * Sets up need-data callbacks to emit frames on-demand.
 *
 * The bin is NOT added to the pipeline by this function.
 * The caller must add it to the pipeline using gst_bin_add().
 *
 * @param cell_number     Layer number (1-50) where this playback will display
 * @param buffer          RingBuffer containing recorded video frames
 * @param output_caps     GStreamer caps describing output format (must match videomixer)
 *
 * @return                Pointer to PlaybackBin struct on success, NULL on failure
 *
 * Error handling:
 * - If element creation fails: logs error and returns NULL
 * - If playback loop creation fails: logs error and returns NULL
 * - If callbacks fail to attach: logs error and returns NULL
 *
 * Memory ownership:
 * - Returned PlaybackBin must be freed with playback_bin_cleanup()
 * - bin element must be unreferenced after removal from pipeline
 * - buffer ownership remains with caller (PlaybackBin doesn't take ownership)
 */
PlaybackBin *playback_bin_create(int cell_number, RingBuffer *buffer, GstCaps *output_caps);

/**
 * playback_bin_is_active - Check if this playback bin is currently active.
 *
 * @param pbin  Playback bin to query
 * @return      TRUE if playback is active, FALSE otherwise
 */
gboolean playback_bin_is_active(PlaybackBin *pbin);

/**
 * playback_bin_get_frame_count - Get the number of frames emitted by this playback.
 *
 * Useful for statistics and monitoring playback progress.
 *
 * @param pbin  Playback bin to query
 * @return      Number of frames emitted, or 0 if pbin is NULL
 */
guint playback_bin_get_frame_count(PlaybackBin *pbin);

/**
 * playback_bin_cleanup - Free a playback bin and release resources.
 *
 * Removes callbacks, unreferences all GStreamer elements,
 * cleans up the playback loop, and frees the PlaybackBin struct.
 * After calling this, pbin is invalid.
 *
 * Error handling: Safe to call with NULL.
 *
 * @param pbin  Playback bin to clean up (may be NULL)
 *
 * Note: The caller is responsible for removing the bin from the pipeline
 * before calling this function.
 */
void playback_bin_cleanup(PlaybackBin *pbin);

#endif // PLAYBACK_BIN_H
