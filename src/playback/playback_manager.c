/**
 * @file playback_manager.c
 * @brief Palindrome playback implementation for video looping
 *
 * Implements the palindrome playback algorithm that cycles through recorded
 * video frames in forward direction, then reverse direction, then repeats.
 * This creates a smooth looping effect without visible jumps or frame repetition.
 *
 * Algorithm:
 * - Forward: frames 0→1→2→...→(N-1)→N
 * - Reverse: frames N→(N-1)→...→2→1 (note: N not repeated again)
 * - Loop: 0→1→...→N→(N-1)→...→1→0→1→2→...
 */

#include "playback_manager.h"
#include "../utils/logging.h"
#include "playback_bin.h"
#include <stdlib.h>
#include <string.h>

/**
 * playback_loop_create - Initialize palindrome playback loop from recorded buffer
 *
 * Creates a new PlaybackLoop state machine initialized to:
 * - current_frame = 0
 * - direction = PLAYBACK_STATE_FORWARD
 * - is_playing = TRUE (unless buffer is empty)
 *
 * The loop does not take ownership of the RingBuffer; the caller must
 * keep the buffer alive for the duration of playback loop usage.
 */
PlaybackLoop *playback_loop_create(RingBuffer *recorded_buffer)
{
    if (!recorded_buffer) {
        LOG_ERROR("playback_loop_create: recorded_buffer is NULL");
        return NULL;
    }

    PlaybackLoop *loop = (PlaybackLoop *) malloc(sizeof(PlaybackLoop));
    if (!loop) {
        LOG_ERROR("playback_loop_create: malloc failed");
        return NULL;
    }

    memset(loop, 0, sizeof(PlaybackLoop));

    /* Initialize state */
    loop->buffer = recorded_buffer;
    loop->current_frame = 0;
    loop->direction = PLAYBACK_STATE_FORWARD;
    loop->total_frames = buffer_get_frame_count(recorded_buffer);

    /* Mark as playing only if buffer has frames */
    if (loop->total_frames > 0) {
        loop->is_playing = TRUE;
        LOG_DEBUG("Playback loop created: %u frames, starting at frame 0 (forward)",
                  loop->total_frames);
    } else {
        loop->is_playing = FALSE;
        LOG_WARNING("Playback loop created with empty buffer (0 frames)");
    }

    return loop;
}

/**
 * playback_advance_frame - Step through palindrome sequence
 *
 * Advances playback to the next frame according to the palindrome algorithm.
 *
 * Palindrome algorithm for N frames (0 to N-1):
 *
 * State: (frame_index, direction)
 * FORWARD phase:  (0,F) → (1,F) → (2,F) → ... → (N-1,F)
 * At (N-1,F), next advance switches to REVERSE
 * REVERSE phase:  (N-1,R) → (N-2,R) → ... → (1,R) → (0,R)
 * At (0,R), next advance switches to FORWARD
 * Loop: (0,F) → (1,F) → ... → (N-1,F) → (N-1,R) → ... → (0,R) → (0,F) → ...
 *
 * Note: Frame N-1 appears once per forward pass, frame 0 appears once per
 * cycle (at the reverse-to-forward transition point). No frame is repeated.
 */
void playback_advance_frame(PlaybackLoop *loop)
{
    if (!loop || !loop->is_playing || loop->total_frames == 0) {
        return;
    }

    if (loop->total_frames == 1) {
        /* Special case: single frame stays at frame 0 forever */
        return;
    }

    if (loop->direction == PLAYBACK_STATE_FORWARD) {
        /* In forward phase, increment frame index */
        loop->current_frame++;

        /* Check if we've reached the end of the buffer */
        if (loop->current_frame >= loop->total_frames - 1) {
            /* We're now at or beyond the last frame (N-1)
               Switch to reverse direction but stay at the same frame */
            loop->direction = PLAYBACK_STATE_REVERSE;
            /* current_frame is now N-1 (or possibly N if we over-incremented)
               Set it explicitly to N-1 to be safe */
            if (loop->current_frame > loop->total_frames - 1) {
                loop->current_frame = loop->total_frames - 1;
            }

            LOG_DEBUG("Playback direction changed to REVERSE at frame %u", loop->current_frame);
        }
    } else {
        /* In reverse phase, decrement frame index */
        loop->current_frame--;

        /* Check if we've reached the start of the sequence */
        if (loop->current_frame == 0) {
            /* We're at frame 0, switch back to forward direction
               The next advance will move to frame 1 */
            loop->direction = PLAYBACK_STATE_FORWARD;

            LOG_DEBUG("Playback direction changed to FORWARD at frame %u", loop->current_frame);
        }
    }
}

/**
 * playback_get_next_frame - Retrieve frame at current playback position
 *
 * Returns the GStreamer buffer for the frame at the current_frame index.
 * This is the frame to render in the current cycle. Call playback_advance_frame()
 * after rendering to move to the next frame.
 *
 * Error handling:
 * - NULL loop: logs error, returns NULL
 * - NULL buffer: logs error, returns NULL
 * - current_frame out of bounds: logs error, returns NULL
 * - buffer_read_frame returns NULL: logs error, returns NULL
 */
GstBuffer *playback_get_next_frame(PlaybackLoop *loop)
{
    if (!loop) {
        LOG_ERROR("playback_get_next_frame: loop is NULL");
        return NULL;
    }

    if (!loop->buffer) {
        LOG_ERROR("playback_get_next_frame: buffer is NULL");
        return NULL;
    }

    if (loop->current_frame >= loop->total_frames) {
        LOG_ERROR("playback_get_next_frame: current_frame (%u) >= total_frames (%u)",
                  loop->current_frame, loop->total_frames);
        return NULL;
    }

    GstBuffer *frame = buffer_read_frame(loop->buffer, loop->current_frame);
    if (!frame) {
        LOG_ERROR("playback_get_next_frame: buffer_read_frame returned NULL for frame %u",
                  loop->current_frame);
        return NULL;
    }

    return frame;
}

/**
 * playback_get_direction - Query current playback direction
 *
 * Returns the direction of playback (forward or reverse).
 * Useful for debugging or monitoring playback state.
 */
PlaybackDirection playback_get_direction(PlaybackLoop *loop)
{
    if (!loop) {
        return PLAYBACK_STATE_FORWARD;
    }
    return loop->direction;
}

/**
 * playback_is_playing - Check if playback is active
 *
 * Returns whether the loop is currently playing (is_playing = TRUE).
 * A loop may be inactive if it was created with an empty buffer.
 */
gboolean playback_is_playing(PlaybackLoop *loop)
{
    if (!loop) {
        return FALSE;
    }
    return loop->is_playing;
}

/**
 * playback_loop_cleanup - Free playback loop resources
 *
 * Frees the PlaybackLoop structure. Does NOT free the RingBuffer
 * (caller retains responsibility for the buffer's lifetime).
 *
 * After calling this, the loop pointer is invalid and must not be used.
 */
void playback_loop_cleanup(PlaybackLoop *loop)
{
    if (!loop) {
        return;
    }
    LOG_DEBUG("Cleaning up playback loop");
    free(loop);
}

/**
 * playback_create_bin - Create a playback bin for videomixer integration
 *
 * This is the high-level factory function that:
 * 1. Creates a PlaybackBin from the RingBuffer
 * 2. Returns the GStreamer bin element
 *
 * The caller must add this element to the pipeline and link it to the videomixer
 * after configuring the videomixer pad properties (xpos, ypos, width, zorder, alpha).
 *
 * Error handling:
 * - Invalid buffer: logs error, returns NULL
 * - Invalid cell_number: logs error, returns NULL
 * - PlaybackBin creation fails: logs error, returns NULL
 *
 * Memory: The returned element is owned by the caller. The internal PlaybackBin
 * structure is managed internally and freed when the element is destroyed.
 */
GstElement *playback_create_bin(RingBuffer *buffer, int cell_number)
{
    if (!buffer) {
        LOG_ERROR("playback_create_bin: buffer is NULL");
        return NULL;
    }

    if (cell_number < 2 || cell_number > 10) {
        LOG_ERROR("playback_create_bin: invalid cell_number %d (must be 2-10)", cell_number);
        return NULL;
    }

    /* Create PlaybackBin using the existing factory function.
       Pass NULL for output_caps since caps negotiation happens at videomixer link time */
    PlaybackBin *pbin = playback_bin_create(cell_number, buffer, NULL);
    if (!pbin) {
        LOG_ERROR("playback_create_bin: failed to create PlaybackBin for cell %d", cell_number);
        return NULL;
    }

    LOG_INFO("Created playback bin for cell %d ready for videomixer integration", cell_number);

    /* Return the GStreamer bin element.
       The PlaybackBin structure pointer is embedded in the bin's user data
       for potential retrieval later, but the caller only needs the element. */
    return pbin->bin;
}
