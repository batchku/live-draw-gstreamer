#ifndef CAMERA_SOURCE_H
#define CAMERA_SOURCE_H

/**
 * @file camera_source.h
 * @brief Camera input component interface
 *
 * Manages camera initialization, format negotiation, and
 * provides the GStreamer camera source element.
 */

#include <gst/gst.h>

/**
 * @struct CameraSource
 * @brief Camera source configuration
 */
typedef struct {
    GstElement *source_element;
    char device_id[256];
    int width;
    int height;
    int framerate;
    char caps_string[512];
} CameraSource;

/**
 * @enum CameraPermissionStatus
 * @brief Camera permission states
 */
typedef enum {
    CAMERA_PERMISSION_GRANTED = 0,
    CAMERA_PERMISSION_DENIED = 1,
    CAMERA_PERMISSION_NOT_DETERMINED = 2,
} CameraPermissionStatus;

/**
 * @struct CameraCapabilities
 * @brief Camera hardware capabilities
 */
typedef struct {
    int *supported_widths;     /**< NULL-terminated array of supported widths */
    int *supported_heights;    /**< NULL-terminated array of supported heights */
    int *supported_framerates; /**< NULL-terminated array of supported frame rates */
    int count;                 /**< Number of format combinations */
} CameraCapabilities;

/**
 * Initialize and open the camera source with format negotiation
 *
 * Performs the following steps in order:
 * 1. Requests camera permission from macOS
 * 2. Attempts format negotiation with preferred resolution (1920x1080 @ 30fps)
 * 3. Falls back to alternative resolution (1280x720 @ 30fps) if preferred fails
 * 4. Returns error if all format candidates fail
 *
 * @return Newly allocated CameraSource with negotiated format, or NULL on failure
 */
CameraSource *camera_source_init(void);

/**
 * Request camera permission from the operating system
 *
 * @return Permission status
 */
CameraPermissionStatus camera_request_permission(void);

/**
 * Create GStreamer source element for the camera
 *
 * @param cam Camera source configuration
 * @return GStreamer element, or NULL on failure
 */
GstElement *camera_source_create_element(CameraSource *cam);

/**
 * Query camera capabilities and supported formats
 *
 * @param cam Camera source to query
 * @return Pointer to CameraCapabilities structure, or NULL on failure
 */
CameraCapabilities *camera_get_capabilities(CameraSource *cam);

/**
 * Free camera capabilities structure
 *
 * @param caps Camera capabilities to free
 */
void camera_capabilities_free(CameraCapabilities *caps);

/**
 * Cleanup camera source
 *
 * @param cam Camera source to cleanup
 */
void camera_source_cleanup(CameraSource *cam);

/**
 * Check if camera is connected and responding
 *
 * Detects if camera has been disconnected during a session.
 * Can be called periodically to detect hardware removal.
 *
 * @param cam Camera source to check
 * @return TRUE if camera is connected, FALSE if disconnected
 */
gboolean camera_source_is_connected(CameraSource *cam);

/**
 * Attempt to reinitialize camera after disconnection
 *
 * Called when camera disconnection is detected. Attempts to
 * reconnect and renegotiate format.
 *
 * @param cam Camera source to reinitialize
 * @return TRUE if reinitialization succeeded, FALSE otherwise
 */
gboolean camera_source_reinitialize(CameraSource *cam);

#endif /* CAMERA_SOURCE_H */
