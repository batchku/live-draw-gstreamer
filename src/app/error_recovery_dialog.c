/**
 * @file error_recovery_dialog.c
 * @brief User-friendly error dialogs with recovery options implementation
 *
 * Implements native macOS error dialogs (NSAlert) for displaying errors
 * and recovery options to users.
 */

#include "error_recovery_dialog.h"
#include "../utils/logging.h"
#include "error_dialog.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

/**
 * Display a user-friendly error dialog for keyboard handler recovery failure
 */
ErrorDialogResult error_recovery_dialog_keyboard_failure(int recovery_attempts)
{
    LOG_ERROR("Displaying keyboard handler recovery failure dialog (attempts=%d)",
              recovery_attempts);

    gchar message[512];
    g_snprintf(message, sizeof(message),
               "The keyboard input handler encountered repeated failures and could not "
               "recover after %d attempt%s. "
               "The application may not respond to keyboard input. "
               "You can continue using the application, but keyboard controls will be unavailable. "
               "Please try restarting the application if this persists.",
               recovery_attempts, recovery_attempts == 1 ? "" : "s");

    gboolean shown =
        error_dialog_show(ERROR_DIALOG_GENERIC_ERROR, "Keyboard Handler Error", message);

    if (shown) {
        LOG_INFO("User acknowledged keyboard handler error");
        return ERROR_DIALOG_RESULT_OK;
    } else {
        LOG_WARNING("Failed to display keyboard handler error dialog");
        return ERROR_DIALOG_RESULT_OK;
    }
}

/**
 * Display a user-friendly error dialog for window recovery failure
 */
ErrorDialogResult error_recovery_dialog_window_failure(const gchar *error_message)
{
    LOG_ERROR("Displaying window recovery failure dialog");

    gchar message[512];
    if (error_message && strlen(error_message) > 0) {
        g_snprintf(message, sizeof(message),
                   "The application window could not be created or recovered. "
                   "Error: %s "
                   "Please try restarting the application.",
                   error_message);
    } else {
        g_snprintf(message, sizeof(message),
                   "The application window could not be created or recovered. "
                   "This is likely a system issue with the display or rendering context. "
                   "Please try restarting the application.");
    }

    gboolean shown =
        error_dialog_show(ERROR_DIALOG_WINDOW_CREATE_FAILED, "Window Creation Failed", message);

    if (shown) {
        LOG_INFO("User acknowledged window failure");
        return ERROR_DIALOG_RESULT_CANCEL;
    } else {
        LOG_WARNING("Failed to display window failure dialog");
        return ERROR_DIALOG_RESULT_CANCEL;
    }
}

/**
 * Display a user-friendly error dialog for rendering failure
 */
ErrorDialogResult error_recovery_dialog_rendering_failure(void)
{
    LOG_ERROR("Displaying rendering failure dialog");

    const gchar *message =
        "Video rendering has failed. This could be due to a GPU issue or "
        "incompatible display settings. The application will attempt to continue, "
        "but video may not display correctly. "
        "Please try restarting the application or checking your display settings.";

    gboolean shown = error_dialog_show(ERROR_DIALOG_GENERIC_ERROR, "Rendering Error", message);

    if (shown) {
        LOG_INFO("User acknowledged rendering failure");
        return ERROR_DIALOG_RESULT_OK;
    } else {
        LOG_WARNING("Failed to display rendering failure dialog");
        return ERROR_DIALOG_RESULT_OK;
    }
}

/**
 * Display a user-friendly error dialog for window visibility loss
 */
ErrorDialogResult error_recovery_dialog_window_visibility_loss(void)
{
    LOG_WARNING("Displaying window visibility loss dialog");

    const gchar *message = "The application window is no longer visible. "
                           "It may have been minimized or hidden. "
                           "The application will attempt to restore the window to the foreground.";

    gboolean shown = error_dialog_show(ERROR_DIALOG_GENERIC_ERROR, "Window Not Visible", message);

    if (shown) {
        LOG_INFO("User acknowledged window visibility loss");
        return ERROR_DIALOG_RESULT_OK;
    } else {
        LOG_WARNING("Failed to display window visibility loss dialog");
        return ERROR_DIALOG_RESULT_OK;
    }
}

/**
 * Display a generic fatal error dialog with custom message
 */
ErrorDialogResult error_recovery_dialog_generic_fatal(const gchar *title, const gchar *message)
{
    if (!title || !message) {
        LOG_ERROR("error_recovery_dialog_generic_fatal: NULL parameters");
        return ERROR_DIALOG_RESULT_OK;
    }

    LOG_ERROR("Displaying generic fatal error dialog: %s", title);

    gboolean shown = error_dialog_show(ERROR_DIALOG_GENERIC_ERROR, title, message);

    if (shown) {
        LOG_INFO("User acknowledged generic fatal error");
        return ERROR_DIALOG_RESULT_OK;
    } else {
        LOG_WARNING("Failed to display generic fatal error dialog");
        return ERROR_DIALOG_RESULT_OK;
    }
}

/**
 * Display an informational dialog about recovery attempt
 */
void error_recovery_dialog_recovery_attempt(const gchar *recovery_type, int attempt)
{
    if (!recovery_type) {
        return;
    }

    LOG_INFO("Displaying recovery attempt dialog: %s (attempt %d)", recovery_type, attempt);

    gchar title[256];
    gchar message[512];

    g_snprintf(title, sizeof(title), "Recovering from %s Error", recovery_type);
    g_snprintf(message, sizeof(message),
               "The application detected an issue with %s and is attempting automatic recovery. "
               "Recovery attempt: %d. Please wait...",
               recovery_type, attempt);

    // This would display a non-modal informational dialog in a real implementation
    // For now, we just log the attempt
    LOG_INFO("Recovery attempt message would show: %s - %s", title, message);
}
