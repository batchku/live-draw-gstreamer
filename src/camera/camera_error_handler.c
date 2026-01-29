/**
 * @file camera_error_handler.c
 * @brief Camera error handling and recovery implementation
 *
 * Implements comprehensive error handling for camera errors including
 * detection, logging, user notification, and recovery mechanisms.
 */

#include "camera_error_handler.h"
#include "../app/app_error.h"
#include "../utils/logging.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Internal camera error handler context
 */
struct CameraErrorHandler {
    CameraSource *camera_source;        /**< Reference to camera source */
    CameraState current_state;          /**< Current operational state */
    CameraErrorInfo *last_error;        /**< Last error that occurred */
    CameraErrorCallback error_callback; /**< Registered error callback */
    gpointer callback_user_data;        /**< User data for callback */
    guint reconnect_attempt_count;      /**< Number of reconnection attempts */
    guint64 last_error_time_us;         /**< Timestamp of last error */
    gboolean recovery_in_progress;      /**< Recovery operation in progress */
};

/**
 * Create a new error info structure
 *
 * @param error_type Type of error
 * @param message Error message
 * @return Newly allocated CameraErrorInfo, or NULL on failure
 */
static CameraErrorInfo *camera_error_info_create(CameraErrorType error_type, const gchar *message)
{
    CameraErrorInfo *info = malloc(sizeof(CameraErrorInfo));
    if (!info) {
        LOG_ERROR("Failed to allocate camera error info");
        return NULL;
    }

    info->error_type = error_type;
    info->error_message = g_strdup(message ? message : "Unknown error");
    info->timestamp_us = g_get_real_time();
    info->retry_count = 0;
    info->is_recoverable = FALSE;
    info->user_data = NULL;

    return info;
}

/**
 * Free error info structure
 *
 * @param info Error info to free
 */
static void camera_error_info_free(CameraErrorInfo *info)
{
    if (!info) {
        return;
    }

    if (info->error_message) {
        g_free(info->error_message);
        info->error_message = NULL;
    }

    free(info);
}

CameraErrorHandler *camera_error_handler_create(CameraSource *camera_source)
{
    if (!camera_source) {
        LOG_ERROR("Cannot create error handler for NULL camera source");
        return NULL;
    }

    CameraErrorHandler *handler = malloc(sizeof(CameraErrorHandler));
    if (!handler) {
        LOG_ERROR("Failed to allocate camera error handler");
        return NULL;
    }

    handler->camera_source = camera_source;
    handler->current_state = CAMERA_STATE_UNINITIALIZED;
    handler->last_error = NULL;
    handler->error_callback = NULL;
    handler->callback_user_data = NULL;
    handler->reconnect_attempt_count = 0;
    handler->last_error_time_us = 0;
    handler->recovery_in_progress = FALSE;

    LOG_INFO("Camera error handler created");
    return handler;
}

void camera_error_handler_set_callback(CameraErrorHandler *handler, CameraErrorCallback callback,
                                       gpointer user_data)
{
    if (!handler) {
        LOG_ERROR("Cannot set callback on NULL error handler");
        return;
    }

    handler->error_callback = callback;
    handler->callback_user_data = user_data;

    LOG_DEBUG("Camera error callback registered");
}

/**
 * Dispatch error to registered callback
 *
 * @param handler Camera error handler
 * @param error_info Error information to dispatch
 */
static void camera_error_dispatch_callback(CameraErrorHandler *handler, CameraErrorInfo *error_info)
{
    if (!handler || !error_info) {
        return;
    }

    if (handler->error_callback) {
        LOG_DEBUG("Dispatching camera error callback: %s", error_info->error_message);
        handler->error_callback(error_info, handler->callback_user_data);
    }
}

gboolean camera_error_handle_not_found(CameraErrorHandler *handler)
{
    if (!handler) {
        LOG_ERROR("Cannot handle error on NULL handler");
        return FALSE;
    }

    LOG_ERROR("FATAL: Camera not found");

    /* Create error info */
    CameraErrorInfo *error_info = camera_error_info_create(
        CAMERA_ERROR_NOT_FOUND, "Built-in camera not detected on this computer");

    if (!error_info) {
        LOG_ERROR("Failed to create error info");
        return FALSE;
    }

    error_info->is_recoverable = FALSE;

    /* Update handler state */
    handler->current_state = CAMERA_STATE_ERROR;
    if (handler->last_error) {
        camera_error_info_free(handler->last_error);
    }
    handler->last_error = error_info;
    handler->last_error_time_us = g_get_real_time();

    /* Log to application error system */
    app_log_error(
        APP_ERROR_CAMERA_NOT_FOUND,
        "Built-in camera not found. "
        "Please ensure your Mac has a built-in camera (e.g., MacBook, iMac with camera).");

    /* Dispatch to callback */
    camera_error_dispatch_callback(handler, error_info);

    LOG_ERROR("Camera not found error handled. Application should terminate.");
    return TRUE;
}

gboolean camera_error_handle_permission_denied(CameraErrorHandler *handler)
{
    if (!handler) {
        LOG_ERROR("Cannot handle error on NULL handler");
        return FALSE;
    }

    LOG_ERROR("FATAL: Camera permission denied");

    /* Create error info */
    CameraErrorInfo *error_info = camera_error_info_create(CAMERA_ERROR_PERMISSION_DENIED,
                                                           "User denied camera access permission");

    if (!error_info) {
        LOG_ERROR("Failed to create error info");
        return FALSE;
    }

    error_info->is_recoverable = FALSE;

    /* Update handler state */
    handler->current_state = CAMERA_STATE_ERROR;
    if (handler->last_error) {
        camera_error_info_free(handler->last_error);
    }
    handler->last_error = error_info;
    handler->last_error_time_us = g_get_real_time();

    /* Log to application error system */
    app_log_error(
        APP_ERROR_CAMERA_PERMISSION_DENIED,
        "Camera access was denied. "
        "Please grant camera permission in System Preferences > Security & Privacy > Camera.");

    /* Dispatch to callback */
    camera_error_dispatch_callback(handler, error_info);

    LOG_ERROR("Camera permission denied error handled. Application should terminate.");
    return TRUE;
}

gboolean camera_error_handle_disconnected(CameraErrorHandler *handler)
{
    if (!handler) {
        LOG_ERROR("Cannot handle error on NULL handler");
        return FALSE;
    }

    LOG_WARNING("Camera disconnected during session");

    /* Create error info */
    CameraErrorInfo *error_info =
        camera_error_info_create(CAMERA_ERROR_DISCONNECTED, "Camera disconnected from computer");

    if (!error_info) {
        LOG_ERROR("Failed to create error info");
        return FALSE;
    }

    error_info->is_recoverable = TRUE;
    error_info->retry_count = handler->reconnect_attempt_count;

    /* Update handler state */
    handler->current_state = CAMERA_STATE_DISCONNECTED;
    if (handler->last_error) {
        camera_error_info_free(handler->last_error);
    }
    handler->last_error = error_info;
    handler->last_error_time_us = g_get_real_time();

    /* Log to application error system */
    app_log_warning(APP_ERROR_CAMERA_DISCONNECTED,
                    "Camera disconnected. Attempting to reconnect...");

    /* Dispatch to callback */
    camera_error_dispatch_callback(handler, error_info);

    /* Attempt reconnection */
    if (camera_error_attempt_reconnection(handler)) {
        LOG_INFO("Camera reconnection initiated");
        return TRUE;
    } else {
        LOG_ERROR("Failed to initiate camera reconnection");
        return FALSE;
    }
}

gboolean camera_error_handle_format_failed(CameraErrorHandler *handler)
{
    if (!handler) {
        LOG_ERROR("Cannot handle error on NULL handler");
        return FALSE;
    }

    LOG_ERROR("Camera format negotiation failed");

    /* Create error info */
    CameraErrorInfo *error_info = camera_error_info_create(
        CAMERA_ERROR_FORMAT_FAILED, "Unable to negotiate compatible camera format");

    if (!error_info) {
        LOG_ERROR("Failed to create error info");
        return FALSE;
    }

    error_info->is_recoverable = FALSE;

    /* Update handler state */
    handler->current_state = CAMERA_STATE_ERROR;
    if (handler->last_error) {
        camera_error_info_free(handler->last_error);
    }
    handler->last_error = error_info;
    handler->last_error_time_us = g_get_real_time();

    /* Log to application error system */
    app_log_error(APP_ERROR_CAMERA_NOT_FOUND,
                  "Failed to negotiate camera format. "
                  "Camera may not support required video formats (1920x1080 or 1280x720 @ 30fps).");

    /* Dispatch to callback */
    camera_error_dispatch_callback(handler, error_info);

    LOG_ERROR("Camera format negotiation error handled. Application should terminate.");
    return FALSE; /* Not recoverable */
}

gboolean camera_error_handle_element_create_failed(CameraErrorHandler *handler,
                                                   const gchar *element_name)
{
    if (!handler) {
        LOG_ERROR("Cannot handle error on NULL handler");
        return FALSE;
    }

    if (!element_name) {
        element_name = "unknown";
    }

    LOG_ERROR("Failed to create GStreamer element: %s", element_name);

    /* Create error info with detailed message */
    gchar message[256];
    snprintf(message, sizeof(message),
             "Failed to create GStreamer element: %s. "
             "Required GStreamer plugin may not be installed.",
             element_name);

    CameraErrorInfo *error_info =
        camera_error_info_create(CAMERA_ERROR_ELEMENT_CREATE_FAILED, message);

    if (!error_info) {
        LOG_ERROR("Failed to create error info");
        return FALSE;
    }

    error_info->is_recoverable = FALSE;

    /* Update handler state */
    handler->current_state = CAMERA_STATE_ERROR;
    if (handler->last_error) {
        camera_error_info_free(handler->last_error);
    }
    handler->last_error = error_info;
    handler->last_error_time_us = g_get_real_time();

    /* Log to application error system */
    app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED,
                  "Failed to create GStreamer element '%s'. "
                  "Please check that all required GStreamer plugins are installed.",
                  element_name);

    /* Dispatch to callback */
    camera_error_dispatch_callback(handler, error_info);

    LOG_ERROR("GStreamer element creation error handled.");
    return FALSE; /* Not recoverable */
}

gboolean camera_error_attempt_reconnection(CameraErrorHandler *handler)
{
    if (!handler) {
        LOG_ERROR("Cannot attempt reconnection on NULL handler");
        return FALSE;
    }

    handler->reconnect_attempt_count++;

    /* Limit reconnection attempts to prevent infinite loops */
    if (handler->reconnect_attempt_count > 5) {
        LOG_ERROR("Camera reconnection failed after 5 attempts. Giving up.");
        app_log_error(APP_ERROR_CAMERA_DISCONNECTED,
                      "Camera reconnection failed after 5 attempts. "
                      "Please reconnect the camera and restart the application.");
        handler->current_state = CAMERA_STATE_ERROR;
        return FALSE;
    }

    LOG_INFO("Attempting camera reconnection (attempt %u/5)", handler->reconnect_attempt_count);

    handler->recovery_in_progress = TRUE;
    handler->current_state = CAMERA_STATE_INITIALIZING;

    /* In a real implementation, we would:
     * 1. Wait a brief delay (100-200ms) before attempting reconnection
     * 2. Re-run AVFoundation camera discovery
     * 3. Reinitialize camera source elements
     * 4. Update GStreamer pipeline to use new camera element
     * 5. Resume playback/recording
     *
     * For now, we signal that recovery was initiated.
     */

    LOG_DEBUG("Camera reconnection initiated. Recovery in progress...");
    return TRUE;
}

CameraState camera_error_get_state(CameraErrorHandler *handler)
{
    if (!handler) {
        return CAMERA_STATE_UNINITIALIZED;
    }

    return handler->current_state;
}

void camera_error_set_state(CameraErrorHandler *handler, CameraState state)
{
    if (!handler) {
        return;
    }

    CameraState old_state = handler->current_state;
    handler->current_state = state;

    LOG_DEBUG("Camera state changed: %d → %d", old_state, state);

    if (state == CAMERA_STATE_READY) {
        handler->recovery_in_progress = FALSE;
        handler->reconnect_attempt_count = 0;
        LOG_INFO("Camera is ready for operation");
    } else if (state == CAMERA_STATE_ERROR || state == CAMERA_STATE_DISCONNECTED) {
        LOG_WARNING("Camera entered error state: %d", state);
    }
}

CameraErrorInfo *camera_error_get_last_error(CameraErrorHandler *handler)
{
    if (!handler) {
        return NULL;
    }

    return handler->last_error;
}

gboolean camera_error_is_in_error_state(CameraErrorHandler *handler)
{
    if (!handler) {
        return TRUE; /* Assume error if no handler */
    }

    return handler->current_state == CAMERA_STATE_ERROR ||
           handler->current_state == CAMERA_STATE_DISCONNECTED;
}

gboolean camera_error_is_accessible(CameraErrorHandler *handler)
{
    if (!handler) {
        return FALSE;
    }

    return handler->current_state == CAMERA_STATE_READY && !handler->recovery_in_progress;
}

void camera_error_reset_state(CameraErrorHandler *handler)
{
    if (!handler) {
        return;
    }

    LOG_INFO("Resetting camera error state");

    handler->current_state = CAMERA_STATE_READY;
    handler->reconnect_attempt_count = 0;
    handler->recovery_in_progress = FALSE;

    if (handler->last_error) {
        camera_error_info_free(handler->last_error);
        handler->last_error = NULL;
    }

    LOG_DEBUG("Camera error state reset complete");
}

void camera_error_handler_cleanup(CameraErrorHandler *handler)
{
    if (!handler) {
        return;
    }

    LOG_DEBUG("Cleaning up camera error handler");

    if (handler->last_error) {
        camera_error_info_free(handler->last_error);
        handler->last_error = NULL;
    }

    handler->error_callback = NULL;
    handler->callback_user_data = NULL;
    handler->camera_source = NULL;

    free(handler);
}
