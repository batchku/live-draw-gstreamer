/**
 * @file buffer_manager.c
 * @brief Ring buffer implementation for GPU-accelerated video frame storage
 *
 * Manages GPU memory allocation and ring buffer operations for recorded video frames.
 * Implements circular buffering with automatic memory exhaustion handling by discarding
 * oldest frames when buffer capacity is exceeded.
 */

#include "buffer_manager.h"
#include "../utils/logging.h"
#include <gst/gst.h>
#include <stdlib.h>
#include <string.h>

/**
 * buffer_create - Allocate a GPU ring buffer for storing video frames
 * @max_frames: Maximum number of frames the buffer can hold
 * @caps: GStreamer caps describing frame format (resolution, format, etc.)
 *
 * Allocates GPU memory for a ring buffer capable of storing up to max_frames frames.
 * The buffer is empty initially; frames are added via buffer_write_frame().
 *
 * Returns: Pointer to allocated RingBuffer on success, NULL on memory allocation failure
 */
RingBuffer *buffer_create(guint max_frames, GstCaps *caps)
{
    LOG_DEBUG("Creating ring buffer for %u frames", max_frames);

    if (max_frames == 0) {
        LOG_ERROR("Cannot create ring buffer with zero capacity");
        return NULL;
    }

    RingBuffer *buf = malloc(sizeof(RingBuffer));
    if (!buf) {
        LOG_ERROR("Failed to allocate RingBuffer structure");
        return NULL;
    }

    memset(buf, 0, sizeof(RingBuffer));

    /* Allocate array of buffer pointers for ring storage */
    buf->frames = calloc(max_frames, sizeof(GstBuffer *));
    if (!buf->frames) {
        LOG_ERROR("Failed to allocate frame storage array for %u frames", max_frames);
        free(buf);
        return NULL;
    }

    buf->capacity = max_frames;
    buf->write_pos = 0;
    buf->read_pos = 0;
    buf->frame_count = 0;
    buf->duration_us = 0;
    buf->caps = caps;
    buf->overflow_count = 0;
    buf->total_frames_written = 0;

    LOG_INFO("Ring buffer created: capacity=%u frames", max_frames);
    return buf;
}

/**
 * buffer_write_frame - Write a frame to the ring buffer
 * @buf: Ring buffer to write to
 * @frame: GStreamer buffer containing the video frame
 *
 * Writes a frame to the current write position in the ring buffer. If the buffer
 * is at capacity, the oldest frame is discarded (overwritten) to make space.
 * Automatically updates frame count, duration, and ring positions.
 *
 * Increments the frame counter and updates total duration based on frame PTS/duration.
 * If buffer is full, oldest frame is unreferenced before being overwritten.
 *
 * Error Handling:
 * - NULL buf or frame: Silently ignored (safety check)
 * - Memory exhaustion: Oldest frame is automatically discarded
 */
void buffer_write_frame(RingBuffer *buf, GstBuffer *frame)
{
    if (!buf || !frame) {
        if (!buf) {
            LOG_WARNING("buffer_write_frame called with NULL buffer");
        } else {
            LOG_WARNING("buffer_write_frame called with NULL frame");
        }
        return;
    }

    LOG_DEBUG("Writing frame to buffer at position %u (count=%u, capacity=%u)", buf->write_pos,
              buf->frame_count, buf->capacity);

    /* Increment total frames written counter (for diagnostics) */
    buf->total_frames_written++;

    /* If buffer is at capacity, we need to drop the oldest frame (overflow) */
    if (buf->frame_count >= buf->capacity) {
        /* Unreference the frame being overwritten to release its GPU memory */
        if (buf->frames[buf->write_pos]) {
            gst_buffer_unref(buf->frames[buf->write_pos]);
            buf->frames[buf->write_pos] = NULL;
        }
        /* Increment overflow counter for diagnostics */
        buf->overflow_count++;
        LOG_WARNING("Buffer overflow: discarded oldest frame at position %u (total overflows: %u)",
                    buf->write_pos, buf->overflow_count);
    } else {
        /* Buffer not yet full, increment count */
        buf->frame_count++;
    }

    /* Reference the new frame and store it */
    gst_buffer_ref(frame);
    buf->frames[buf->write_pos] = frame;

    /* Update duration: sum of all frame durations */
    if (GST_BUFFER_DURATION_IS_VALID(frame)) {
        buf->duration_us += GST_BUFFER_DURATION(frame) / 1000; /* Convert ns to us */
    } else {
        /* If duration not set, assume 33ms per frame (~30fps) */
        buf->duration_us += 33000;
    }

    /* Advance write position with wraparound */
    buf->write_pos = (buf->write_pos + 1) % buf->capacity;

    LOG_DEBUG("Frame written successfully (frame_count=%u, duration_us=%lu, total_written=%u, "
              "overflows=%u)",
              buf->frame_count, buf->duration_us, buf->total_frames_written, buf->overflow_count);
}

/**
 * buffer_read_frame - Read a specific frame from the ring buffer
 * @buf: Ring buffer to read from
 * @frame_index: Index of the frame to retrieve (0 = first recorded, increments forward)
 *
 * Retrieves the frame at the specified index from the ring buffer.
 * Frames are indexed from 0 (oldest available frame) to frame_count-1 (newest).
 *
 * Returns: Pointer to GstBuffer at the specified index, or NULL if:
 *   - buf is NULL
 *   - frame_index is out of bounds (>= frame_count)
 *   - The requested frame is invalid
 *
 * Note: The returned buffer is owned by the ring buffer; do NOT unreference it.
 * The buffer remains valid as long as this ring buffer is not cleaned up.
 */
GstBuffer *buffer_read_frame(RingBuffer *buf, guint frame_index)
{
    if (!buf) {
        LOG_WARNING("buffer_read_frame called with NULL buffer");
        return NULL;
    }

    if (frame_index >= buf->frame_count) {
        LOG_WARNING("Requested frame index %u out of bounds (frame_count=%u)", frame_index,
                    buf->frame_count);
        return NULL;
    }

    /* Calculate actual position in ring array */
    guint actual_pos;
    if (buf->frame_count < buf->capacity) {
        /* Buffer not yet full: frames are stored sequentially from position 0 */
        actual_pos = frame_index;
    } else {
        /* Buffer is full: calculate offset from write_pos */
        actual_pos = (buf->write_pos + frame_index) % buf->capacity;
    }

    GstBuffer *frame = buf->frames[actual_pos];

    if (!frame) {
        LOG_ERROR("Frame at index %u (actual_pos=%u) is NULL", frame_index, actual_pos);
        return NULL;
    }

    return frame;
}

/**
 * buffer_get_frame_count - Get the number of frames in the buffer
 * @buf: Ring buffer to query
 *
 * Returns the total number of frames currently stored in the ring buffer.
 * This is capped at the buffer's capacity; older frames are discarded when full.
 *
 * Returns: Frame count (0 if buf is NULL or buffer is empty)
 */
guint buffer_get_frame_count(RingBuffer *buf)
{
    if (!buf) {
        LOG_WARNING("buffer_get_frame_count called with NULL buffer");
        return 0;
    }
    return buf->frame_count;
}

/**
 * buffer_get_duration - Get the total duration of all frames in the buffer
 * @buf: Ring buffer to query
 *
 * Returns the sum of the durations of all frames currently in the buffer.
 * This represents the total playback time if all frames are played sequentially.
 *
 * Returns: Total duration in microseconds (0 if buf is NULL or buffer is empty)
 */
guint64 buffer_get_duration(RingBuffer *buf)
{
    if (!buf) {
        LOG_WARNING("buffer_get_duration called with NULL buffer");
        return 0;
    }
    return buf->duration_us;
}

/**
 * buffer_cleanup - Free all resources associated with a ring buffer
 * @buf: Ring buffer to clean up
 *
 * Unreferences all frames stored in the buffer and frees the ring buffer structure.
 * Should be called when the buffer is no longer needed.
 *
 * Error Handling: If buf is NULL, function returns silently.
 */
void buffer_cleanup(RingBuffer *buf)
{
    if (!buf) {
        LOG_WARNING("buffer_cleanup called with NULL buffer");
        return;
    }

    LOG_DEBUG("Cleaning up ring buffer (frame_count=%u, capacity=%u)", buf->frame_count,
              buf->capacity);

    /* Unreference all stored frames to release GPU memory */
    if (buf->frames) {
        for (guint i = 0; i < buf->capacity; i++) {
            if (buf->frames[i]) {
                gst_buffer_unref(buf->frames[i]);
                buf->frames[i] = NULL;
            }
        }
        free(buf->frames);
        buf->frames = NULL;
    }

    /* Unreference caps if present */
    if (buf->caps) {
        gst_caps_unref(buf->caps);
        buf->caps = NULL;
    }

    /* Free the structure itself */
    free(buf);

    LOG_INFO("Ring buffer cleaned up and freed");
}

/**
 * buffer_get_overflow_count - Get number of buffer overflow events
 * @buf: Ring buffer to query
 *
 * Returns the number of times the buffer reached capacity and had to discard
 * the oldest frame. This is useful for diagnostics and monitoring memory
 * exhaustion during long recording sessions.
 *
 * Returns: Overflow count (0 if buf is NULL or no overflows occurred)
 */
guint buffer_get_overflow_count(RingBuffer *buf)
{
    if (!buf) {
        LOG_WARNING("buffer_get_overflow_count called with NULL buffer");
        return 0;
    }
    return buf->overflow_count;
}

/**
 * buffer_get_total_frames_written - Get total frames written to buffer
 * @buf: Ring buffer to query
 *
 * Returns the total number of frames that have been written to this buffer
 * since it was created, including frames that were later overwritten due to
 * capacity limits. This differs from buffer_get_frame_count() which is capped
 * at the buffer's capacity.
 *
 * This metric is useful for understanding buffer churn and efficiency:
 * - total_written = frame_count only when buffer has never overflowed
 * - total_written > frame_count when overflows have occurred
 *
 * Returns: Total frames written (0 if buf is NULL)
 */
guint buffer_get_total_frames_written(RingBuffer *buf)
{
    if (!buf) {
        LOG_WARNING("buffer_get_total_frames_written called with NULL buffer");
        return 0;
    }
    return buf->total_frames_written;
}
