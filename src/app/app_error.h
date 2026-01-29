#ifndef APP_ERROR_H
#define APP_ERROR_H

/**
 * @file app_error.h
 * @brief Application error codes and error handling
 *
 * Defines error codes for application errors and provides error
 * handling infrastructure using GLib error model.
 */

#include <glib.h>

/**
 * @enum AppErrorCode
 * @brief Application error codes
 */
typedef enum {
    /* Initialization errors */
    APP_ERROR_GSTREAMER_INIT_FAILED = 100,
    APP_ERROR_WINDOW_CREATE_FAILED = 101,
    APP_ERROR_CAMERA_NOT_FOUND = 102,
    APP_ERROR_CAMERA_PERMISSION_DENIED = 103,
    APP_ERROR_PIPELINE_BUILD_FAILED = 104,

    /* Runtime errors */
    APP_ERROR_CAMERA_DISCONNECTED = 200,
    APP_ERROR_PIPELINE_STATE_CHANGE_FAILED = 201,
    APP_ERROR_MEMORY_ALLOCATION_FAILED = 202,
    APP_ERROR_RECORDING_BUFFER_FULL = 203,
    APP_ERROR_KEYBOARD_HANDLER_FAILED = 204,

    /* Warnings (non-fatal) */
    APP_WARNING_FRAME_DROP_DETECTED = 300,
    APP_WARNING_MEMORY_USAGE_HIGH = 301,
} AppErrorCode;

/**
 * @struct AppError
 * @brief Application error information
 */
typedef struct {
    AppErrorCode code;
    gchar *message;
    const gchar *function;
    const gchar *file;
    guint line;
    gpointer user_data;
} AppError;

/**
 * Callback type for error handling
 */
typedef void (*AppErrorCallback)(AppError *error, gpointer user_data);

/**
 * Register application error handler callback
 *
 * @param handler Callback function to handle errors
 * @param user_data Optional user data to pass to callback
 */
void app_register_error_handler(AppErrorCallback handler, gpointer user_data);

/**
 * Log an error and invoke registered error handler
 *
 * @param code Error code
 * @param format Format string for error message
 * @param ... Arguments for format string
 */
void app_log_error(AppErrorCode code, const gchar *format, ...);

/**
 * Log a warning and invoke registered error handler
 *
 * @param code Warning code
 * @param format Format string for warning message
 * @param ... Arguments for format string
 */
void app_log_warning(AppErrorCode code, const gchar *format, ...);

/**
 * Retrieve the last error
 *
 * @return Last AppError, or NULL if no error
 */
AppError *app_get_last_error(void);

#endif /* APP_ERROR_H */
