/**
 * @file performance_config.c
 * @brief Performance optimization configuration for 120 fps stability.
 *
 * Implements queue sizing, osxvideosink synchronization, and latency tuning
 * for sustained 120 fps playback across all 10 grid cells.
 *
 * All configuration values are based on:
 * - Camera input: 30 fps continuous stream
 * - Target output: 120 fps composited grid
 * - Interpolation: 4× frame rate (30 → 120)
 * - GPU memory constraint: ~3.4GB for 9 simultaneous recordings
 * - CPU constraint: <5% single-core usage for video processing
 */

#include "performance_config.h"
#include "../utils/logging.h"
#include <glib.h>
#include <gst/gst.h>
#include <stdio.h>
#include <string.h>

/**
 * Queue leaky mode constants (from GStreamer queue element).
 *
 * GST_QUEUE_NO_LEAK: Buffer fully, allow backpressure
 * GST_QUEUE_LEAK_UPSTREAM: Drop new frames if full
 * GST_QUEUE_LEAK_DOWNSTREAM: Drop oldest frames if full
 */
#define GST_QUEUE_NO_LEAK 0
#define GST_QUEUE_LEAK_UPSTREAM 1
#define GST_QUEUE_LEAK_DOWNSTREAM 2

PerformanceQueueConfig performance_config_live_queue(void)
{
    /**
     * Live feed queue configuration.
     *
     * Purpose: Buffer the continuous 30fps camera stream for:
     * - Display in cell 1 (live feed)
     * - Splitting to record bins (when keys are pressed)
     *
     * Constraints:
     * - Latency: Minimize (camera → display should be < 100ms)
     * - Buffering: Conservative (4-6 frames ≈ 133-200ms @ 30fps)
     * - Memory: Small (6 frames × 1920×1080 × 4 bytes ≈ 50MB per camera quality)
     *
     * Strategy:
     * - Use frame count limit (max_size_buffers=6)
     * - Downstream leaky to prevent backing up video pipeline
     * - No byte/time limits (frame count is primary constraint)
     * - Silent mode to reduce debug overhead in hot path
     *
     * Rationale:
     * - 6 buffers ≈ 200ms latency at 30fps input
     * - Downstream leaky ensures oldest frames dropped if videomixer gets slow
     * - This balances responsiveness with smooth playback
     */
    PerformanceQueueConfig config = {
        .max_size_buffers = 6,                   // 6 frames ≈ 200ms @ 30fps
        .max_size_bytes = 0,                     // No byte limit (frame count controls)
        .max_size_time = 0,                      // No time limit (frame count controls)
        .leaky_mode = GST_QUEUE_LEAK_DOWNSTREAM, // Drop old frames if buffer full
        .silent = TRUE,                          // Suppress debug logs (reduce CPU)
    };

    LOG_DEBUG("Live queue config: %u buffers, downstream leaky", config.max_size_buffers);

    return config;
}

PerformanceQueueConfig performance_config_playback_queue(void)
{
    /**
     * Playback queue configuration.
     *
     * Purpose: Buffer frames from playback bins during interpolation
     * from 30fps playback to 120fps display.
     *
     * Each playback cell shows recorded video at 120fps (interpolated
     * from 30fps source frames). The queue buffers intermediate
     * interpolated frames between videomixer composition and osxvideosink.
     *
     * Constraints:
     * - Latency: Low-moderate (16.67-50ms for smooth 120fps)
     * - Buffering: Moderate (16-20 frames ≈ 533-666ms @ 30fps input)
     * - Memory: Moderate (20 frames × 320×180 × 4 ≈ 4.6MB per cell)
     *
     * Strategy:
     * - Use frame count limit (max_size_buffers=16-20)
     * - No leaky mode (preserve all playback frames)
     * - Normal logging (helps debug playback issues)
     *
     * Rationale:
     * - 16 buffers ≈ 533ms @ 30fps input, provides smooth interpolation
     * - No leaky prevents loss of recorded content during playback
     * - Balance: enough buffering for smooth playback without excessive latency
     */
    PerformanceQueueConfig config = {
        .max_size_buffers = 16,          // 16 frames ≈ 533ms @ 30fps
        .max_size_bytes = 0,             // No byte limit
        .max_size_time = 0,              // No time limit
        .leaky_mode = GST_QUEUE_NO_LEAK, // Never drop recorded frames
        .silent = FALSE,                 // Normal logging for debugging
    };

    LOG_DEBUG("Playback queue config: %u buffers, no leaky", config.max_size_buffers);

    return config;
}

PerformanceQueueConfig performance_config_recording_queue(void)
{
    /**
     * Recording queue configuration.
     *
     * Purpose: Buffer frames being recorded from camera to ring buffer.
     * Used in record bins (keys 1-9) to hold camera frames before
     * storing in GPU memory.
     *
     * Constraints:
     * - Latency: Not critical (recording is asynchronous)
     * - Buffering: Large (60+ frames ≈ 2 seconds @ 30fps)
     * - Memory: Large (60 frames × 1920×1080 × 4 ≈ 460MB per recording)
     *
     * Strategy:
     * - Use frame count limit (max_size_buffers=60)
     * - No leaky mode (capture all frames during recording)
     * - Silent mode (reduce CPU during heavy multi-key recording)
     *
     * Rationale:
     * - 60 frames ≈ 2 seconds @ 30fps, reasonable recording duration
     * - No leaky ensures every frame captured (no frame loss)
     * - Silent suppresses logging during high-frequency recording
     * - Total GPU memory: 9 recordings × 460MB ≈ 4.1GB (within budget)
     */
    PerformanceQueueConfig config = {
        .max_size_buffers = 60,          // 60 frames ≈ 2 seconds @ 30fps
        .max_size_bytes = 0,             // No byte limit
        .max_size_time = 0,              // No time limit
        .leaky_mode = GST_QUEUE_NO_LEAK, // Never drop frames during recording
        .silent = TRUE,                  // Suppress logs during recording
    };

    LOG_DEBUG("Recording queue config: %u buffers, no leaky", config.max_size_buffers);

    return config;
}

PerformanceOsxvideosinkConfig performance_config_osxvideosink(void)
{
    /**
     * osxvideosink configuration for 120 fps rendering.
     *
     * The osxvideosink element renders the composited video grid to
     * the Cocoa NSWindow using Metal or OpenGL. Proper synchronization
     * is essential for stable 120 fps playback.
     *
     * Frame timing calculation:
     * - Target: 120 fps = 8.333 ms per frame
     * - GStreamer clock: 1 ns resolution (1 GHz frequency)
     * - Required synchronization precision: <8.33 ns (much better than needed)
     * - In practice: ~60-100 ns jitter acceptable (well within 1 frame margin)
     *
     * Configuration strategy:
     * - sync=TRUE: Enable clock synchronization
     *   GStreamer will pace frame rendering to match GStreamer clock,
     *   which is synchronized to system time via display link on macOS.
     *
     * - throttle_time=0: Let clock synchronization control frame rate
     *   The osxvideosink will query the pipeline clock and render frames
     *   according to their timestamps. No additional throttling needed.
     *
     * - force_aspect_ratio=TRUE: Maintain correct video proportions
     *   Camera aspect ratio (usually 16:9) should be preserved.
     *   Pillarbox/letterbox rather than stretch.
     *
     * - fullscreen=FALSE: Window mode (not fullscreen)
     *   Application runs in normal window, not fullscreen display.
     *
     * Implementation notes:
     * - GStreamer uses the system clock by default
     * - macOS display link provides wall-clock synchronization
     * - osxvideosink will wait for correct timestamps before rendering
     * - Frame drops only occur if pipeline cannot keep up with real-time
     *   (e.g., CPU bottleneck, GPU overload, too many simultaneous recordings)
     */
    PerformanceOsxvideosinkConfig config = {
        .sync = TRUE,               // Synchronize to clock for precise frame rate
        .throttle_time = 0,         // Let clock synchronization control rate
        .force_aspect_ratio = TRUE, // Preserve camera aspect ratio
        .fullscreen = FALSE,        // Window mode (not fullscreen)
    };

    LOG_DEBUG("osxvideosink config: sync=true, throttle_time=0ns, "
              "force_aspect_ratio=true, fullscreen=false");

    return config;
}

gboolean performance_apply_queue_config(GstElement *queue_element,
                                        const PerformanceQueueConfig *config,
                                        const char *context_name)
{
    if (!queue_element) {
        LOG_ERROR("performance_apply_queue_config: queue_element is NULL");
        return FALSE;
    }

    if (!config) {
        LOG_ERROR("performance_apply_queue_config: config is NULL");
        return FALSE;
    }

    if (!context_name) {
        context_name = "unknown";
    }

    gboolean success = TRUE;

    // Set max-size-buffers
    LOG_DEBUG("Setting %s queue max-size-buffers=%u", context_name, config->max_size_buffers);
    g_object_set(G_OBJECT(queue_element), "max-size-buffers", config->max_size_buffers, NULL);

    // Set max-size-bytes (0 = unlimited, typically used here)
    if (config->max_size_bytes > 0) {
        LOG_DEBUG("Setting %s queue max-size-bytes=%u", context_name, config->max_size_bytes);
        g_object_set(G_OBJECT(queue_element), "max-size-bytes", config->max_size_bytes, NULL);
    } else {
        LOG_DEBUG("Setting %s queue max-size-bytes=0 (unlimited)", context_name);
        g_object_set(G_OBJECT(queue_element), "max-size-bytes", 0, NULL);
    }

    // Set max-size-time (0 = unlimited, typically used here)
    if (config->max_size_time > 0) {
        LOG_DEBUG("Setting %s queue max-size-time=%llu ns", context_name,
                  (unsigned long long) config->max_size_time);
        g_object_set(G_OBJECT(queue_element), "max-size-time", config->max_size_time, NULL);
    } else {
        LOG_DEBUG("Setting %s queue max-size-time=0 (unlimited)", context_name);
        g_object_set(G_OBJECT(queue_element), "max-size-time", 0ULL, NULL);
    }

    // Set leaky mode (GST_QUEUE_LEAK_*)
    const char *leaky_name = "no-leak";
    if (config->leaky_mode == GST_QUEUE_LEAK_UPSTREAM) {
        leaky_name = "upstream";
    } else if (config->leaky_mode == GST_QUEUE_LEAK_DOWNSTREAM) {
        leaky_name = "downstream";
    }

    LOG_DEBUG("Setting %s queue leaky=%s", context_name, leaky_name);
    g_object_set(G_OBJECT(queue_element), "leaky", config->leaky_mode, NULL);

    // Set silent mode (suppress debug logs)
    LOG_DEBUG("Setting %s queue silent=%s", context_name, config->silent ? "true" : "false");
    g_object_set(G_OBJECT(queue_element), "silent", config->silent, NULL);

    LOG_INFO("Applied performance config to %s queue: "
             "buffers=%u, leaky=%s, silent=%s",
             context_name, config->max_size_buffers, leaky_name, config->silent ? "yes" : "no");

    return success;
}

gboolean performance_apply_osxvideosink_config(GstElement *sink_element,
                                               const PerformanceOsxvideosinkConfig *config)
{
    if (!sink_element) {
        LOG_ERROR("performance_apply_osxvideosink_config: sink_element is NULL");
        return FALSE;
    }

    if (!config) {
        LOG_ERROR("performance_apply_osxvideosink_config: config is NULL");
        return FALSE;
    }

    gboolean success = TRUE;

    // Set sync property for clock synchronization
    LOG_DEBUG("Setting osxvideosink sync=%s", config->sync ? "true" : "false");
    g_object_set(G_OBJECT(sink_element), "sync", config->sync, NULL);

    // Set throttle-time (in nanoseconds)
    // For 120 fps with sync=true, throttle_time should typically be 0
    // to let the clock synchronization control frame rate
    if (config->throttle_time > 0) {
        LOG_DEBUG("Setting osxvideosink throttle-time=%llu ns",
                  (unsigned long long) config->throttle_time);
        g_object_set(G_OBJECT(sink_element), "throttle-time", config->throttle_time, NULL);
    } else {
        LOG_DEBUG("Setting osxvideosink throttle-time=0 (use clock sync)");
        g_object_set(G_OBJECT(sink_element), "throttle-time", 0ULL, NULL);
    }

    // Set force-aspect-ratio for correct video display
    LOG_DEBUG("Setting osxvideosink force-aspect-ratio=%s",
              config->force_aspect_ratio ? "true" : "false");
    g_object_set(G_OBJECT(sink_element), "force-aspect-ratio", config->force_aspect_ratio, NULL);

    // Set fullscreen mode
    LOG_DEBUG("Setting osxvideosink fullscreen=%s", config->fullscreen ? "true" : "false");
    g_object_set(G_OBJECT(sink_element), "fullscreen", config->fullscreen, NULL);

    LOG_INFO("Applied performance config to osxvideosink: "
             "sync=%s, throttle_time=%llu ns, force_aspect_ratio=%s, fullscreen=%s",
             config->sync ? "yes" : "no", (unsigned long long) config->throttle_time,
             config->force_aspect_ratio ? "yes" : "no", config->fullscreen ? "yes" : "no");

    return success;
}

guint64 performance_config_videomixer_latency(void)
{
    /**
     * Videomixer latency for 120 fps composition.
     *
     * The videomixer element combines multiple input streams (live feed
     * + playback loops) into a single composited output. Latency affects
     * how long the videomixer waits for all inputs to become ready before
     * producing an output frame.
     *
     * Calculation for 120 fps target:
     * - Frame period: 1,000,000,000 ns / 120 = 8,333,333 ns ≈ 8.3 ms
     * - Typical videomixer latency: 0-20ms
     * - Optimal for 120fps: 0ms (minimize) or ~1 frame (8.3ms)
     *
     * Strategy: Return 0 (minimize latency)
     * - Videomixer will produce output as soon as possible
     * - Slower inputs may cause frame drops if too far behind
     * - For our use case (single camera at 30fps + playback at ~30fps):
     *   all inputs have same timing, latency is not limiting factor
     *
     * Return value: Latency in nanoseconds (0 = no minimum latency)
     */
    const guint64 VIDEOMIXER_LATENCY_NS = 0; // Minimize latency

    LOG_DEBUG("Videomixer latency: %llu ns (minimize)", (unsigned long long) VIDEOMIXER_LATENCY_NS);

    return VIDEOMIXER_LATENCY_NS;
}
