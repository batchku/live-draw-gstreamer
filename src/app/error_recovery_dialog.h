/**
 * @file error_recovery_dialog.h
 * @brief User-friendly error dialogs with recovery options
 *
 * Provides functions to display error dialogs to users when fatal errors occur.
 * Dialogs are displayed for:
 * - Camera errors (not found, permission denied, disconnected)
 * - GStreamer errors (initialization, pipeline, state changes)
 * - Window/rendering errors (creation failed, visibility lost)
 * - Keyboard handler errors (recovery failed)
 *
 * Dialogs are native macOS NSAlert windows with appropriate messaging and
 * suggestions for recovery or further action.
 */

#ifndef ERROR_RECOVERY_DIALOG_H
#define ERROR_RECOVERY_DIALOG_H

#include <glib.h>

/**
 * @enum ErrorDialogResult
 * @brief User's response to error dialog
 */
typedef enum {
    ERROR_DIALOG_RESULT_OK,     /**< User clicked OK/Close */
    ERROR_DIALOG_RESULT_RETRY,  /**< User clicked Retry */
    ERROR_DIALOG_RESULT_CANCEL, /**< User clicked Cancel/Exit */
} ErrorDialogResult;

/**
 * Display a user-friendly error dialog for keyboard handler recovery failure
 *
 * Shows a dialog explaining that the keyboard input handler has failed
 * and recovered multiple times. Offers options to continue or exit.
 *
 * @param recovery_attempts Number of recovery attempts that failed
 * @return ERROR_DIALOG_RESULT_OK to continue, ERROR_DIALOG_RESULT_CANCEL to exit
 */
ErrorDialogResult error_recovery_dialog_keyboard_failure(int recovery_attempts);

/**
 * Display a user-friendly error dialog for window recovery failure
 *
 * Shows a dialog explaining that the window could not be created or
 * recovery failed. Offers options to retry or exit.
 *
 * @param error_message Optional detailed error message
 * @return ERROR_DIALOG_RESULT_RETRY to retry, ERROR_DIALOG_RESULT_CANCEL to exit
 */
ErrorDialogResult error_recovery_dialog_window_failure(const gchar *error_message);

/**
 * Display a user-friendly error dialog for rendering failure
 *
 * Shows a dialog explaining that video rendering has failed.
 * Offers options to continue or exit.
 *
 * @return ERROR_DIALOG_RESULT_OK to continue, ERROR_DIALOG_RESULT_CANCEL to exit
 */
ErrorDialogResult error_recovery_dialog_rendering_failure(void);

/**
 * Display a user-friendly error dialog for window visibility loss
 *
 * Shows a dialog explaining that the window is no longer visible.
 * Offers to bring window back to front.
 *
 * @return ERROR_DIALOG_RESULT_OK to restore, ERROR_DIALOG_RESULT_CANCEL to exit
 */
ErrorDialogResult error_recovery_dialog_window_visibility_loss(void);

/**
 * Display a generic fatal error dialog with custom message
 *
 * Shows a generic fatal error dialog with custom title and message.
 * Used for unexpected errors not covered by specific dialog functions.
 *
 * @param title Dialog title/heading
 * @param message Dialog message body
 * @return ERROR_DIALOG_RESULT_OK if user confirms
 */
ErrorDialogResult error_recovery_dialog_generic_fatal(const gchar *title, const gchar *message);

/**
 * Display an informational dialog about recovery attempt
 *
 * Shows an informational dialog (not an error) explaining that the system
 * is attempting to recover from a failure. Used for non-blocking recovery messages.
 *
 * @param recovery_type Description of what's being recovered (e.g., "keyboard handler")
 * @param attempt Current recovery attempt number (e.g., 1, 2, 3)
 */
void error_recovery_dialog_recovery_attempt(const gchar *recovery_type, int attempt);

#endif /* ERROR_RECOVERY_DIALOG_H */
