/**
 * @file performance_config.h
 * @brief Performance optimization configuration for 120 fps stability.
 *
 * This module provides optimized configuration settings for GStreamer queue elements,
 * osxvideosink synchronization, and videomixer latency to achieve sustained 120 fps
 * video playback across all 10 grid cells.
 *
 * Key optimization areas:
 * - Queue buffer sizing (max-size-buffers, max-size-bytes, max-size-time)
 * - Queue latency settings (latency, min-threshold)
 * - osxvideosink synchronization (sync, throttle-time)
 * - Videomixer latency tuning
 * - Frame drop detection and reporting
 *
 * The configuration is based on:
 * 1. Target frame rate: 120 fps output
 * 2. Input frame rate: 30 fps from camera
 * 3. Interpolation: 4× from input to output (30fps → 120fps)
 * 4. Memory constraints: <3.4GB GPU memory for 9 simultaneous recordings
 * 5. CPU constraint: <5% single-core CPU usage for video processing
 *
 * Implementation approach:
 * - Conservative buffering: Minimize latency while maintaining smooth playback
 * - Dynamic thresholds: Detect and report frame drops for monitoring
 * - Platform-specific: osxvideosink properties tailored for macOS Metal/OpenGL rendering
 */

#ifndef PERFORMANCE_CONFIG_H
#define PERFORMANCE_CONFIG_H

#include <glib.h>
#include <gst/gst.h>

/**
 * Performance configuration for a queue element.
 *
 * Specifies optimal buffer sizing and latency parameters for different
 * queue contexts (live feed, playback, recording).
 */
typedef struct {
    /**
     * Maximum number of buffers in queue.
     *
     * For live feed: 4-8 buffers (low latency)
     * For playback: 12-30 buffers (smooth interpolation)
     * For recording: 60+ buffers (capture all frames)
     *
     * Unit: buffer count
     */
    guint max_size_buffers;

    /**
     * Maximum total bytes in queue (0 = unlimited).
     *
     * Prevents excessive memory use if buffers are large.
     * For video: typically 0 (unlimited, let frame count limit size)
     *
     * Unit: bytes (0 for no limit)
     */
    guint max_size_bytes;

    /**
     * Maximum total duration in queue (0 = unlimited).
     *
     * Limits temporal buffering to prevent stale frames.
     * For 120fps output from 30fps input:
     *   - Live: 0-100ms (very low latency)
     *   - Playback: 0 (unlimited, let frame count limit)
     *   - Recording: 0 (unlimited)
     *
     * Unit: nanoseconds (0 for no limit)
     */
    guint64 max_size_time;

    /**
     * Queue leaky mode for overflow handling.
     *
     * Options:
     * - 0 (GST_QUEUE_NO_LEAK): Buffer fills and deadlocks (not used here)
     * - 1 (GST_QUEUE_LEAK_UPSTREAM): Drop new frames if buffer full (not used here)
     * - 2 (GST_QUEUE_LEAK_DOWNSTREAM): Drop oldest frames if buffer full (recommended)
     *
     * For live feed: GST_QUEUE_LEAK_DOWNSTREAM (discard old live frames)
     * For playback: GST_QUEUE_NO_LEAK (keep all recorded frames)
     * For recording: GST_QUEUE_NO_LEAK (capture all frames)
     */
    gint leaky_mode;

    /**
     * Silent mode flag.
     *
     * If TRUE, suppress debug logging for this queue (reduces CPU overhead).
     * Useful for high-frequency queues with frequent state changes.
     *
     * TRUE = suppress logs, FALSE = normal logging
     */
    gboolean silent;

} PerformanceQueueConfig;

/**
 * Performance configuration for osxvideosink rendering.
 *
 * Specifies synchronization and rendering parameters for optimal
 * 120 fps playback on macOS using Metal/OpenGL.
 */
typedef struct {
    /**
     * Synchronization to clock.
     *
     * TRUE: Synchronize to GStreamer clock for precise frame rate control
     * FALSE: Render frames as fast as GPU can process
     *
     * Recommended: TRUE (for 120 fps stability)
     */
    gboolean sync;

    /**
     * Throttle time to pace rendering.
     *
     * When sync=TRUE, this limits rendering to one frame per throttle interval.
     * For 120 fps: throttle_time = 1,000,000,000 / 120 = 8,333,333 ns ≈ 8.3ms
     *
     * Set to 0 for unlimited rendering speed (let clock synchronize).
     *
     * Unit: nanoseconds
     */
    guint64 throttle_time;

    /**
     * Force aspect ratio preservation.
     *
     * TRUE: Maintain camera aspect ratio, pillarbox/letterbox as needed
     * FALSE: Stretch to fill window
     *
     * Recommended: TRUE (for correct video display)
     */
    gboolean force_aspect_ratio;

    /**
     * Fullscreen mode.
     *
     * TRUE: Render fullscreen
     * FALSE: Render in window
     *
     * Recommended: FALSE (for application window mode)
     */
    gboolean fullscreen;

} PerformanceOsxvideosinkConfig;

/**
 * Get optimized queue configuration for live feed (cell 1).
 *
 * The live feed queue buffers the continuous camera stream for:
 * - Low-latency display in cell 1
 * - Splitting to record bins when keys are pressed
 *
 * Optimization strategy:
 * - Minimal buffering (4-6 frames = ~130-200ms at 30fps input)
 * - Downstream leaky to prevent backing up other operations
 * - Silent mode to reduce logging overhead
 *
 * @return  PerformanceQueueConfig struct with optimal live feed settings
 */
PerformanceQueueConfig performance_config_live_queue(void);

/**
 * Get optimized queue configuration for playback cells (2-10).
 *
 * Playback queues feed recorded buffers to the videomixer compositor
 * at interpolated frame rates (120fps output from 30fps input).
 *
 * Optimization strategy:
 * - Moderate buffering (12-20 frames = ~400-666ms at 30fps input)
 * - No leaky mode (preserve all recorded frames during playback)
 * - Normal logging for debugging
 *
 * @return  PerformanceQueueConfig struct with optimal playback settings
 */
PerformanceQueueConfig performance_config_playback_queue(void);

/**
 * Get optimized queue configuration for recording bins (keys 1-9).
 *
 * Recording queues capture incoming camera frames for storage in ring buffers.
 * Must handle peak rates during simultaneous multi-key recording.
 *
 * Optimization strategy:
 * - Large buffering (60+ frames = ~2 seconds at 30fps input)
 * - No leaky mode (capture all frames without loss)
 * - Silent mode to minimize CPU during heavy recording
 *
 * @return  PerformanceQueueConfig struct with optimal recording settings
 */
PerformanceQueueConfig performance_config_recording_queue(void);

/**
 * Get optimized osxvideosink configuration for 120 fps rendering.
 *
 * The osxvideosink element renders the composited grid to the Cocoa window.
 * Proper synchronization is critical for achieving sustained 120 fps without
 * frame drops or stuttering.
 *
 * Optimization strategy:
 * - sync=TRUE: Synchronize to GStreamer clock for precise frame rate
 * - throttle_time=0: Let clock synchronization control frame rate
 * - force_aspect_ratio=TRUE: Maintain correct video proportions
 * - fullscreen=FALSE: Window mode for application integration
 *
 * Frame rate calculation:
 * - Target: 120 fps = 8.333 ms per frame
 * - GStreamer clock frequency: 1 GHz (1 ns resolution)
 * - Required clock synchronization for <1 frame jitter: <8.33 ns variance
 *
 * @return  PerformanceOsxvideosinkConfig struct with optimal sink settings
 */
PerformanceOsxvideosinkConfig performance_config_osxvideosink(void);

/**
 * Apply queue configuration to a GStreamer queue element.
 *
 * Sets all properties on the queue element according to the provided
 * configuration struct. Handles property availability gracefully
 * (some properties may not exist on all GStreamer versions).
 *
 * @param queue_element  The GStreamer queue element to configure
 * @param config         The configuration struct with desired properties
 * @param context_name   Human-readable context (e.g., "live_feed") for logging
 * @return               TRUE if configuration successful, FALSE on critical errors
 *
 * Error handling:
 * - Returns FALSE if queue_element is NULL
 * - Returns FALSE if config is NULL
 * - Continues with available properties if some are unsupported
 * - Logs warnings for unsupported properties
 * - Logs debug messages for each property set
 */
gboolean performance_apply_queue_config(GstElement *queue_element,
                                        const PerformanceQueueConfig *config,
                                        const char *context_name);

/**
 * Apply osxvideosink configuration to rendering element.
 *
 * Sets properties on osxvideosink for optimal frame rate control and
 * synchronization. Critical for achieving stable 120 fps playback.
 *
 * @param sink_element  The osxvideosink element to configure
 * @param config        The configuration struct with desired properties
 * @return              TRUE if configuration successful, FALSE on critical errors
 *
 * Error handling:
 * - Returns FALSE if sink_element is NULL
 * - Returns FALSE if config is NULL
 * - Continues with available properties if some are unsupported
 * - Logs warnings for unsupported properties
 */
gboolean performance_apply_osxvideosink_config(GstElement *sink_element,
                                               const PerformanceOsxvideosinkConfig *config);

/**
 * Get default videomixer latency setting for 120 fps composition.
 *
 * The videomixer element combines multiple input streams (live feed +
 * playback loops) into a single composited output. Latency tuning
 * affects how quickly composition can react to changes in input streams.
 *
 * Optimization:
 * - Minimize latency for responsive grid updates
 * - Balance with compositor's ability to wait for all inputs
 * - Typical value: 0-16.67ms (0-2 frames @ 120fps)
 *
 * @return  Recommended latency in nanoseconds (0 = minimize)
 */
guint64 performance_config_videomixer_latency(void);

#endif // PERFORMANCE_CONFIG_H
