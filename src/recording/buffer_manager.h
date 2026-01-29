/**
 * @file buffer_manager.h
 * @brief GPU ring buffer management for recorded video frames
 *
 * Provides a GPU-accelerated ring buffer implementation for storing video frames
 * during recording. Frames are stored as GStreamer GstBuffer objects on GPU memory
 * with automatic memory exhaustion handling (oldest frames discarded when full).
 */

#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include <gst/gst.h>

/**
 * RingBuffer - Circular buffer for storing video frames on GPU memory
 * @frames: Array of GstBuffer pointers (GPU memory)
 * @capacity: Maximum number of frames the buffer can hold
 * @write_pos: Current write position in the ring
 * @read_pos: Current read position (unused; frames read by explicit index)
 * @frame_count: Number of frames currently stored (capped at capacity)
 * @duration_us: Total duration of all stored frames in microseconds
 * @caps: GStreamer caps describing frame format (resolution, color space, etc.)
 * @overflow_count: Number of times buffer capacity was exceeded (frames discarded)
 * @total_frames_written: Total frames written since creation (for diagnostics)
 *
 * Implements a fixed-size circular buffer for storing video frames. When the buffer
 * reaches capacity, new frames overwrite the oldest frames. All frames are stored
 * in GPU memory via GStreamer's memory management system.
 *
 * Edge Case Handling:
 * - Buffer overflow: Oldest frame automatically discarded, overflow_count incremented
 * - Short recordings: Minimum 1 frame enforced at recording state level
 * - Multiple wraparounds: Circular cell assignment ensures 9-cell rotation
 *
 * Memory model:
 * - Each frame is a GstBuffer on GPU
 * - Ring size ≈ 60 frames per recording at 30 fps input = ~2 seconds
 * - GPU memory per recording ≈ 1920×1080×3 bytes × 60 frames ≈ 373 MB (RGB)
 * - Maximum 9 simultaneous recordings ≈ 3.4 GB (acceptable for modern Macs)
 */
typedef struct {
    GstBuffer **frames;         /**< Circular array of GStreamer buffers */
    guint capacity;             /**< Maximum frames in buffer */
    guint write_pos;            /**< Current write position (0 to capacity-1) */
    guint read_pos;             /**< Unused; provided for future expansion */
    guint frame_count;          /**< Current frame count (min 0, max capacity) */
    guint64 duration_us;        /**< Sum of frame durations in microseconds */
    GstCaps *caps;              /**< Frame format capabilities */
    guint overflow_count;       /**< Number of times capacity was exceeded */
    guint total_frames_written; /**< Total frames written (includes discarded) */
} RingBuffer;

/**
 * buffer_create - Allocate and initialize a GPU ring buffer
 * @max_frames: Maximum number of frames to store
 * @caps: GStreamer caps for frames (NULL if unknown; set later)
 *
 * Allocates a ring buffer capable of storing up to max_frames video frames
 * in GPU memory. Initial state is empty (frame_count = 0).
 *
 * Error handling:
 * - If max_frames is 0: returns NULL and logs error
 * - If malloc fails: returns NULL and logs error
 *
 * Returns: Pointer to RingBuffer on success, NULL on failure
 */
RingBuffer *buffer_create(guint max_frames, GstCaps *caps);

/**
 * buffer_write_frame - Write a video frame to the ring buffer
 * @buf: Target ring buffer
 * @frame: GStreamer buffer containing video frame
 *
 * Writes a frame to the current write position. If buffer is at capacity,
 * the oldest frame is automatically discarded. Increments frame_count and
 * updates duration based on frame duration property.
 *
 * Memory exhaustion handling:
 * - Buffer full: oldest frame unreferenced, new frame overwrites it
 * - Frame duration: if not set, assumes 33ms (~30 fps)
 *
 * Error handling:
 * - NULL buf or frame: silently ignored (safety check)
 *
 * Ownership: The buffer takes ownership of the frame (calls gst_buffer_ref).
 * The caller may unreference their reference after calling this function.
 */
void buffer_write_frame(RingBuffer *buf, GstBuffer *frame);

/**
 * buffer_read_frame - Retrieve a specific frame from the ring buffer
 * @buf: Source ring buffer
 * @frame_index: Index of frame to retrieve (0 = oldest, frame_count-1 = newest)
 *
 * Returns the frame at the specified index. Frames are indexed starting from
 * the oldest recorded frame (frame_index=0) through the newest (frame_count-1).
 *
 * Error handling:
 * - buf is NULL: returns NULL, logs warning
 * - frame_index >= frame_count: returns NULL, logs warning
 * - Internal frame is NULL: returns NULL, logs error
 *
 * Returns: Pointer to GstBuffer at the specified index, or NULL on error
 *
 * Ownership: The returned buffer is owned by the ring buffer. Do NOT unreference it.
 * The buffer remains valid while this RingBuffer is alive.
 */
GstBuffer *buffer_read_frame(RingBuffer *buf, guint frame_index);

/**
 * buffer_get_frame_count - Get number of frames currently in buffer
 * @buf: Ring buffer to query
 *
 * Returns the number of frames currently stored in the ring buffer.
 * This value ranges from 0 (empty) to capacity (full).
 *
 * Returns: Frame count, or 0 if buf is NULL
 */
guint buffer_get_frame_count(RingBuffer *buf);

/**
 * buffer_get_duration - Get total duration of frames in buffer
 * @buf: Ring buffer to query
 *
 * Returns the sum of the durations of all frames in the buffer.
 * This represents the total playback time for the recorded video.
 *
 * Returns: Total duration in microseconds, or 0 if buf is NULL
 */
guint64 buffer_get_duration(RingBuffer *buf);

/**
 * buffer_cleanup - Free ring buffer and release GPU memory
 * @buf: Ring buffer to clean up
 *
 * Unreferences all frames (releasing GPU memory), unreferences caps,
 * and frees the ring buffer structure. After calling this, buf is invalid.
 *
 * Error handling: If buf is NULL, function returns silently.
 *
 * Should be called when the recorded video is no longer needed.
 */
void buffer_cleanup(RingBuffer *buf);

/**
 * buffer_get_overflow_count - Get number of times buffer capacity was exceeded
 * @buf: Ring buffer to query
 *
 * Returns the count of overflow events (frames discarded due to capacity).
 * This is useful for diagnostics and monitoring buffer exhaustion.
 *
 * Returns: Overflow count, or 0 if buf is NULL
 */
guint buffer_get_overflow_count(RingBuffer *buf);

/**
 * buffer_get_total_frames_written - Get total frames written to buffer
 * @buf: Ring buffer to query
 *
 * Returns the total number of frames written to the buffer since creation,
 * including frames that were later discarded due to overflow.
 * This differs from frame_count which is capped at capacity.
 *
 * Returns: Total frames written, or 0 if buf is NULL
 */
guint buffer_get_total_frames_written(RingBuffer *buf);

#endif /* BUFFER_MANAGER_H */
