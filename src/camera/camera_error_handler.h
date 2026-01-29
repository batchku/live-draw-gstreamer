/**
 * @file camera_error_handler.h
 * @brief Camera error handling and recovery mechanisms
 *
 * Provides comprehensive error handling for camera errors including:
 * - Camera not found errors
 * - Permission denied errors
 * - Camera disconnection during session
 * - Camera reconnection logic
 * - Error state tracking and recovery
 */

#ifndef CAMERA_ERROR_HANDLER_H
#define CAMERA_ERROR_HANDLER_H

#include "camera_source.h"
#include <gst/gst.h>

/**
 * @enum CameraErrorType
 * @brief Types of camera errors
 */
typedef enum {
    CAMERA_ERROR_NOT_FOUND = 0,             /**< Camera not detected */
    CAMERA_ERROR_PERMISSION_DENIED = 1,     /**< User denied camera access */
    CAMERA_ERROR_DISCONNECTED = 2,          /**< Camera disconnected during session */
    CAMERA_ERROR_FORMAT_FAILED = 3,         /**< Format negotiation failed */
    CAMERA_ERROR_ELEMENT_CREATE_FAILED = 4, /**< GStreamer element creation failed */
    CAMERA_ERROR_UNKNOWN = 5,               /**< Unknown camera error */
} CameraErrorType;

/**
 * @enum CameraState
 * @brief Camera operational state
 */
typedef enum {
    CAMERA_STATE_UNINITIALIZED = 0, /**< Not yet initialized */
    CAMERA_STATE_INITIALIZING = 1,  /**< Initialization in progress */
    CAMERA_STATE_READY = 2,         /**< Ready and operational */
    CAMERA_STATE_ERROR = 3,         /**< Error occurred, needs recovery */
    CAMERA_STATE_DISCONNECTED = 4,  /**< Disconnected but attempting recovery */
    CAMERA_STATE_SHUTDOWN = 5,      /**< Shutting down */
} CameraState;

/**
 * @struct CameraErrorInfo
 * @brief Camera error information
 */
typedef struct {
    CameraErrorType error_type; /**< Type of error that occurred */
    gchar *error_message;       /**< Detailed error message */
    guint64 timestamp_us;       /**< Timestamp when error occurred */
    guint retry_count;          /**< Number of reconnection attempts */
    gboolean is_recoverable;    /**< Whether error is recoverable */
    gpointer user_data;         /**< User context data */
} CameraErrorInfo;

/**
 * Callback type for camera error events
 *
 * @param error_info Camera error information
 * @param user_data User context data
 */
typedef void (*CameraErrorCallback)(CameraErrorInfo *error_info, gpointer user_data);

/**
 * @struct CameraErrorHandler
 * @brief Camera error handler context
 */
typedef struct CameraErrorHandler CameraErrorHandler;

/**
 * Create a new camera error handler
 *
 * @param camera_source Camera source to monitor
 * @return Pointer to newly allocated handler, or NULL on failure
 */
CameraErrorHandler *camera_error_handler_create(CameraSource *camera_source);

/**
 * Register a callback for camera errors
 *
 * @param handler Camera error handler
 * @param callback Function to call on camera errors
 * @param user_data User context to pass to callback
 */
void camera_error_handler_set_callback(CameraErrorHandler *handler, CameraErrorCallback callback,
                                       gpointer user_data);

/**
 * Handle a camera not found error
 *
 * Logs error, notifies application, and triggers graceful shutdown.
 *
 * @param handler Camera error handler
 * @return TRUE if error was handled, FALSE otherwise
 */
gboolean camera_error_handle_not_found(CameraErrorHandler *handler);

/**
 * Handle a camera permission denied error
 *
 * Logs error, notifies application, and triggers graceful shutdown.
 *
 * @param handler Camera error handler
 * @return TRUE if error was handled, FALSE otherwise
 */
gboolean camera_error_handle_permission_denied(CameraErrorHandler *handler);

/**
 * Handle a camera disconnection error
 *
 * Logs error, stops recording/playback, attempts reconnection.
 * This is a recoverable error - reconnection will be attempted.
 *
 * @param handler Camera error handler
 * @return TRUE if error was handled, FALSE otherwise
 */
gboolean camera_error_handle_disconnected(CameraErrorHandler *handler);

/**
 * Handle a format negotiation error
 *
 * Logs error and attempts fallback to alternative format.
 *
 * @param handler Camera error handler
 * @return TRUE if fallback succeeded, FALSE if fatal
 */
gboolean camera_error_handle_format_failed(CameraErrorHandler *handler);

/**
 * Handle a GStreamer element creation error
 *
 * Logs error details and returns recoverable status.
 *
 * @param handler Camera error handler
 * @param element_name Name of element that failed to create
 * @return TRUE if recoverable, FALSE if fatal
 */
gboolean camera_error_handle_element_create_failed(CameraErrorHandler *handler,
                                                   const gchar *element_name);

/**
 * Attempt to reconnect to the camera
 *
 * Called when camera disconnection is detected. Attempts to re-initialize
 * the camera after a brief delay.
 *
 * @param handler Camera error handler
 * @return TRUE if reconnection started, FALSE otherwise
 */
gboolean camera_error_attempt_reconnection(CameraErrorHandler *handler);

/**
 * Get the current camera error state
 *
 * @param handler Camera error handler
 * @return Current camera state
 */
CameraState camera_error_get_state(CameraErrorHandler *handler);

/**
 * Set the camera error state
 *
 * @param handler Camera error handler
 * @param state New camera state
 */
void camera_error_set_state(CameraErrorHandler *handler, CameraState state);

/**
 * Get the last camera error information
 *
 * @param handler Camera error handler
 * @return Last error info, or NULL if no error
 */
CameraErrorInfo *camera_error_get_last_error(CameraErrorHandler *handler);

/**
 * Check if camera is in error state
 *
 * @param handler Camera error handler
 * @return TRUE if camera is in error or disconnected state
 */
gboolean camera_error_is_in_error_state(CameraErrorHandler *handler);

/**
 * Check if camera is accessible for operations
 *
 * @param handler Camera error handler
 * @return TRUE if camera is ready and accessible
 */
gboolean camera_error_is_accessible(CameraErrorHandler *handler);

/**
 * Reset camera error state (after successful recovery)
 *
 * @param handler Camera error handler
 */
void camera_error_reset_state(CameraErrorHandler *handler);

/**
 * Cleanup camera error handler
 *
 * @param handler Camera error handler to cleanup
 */
void camera_error_handler_cleanup(CameraErrorHandler *handler);

#endif /* CAMERA_ERROR_HANDLER_H */
