/**
 * @file error_dialog.h
 * @brief macOS native error dialog handling
 *
 * Provides functions to display native macOS error dialogs (NSAlert)
 * for user-facing error messages. Used when fatal errors occur during
 * application initialization.
 */

#ifndef ERROR_DIALOG_H
#define ERROR_DIALOG_H

#include <glib.h>

/**
 * @enum ErrorDialogType
 * @brief Type of error dialog to display
 */
typedef enum {
    ERROR_DIALOG_CAMERA_NOT_FOUND,
    ERROR_DIALOG_CAMERA_PERMISSION_DENIED,
    ERROR_DIALOG_GSTREAMER_INIT_FAILED,
    ERROR_DIALOG_WINDOW_CREATE_FAILED,
    ERROR_DIALOG_PIPELINE_BUILD_FAILED,
    ERROR_DIALOG_GENERIC_ERROR,
} ErrorDialogType;

/**
 * Display a native macOS error dialog
 *
 * Shows an NSAlert with the given error type and message.
 * The dialog is modal and blocks until the user dismisses it.
 *
 * @param dialog_type Type of error dialog to display
 * @param title Dialog title/heading
 * @param message Dialog message body
 * @return TRUE if dialog was displayed successfully, FALSE on failure
 */
gboolean error_dialog_show(ErrorDialogType dialog_type, const gchar *title, const gchar *message);

/**
 * Display a camera permission denied error dialog
 *
 * Shows a specific error dialog for when the user denies camera access.
 * Explains the issue and suggests the user enable permissions in System Settings.
 *
 * @return TRUE if dialog was displayed successfully
 */
gboolean error_dialog_show_camera_permission_denied(void);

/**
 * Display a camera not found error dialog
 *
 * Shows a specific error dialog for when no built-in camera is detected.
 *
 * @return TRUE if dialog was displayed successfully
 */
gboolean error_dialog_show_camera_not_found(void);

/**
 * Display a GStreamer initialization failed error dialog
 *
 * Shows a specific error dialog when GStreamer library fails to initialize.
 *
 * @param reason Optional reason message (can be NULL)
 * @return TRUE if dialog was displayed successfully
 */
gboolean error_dialog_show_gstreamer_init_failed(const gchar *reason);

/**
 * Display a generic fatal error dialog
 *
 * Shows a generic fatal error dialog with custom title and message.
 *
 * @param title Dialog title
 * @param message Dialog message
 * @return TRUE if dialog was displayed successfully
 */
gboolean error_dialog_show_generic(const gchar *title, const gchar *message);

#endif /* ERROR_DIALOG_H */
