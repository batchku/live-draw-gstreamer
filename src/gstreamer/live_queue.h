/**
 * @file live_queue.h
 * @brief Live queue element creation and caps negotiation for cell 1 live feed.
 *
 * This module provides specialized configuration for the live_queue element
 * used in cell 1 of the video grid. The live queue buffers the live camera feed
 * in GPU memory before sending it to the videomixer compositor.
 *
 * Key responsibilities:
 * - Create queue element with proper GPU memory configuration
 * - Negotiate caps (video format, resolution, framerate) with camera source
 * - Configure downstream capsfilter for format compatibility
 * - Handle queue overflow scenarios (leaky behavior)
 *
 * The live queue is critical for:
 * - Decoupling camera input (30fps) from output rendering (120fps interpolation)
 * - Buffering frames in GPU memory without CPU transfer
 * - Maintaining continuous live feed in cell 1 during recording/playback
 */

#ifndef LIVE_QUEUE_H
#define LIVE_QUEUE_H

#include <gst/gst.h>

/**
 * Configuration for live queue caps negotiation.
 *
 * Used to specify caps constraints for the live feed pipeline.
 */
typedef struct {
    gint width;         /**< Video width (e.g., 1920) */
    gint height;        /**< Video height (e.g., 1080) */
    gint framerate_num; /**< Framerate numerator (e.g., 30) */
    gint framerate_den; /**< Framerate denominator (e.g., 1 for 30fps) */
    const char *format; /**< Pixel format (e.g., "BGRx", "YUY2") */
} LiveQueueCaps;

/**
 * Create a live queue element with GPU memory configuration.
 *
 * Initializes a queue element specifically for buffering the live camera feed
 * in GPU memory. Configured with:
 * - max-size-buffers: 30 frames (approximately 1 second at 30fps)
 * - max-size-bytes: unlimited (0)
 * - max-size-time: unlimited (0)
 * - leaky: downstream (drop oldest frame if buffer full, never drop input)
 *
 * The downstream leaky behavior ensures continuous recording even if
 * videomixer gets backed up; we discard old live frames but maintain
 * buffer flow.
 *
 * @param name  Unique name for the queue element (e.g., "live-queue")
 * @return      Pointer to GstElement queue on success, NULL on failure
 *
 * Error handling:
 * - Returns NULL if element factory "queue" not found
 * - Returns NULL if element creation fails
 * Errors are logged via logging module.
 *
 * The returned element has ref count 1 and must be unreferenced
 * by the caller using gst_object_unref() when no longer needed.
 */
GstElement *live_queue_create(const char *name);

/**
 * Configure caps negotiation for live feed.
 *
 * Sets up the downstream capsfilter element to negotiate video format
 * between camera source and videomixer. Specifies constraints that
 * GStreamer uses to negotiate compatible formats.
 *
 * The live feed should maintain camera native format (typically BGRx at 30fps)
 * which will be interpolated to 120fps by the videomixer and osxvideosink
 * for smooth playback.
 *
 * @param live_caps_elem  The capsfilter element created by gst_elements_create_capsfilter()
 * @param caps_config     Caps configuration (width, height, format, framerate)
 * @return                TRUE if caps configured successfully, FALSE on failure
 *
 * Error handling:
 * - Returns FALSE if live_caps_elem is NULL
 * - Returns FALSE if caps_config is NULL
 * - Returns FALSE if GStreamer caps string parsing fails
 * Errors are logged via logging module.
 *
 * Example:
 * @code
 *   GstElement *live_caps = gst_elements_create_capsfilter("live-caps");
 *   LiveQueueCaps caps_config = {
 *       .width = 1920,
 *       .height = 1080,
 *       .framerate_num = 30,
 *       .framerate_den = 1,
 *       .format = "BGRx"
 *   };
 *   if (live_queue_configure_caps(live_caps, &caps_config)) {
 *       // Successfully configured
 *   }
 * @endcode
 */
gboolean live_queue_configure_caps(GstElement *live_caps_elem, const LiveQueueCaps *caps_config);

/**
 * Get recommended caps configuration from camera source element.
 *
 * Negotiates with upstream camera source to determine optimal caps
 * for the live feed. Uses the camera source's allow-caps list to
 * select a compatible format.
 *
 * Prefers:
 * 1. BGRx at 1920×1080 @ 30fps (primary)
 * 2. BGRx at 1280×720 @ 30fps (fallback)
 * 3. YUY2 at 1920×1080 @ 30fps (secondary)
 * 4. First available format from camera (last resort)
 *
 * @param camera_source   Camera source element (typically avfvideosrc)
 * @param out_caps_config Output: Recommended caps configuration
 * @return                TRUE if negotiation successful, FALSE on failure
 *
 * Error handling:
 * - Returns FALSE if camera_source is NULL
 * - Returns FALSE if out_caps_config is NULL
 * - Returns FALSE if no compatible format found
 * Errors are logged via logging module.
 *
 * Note: The out_caps_config->format field points to a static string
 * and should not be freed by the caller.
 */
gboolean live_queue_negotiate_caps(GstElement *camera_source, LiveQueueCaps *out_caps_config);

#endif // LIVE_QUEUE_H
