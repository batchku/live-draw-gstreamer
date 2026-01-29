/**
 * @file playback_manager.h
 * @brief Palindrome playback loop management for video looping
 *
 * Implements palindrome playback algorithm where video plays forward from
 * frame 0 to N, then backward from N-1 to 0, then repeats. Manages playback
 * state (current frame, direction) and provides frame-by-frame progression.
 *
 * Palindrome sequence for N frames:
 *   Forward: 0→1→2→...→(N-1)→N
 *   Reverse: N→(N-1)→...→2→1
 *   Loop: 0→1→2→...→N→(N-1)→...→1→0→1→2→...
 *
 * Note: Frame 0 and frame N are not repeated; the sequence avoids:
 *   WRONG: 0→1→2→N→N→(N-1)→1→0→0→1  (frame 0 and N repeated)
 *   RIGHT: 0→1→2→N→(N-1)→1→0→1→2  (smooth palindrome)
 */

#ifndef PLAYBACK_MANAGER_H
#define PLAYBACK_MANAGER_H

#include "../recording/buffer_manager.h"
#include <glib.h>
#include <gst/gst.h>

/**
 * PlaybackDirection - Direction of playback progression
 * @PLAYBACK_STATE_FORWARD: Playing forward through frames (0→1→2→...→N)
 * @PLAYBACK_STATE_REVERSE: Playing backward through frames (N→(N-1)→...→0)
 */
typedef enum {
    PLAYBACK_STATE_FORWARD,
    PLAYBACK_STATE_REVERSE,
} PlaybackDirection;

/**
 * PlaybackLoop - State machine for palindrome video playback
 * @buffer: Pointer to RingBuffer containing recorded video frames
 * @current_frame: Index of frame to be output next (0 to total_frames-1)
 * @direction: Forward or reverse playback direction
 * @total_frames: Total number of frames in the buffer
 * @is_playing: Whether playback is currently active
 *
 * Maintains the playback state for a single video loop. Tracks the current
 * frame index, direction (forward/reverse), and provides frame advancement
 * for each render cycle.
 */
typedef struct {
    RingBuffer *buffer;
    guint current_frame;
    PlaybackDirection direction;
    guint total_frames;
    gboolean is_playing;
} PlaybackLoop;

/**
 * playback_loop_create - Create a new palindrome playback loop from a recorded buffer
 * @recorded_buffer: RingBuffer containing video frames to loop
 *
 * Initializes a PlaybackLoop from a RingBuffer of recorded frames. The playback
 * loop starts at frame 0 in forward direction. The loop does not take ownership
 * of the buffer; the caller remains responsible for managing its lifetime.
 *
 * Error handling:
 * - If recorded_buffer is NULL: logs error, returns NULL
 * - If buffer has 0 frames: logs warning, returns loop but marks as not playing
 * - If malloc fails: logs error, returns NULL
 *
 * Returns: Pointer to new PlaybackLoop on success, NULL on failure
 *
 * Ownership: Caller must free with playback_loop_cleanup()
 */
PlaybackLoop *playback_loop_create(RingBuffer *recorded_buffer);

/**
 * playback_advance_frame - Progress playback by one frame (called each render cycle)
 * @loop: PlaybackLoop to advance
 *
 * Increments the frame index according to the current direction (forward or reverse).
 * When reaching the end of forward playback, switches to reverse direction without
 * repeating the final frame. When reaching the start during reverse, switches back
 * to forward without repeating frame 0.
 *
 * Palindrome state machine:
 *   Forward (0 to N-1):
 *     - current_frame++
 *     - If current_frame >= total_frames: direction = REVERSE, current_frame = total_frames - 2
 *   Reverse (N-1 down to 1):
 *     - current_frame--
 *     - If current_frame < 1: direction = FORWARD, current_frame = 1
 *
 * Error handling: If loop is NULL, silently ignored (safety check)
 */
void playback_advance_frame(PlaybackLoop *loop);

/**
 * playback_get_next_frame - Retrieve the current frame for rendering
 * @loop: PlaybackLoop to get frame from
 *
 * Returns the GStreamer buffer for the frame at current_frame index.
 * This is the frame that should be rendered in the current cycle BEFORE
 * calling playback_advance_frame().
 *
 * Error handling:
 * - If loop is NULL: logs error, returns NULL
 * - If buffer is NULL: logs error, returns NULL
 * - If current_frame is out of bounds: logs error, returns NULL (frame index corrupted)
 *
 * Ownership: The returned buffer is owned by the RingBuffer. Do NOT unreference it.
 * The buffer remains valid while the RingBuffer is alive.
 *
 * Returns: Pointer to GstBuffer at current_frame, or NULL on error
 */
GstBuffer *playback_get_next_frame(PlaybackLoop *loop);

/**
 * playback_get_direction - Query current playback direction
 * @loop: PlaybackLoop to query
 *
 * Returns the current direction (PLAYBACK_STATE_FORWARD or PLAYBACK_STATE_REVERSE).
 * Useful for debugging or displaying playback state.
 *
 * Error handling: If loop is NULL, returns PLAYBACK_STATE_FORWARD (safe default)
 *
 * Returns: Current PlaybackDirection
 */
PlaybackDirection playback_get_direction(PlaybackLoop *loop);

/**
 * playback_is_playing - Check if playback loop is active
 * @loop: PlaybackLoop to query
 *
 * Returns TRUE if the playback loop is currently playing (is_playing = TRUE).
 * A loop may be paused (is_playing = FALSE) if created with an empty buffer.
 *
 * Error handling: If loop is NULL, returns FALSE
 *
 * Returns: TRUE if playing, FALSE otherwise
 */
gboolean playback_is_playing(PlaybackLoop *loop);

/**
 * playback_loop_cleanup - Free a playback loop and clean up resources
 * @loop: PlaybackLoop to clean up
 *
 * Frees the PlaybackLoop structure. Does NOT free the RingBuffer
 * (caller retains responsibility for buffer lifetime).
 *
 * Error handling: If loop is NULL, function returns silently
 */
void playback_loop_cleanup(PlaybackLoop *loop);

/**
 * playback_create_bin - Create and configure a playback bin for the videomixer
 *
 * This is the high-level factory function that combines:
 * 1. Creating a PlaybackBin from a recorded RingBuffer
 * 2. Returning the GStreamer bin element for pipeline integration
 *
 * The returned element is a GStreamer bin containing appsrc and queue,
 * ready to be added to the pipeline and linked to the videomixer.
 *
 * Videomixer pad properties that the caller MUST configure:
 * - xpos: X position in grid (0 for cell 1, 320 for cell 2, 640 for cell 3, etc.)
 * - ypos: 0 (all cells in same row)
 * - width: 320 pixels (standard cell width)
 * - zorder: Stack order (0 for live feed, 1-9 for playback loops)
 * - alpha: 1.0 (fully opaque)
 *
 * Example usage:
 * @code
 *   // Create playback bin from recorded buffer
 *   GstElement *playback_bin = playback_create_bin(recorded_buffer, 2);  // cell 2
 *
 *   // Add to pipeline
 *   gst_bin_add(GST_BIN(pipeline), playback_bin);
 *
 *   // Get videomixer sink pad for cell 2 (pad index 1)
 *   GstPad *mixer_pad = gst_element_get_request_pad(videomixer, "sink_%u");
 *
 *   // Configure pad properties for cell 2
 *   g_object_set(mixer_pad,
 *                "xpos", 320,      // Cell 2 starts at x=320
 *                "ypos", 0,
 *                "width", 320,
 *                "zorder", 1,
 *                "alpha", 1.0,
 *                NULL);
 *
 *   // Link playback_bin source to videomixer sink
 *   gst_element_link(playback_bin, videomixer);
 * @endcode
 *
 * @param buffer        RingBuffer containing recorded video frames
 * @param cell_number   Grid cell number (2-10) for this playback
 *
 * @return              GStreamer bin element on success (ready for pipeline integration)
 *                      NULL on failure
 *
 * Error handling:
 * - If buffer is NULL: logs error, returns NULL
 * - If cell_number is out of range (not 2-10): logs error, returns NULL
 * - If playback bin creation fails: logs error, returns NULL
 *
 * Memory ownership:
 * - Returned element is owned by the caller
 * - Caller must add it to pipeline and eventually unref it
 * - Internal PlaybackBin structure is managed internally and freed on cleanup
 * - RingBuffer ownership remains with caller (not transferred)
 *
 * Note: The returned bin is NOT yet linked to the videomixer.
 * The caller is responsible for linking it using gst_element_link() or manual pad linking
 * after configuring videomixer pad properties.
 */
GstElement *playback_create_bin(RingBuffer *buffer, int cell_number);

#endif /* PLAYBACK_MANAGER_H */
