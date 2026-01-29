/**
 * @file error_handler_integration.c
 * @brief Unified error handling and recovery integration implementation
 *
 * Central coordination point for all error handling and recovery.
 * Integrates keyboard recovery, window recovery, error dialogs, and logging.
 */

#include "error_handler_integration.h"
#include "../input/keyboard_event_recovery.h"
#include "../osx/window_event_recovery.h"
#include "../utils/logging.h"
#include "error_recovery_dialog.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

/**
 * Global error handler integration state
 */
typedef struct {
    gint total_errors;       /**< Total error count */
    gint recoverable_errors; /**< Count of recoverable errors */
    gint fatal_errors;       /**< Count of fatal errors */
    guint64 startup_time;    /**< Application startup time */
    GMutex state_mutex;      /**< Thread safety */
    gboolean initialized;    /**< Initialization flag */
} ErrorHandlerIntegrationState;

static ErrorHandlerIntegrationState g_integration_state = {.total_errors = 0,
                                                           .recoverable_errors = 0,
                                                           .fatal_errors = 0,
                                                           .startup_time = 0,
                                                           .initialized = FALSE};

/**
 * Initialize unified error handling and recovery system
 */
void error_handler_integration_init(void)
{
    if (g_integration_state.initialized) {
        return;
    }

    LOG_INFO("Initializing unified error handling and recovery system");

    // Initialize sub-systems
    keyboard_recovery_init();
    window_recovery_init();

    // Initialize state
    g_mutex_init(&g_integration_state.state_mutex);
    g_integration_state.total_errors = 0;
    g_integration_state.recoverable_errors = 0;
    g_integration_state.fatal_errors = 0;
    g_integration_state.startup_time = g_get_monotonic_time() / 1000; // microseconds
    g_integration_state.initialized = TRUE;

    LOG_INFO("Error handling integration system initialized successfully");
}

/**
 * Handle error with integrated recovery
 */
gboolean error_handler_integration_handle_error(const ErrorHandlerContext *context)
{
    if (!context) {
        LOG_ERROR("error_handler_integration_handle_error: NULL context");
        return FALSE;
    }

    if (!g_integration_state.initialized) {
        error_handler_integration_init();
    }

    g_mutex_lock(&g_integration_state.state_mutex);

    g_integration_state.total_errors++;

    const gchar *component_name = "UNKNOWN";
    switch (context->component) {
    case ERROR_COMPONENT_KEYBOARD:
        component_name = "KEYBOARD";
        break;
    case ERROR_COMPONENT_WINDOW:
        component_name = "WINDOW";
        break;
    case ERROR_COMPONENT_CAMERA:
        component_name = "CAMERA";
        break;
    case ERROR_COMPONENT_GSTREAMER:
        component_name = "GSTREAMER";
        break;
    case ERROR_COMPONENT_APPLICATION:
        component_name = "APPLICATION";
        break;
    }

    const gchar *severity_name = "UNKNOWN";
    switch (context->severity) {
    case ERROR_SEVERITY_INFO:
        severity_name = "INFO";
        break;
    case ERROR_SEVERITY_WARNING:
        severity_name = "WARNING";
        break;
    case ERROR_SEVERITY_ERROR:
        severity_name = "ERROR";
        break;
    case ERROR_SEVERITY_FATAL:
        severity_name = "FATAL";
        break;
    }

    LOG_ERROR("[%s:%s] Code %d: %s", component_name, severity_name, context->error_code,
              context->error_message ? context->error_message : "(no message)");

    gboolean is_recoverable = (context->severity != ERROR_SEVERITY_FATAL);

    if (is_recoverable) {
        g_integration_state.recoverable_errors++;

        // Attempt recovery based on component
        gboolean recovery_scheduled = FALSE;

        switch (context->component) {
        case ERROR_COMPONENT_KEYBOARD: {
            recovery_scheduled = keyboard_recovery_handle_error(
                context->error_code,
                context->error_message ? context->error_message : "Unknown keyboard error");
            break;
        }

        case ERROR_COMPONENT_WINDOW: {
            recovery_scheduled = window_recovery_handle_error(
                context->error_code,
                context->error_message ? context->error_message : "Unknown window error",
                (void *) context->component_context);
            break;
        }

        default:
            // For other components, just log the error
            LOG_WARNING("Recovery not available for component %s", component_name);
            break;
        }

        if (recovery_scheduled) {
            LOG_INFO("Recovery scheduled for %s error", component_name);
            g_mutex_unlock(&g_integration_state.state_mutex);
            return TRUE;
        }
    } else {
        // Fatal error
        g_integration_state.fatal_errors++;

        g_mutex_unlock(&g_integration_state.state_mutex);

        // Show fatal error dialog
        error_recovery_dialog_generic_fatal(
            "Fatal Application Error",
            context->error_message ? context->error_message : "An unexpected fatal error occurred");

        return FALSE;
    }

    g_mutex_unlock(&g_integration_state.state_mutex);
    return TRUE;
}

/**
 * Check if error recovery is needed for any component
 */
gboolean error_handler_integration_recovery_needed(void)
{
    if (!g_integration_state.initialized) {
        return FALSE;
    }

    gboolean keyboard_needs_recovery = keyboard_recovery_is_needed();
    gboolean window_needs_recovery = window_recovery_is_needed(NULL); // NULL for primary window

    return keyboard_needs_recovery || window_needs_recovery;
}

/**
 * Attempt recovery for all components
 */
gboolean error_handler_integration_attempt_recovery(void)
{
    if (!g_integration_state.initialized) {
        LOG_WARNING("error_handler_integration_attempt_recovery: system not initialized");
        return FALSE;
    }

    LOG_INFO("Attempting error recovery for all components");

    gboolean all_successful = TRUE;

    // Check keyboard recovery
    if (keyboard_recovery_is_needed()) {
        LOG_INFO("Attempting keyboard handler recovery");
        KeyboardRecoveryState state = keyboard_recovery_get_state();

        if (state.recovery_attempts < 3) {
            // Keyboard recovery is already scheduled; just reset and continue
            keyboard_recovery_reset();
            LOG_INFO("Keyboard handler recovery completed");
        } else {
            LOG_ERROR("Keyboard handler recovery failed after %d attempts",
                      state.recovery_attempts);
            error_recovery_dialog_keyboard_failure(state.recovery_attempts);
            all_successful = FALSE;
        }
    }

    // Check window recovery
    if (window_recovery_is_needed(NULL)) {
        LOG_INFO("Attempting window recovery");
        WindowRecoveryState state = window_recovery_get_state(NULL);

        if (state.recovery_attempts < 2) {
            LOG_INFO("Window recovery completed");
            window_recovery_reset(NULL);
        } else {
            LOG_ERROR("Window recovery failed after %d attempts", state.recovery_attempts);
            error_recovery_dialog_window_failure("Max recovery attempts exceeded");
            all_successful = FALSE;
        }
    }

    if (all_successful) {
        LOG_INFO("All error recovery attempts completed successfully");
    } else {
        LOG_WARNING("Some error recovery attempts failed");
    }

    return all_successful;
}

/**
 * Get summary of errors and recovery attempts
 */
void error_handler_integration_get_summary(gchar *out_summary, guint max_len)
{
    if (!out_summary || max_len < 1) {
        return;
    }

    if (!g_integration_state.initialized) {
        g_snprintf(out_summary, max_len, "Error handling system not initialized");
        return;
    }

    g_mutex_lock(&g_integration_state.state_mutex);

    KeyboardRecoveryState kb_state = keyboard_recovery_get_state();
    WindowRecoveryState win_state = window_recovery_get_state(NULL);

    g_snprintf(out_summary, max_len,
               "Total errors: %d (recoverable: %d, fatal: %d) | "
               "Keyboard: %d errors, %d recovery attempts | "
               "Window: %d errors, %d recovery attempts",
               g_integration_state.total_errors, g_integration_state.recoverable_errors,
               g_integration_state.fatal_errors, kb_state.error_count, kb_state.recovery_attempts,
               win_state.error_count, win_state.recovery_attempts);

    g_mutex_unlock(&g_integration_state.state_mutex);
}

/**
 * Reset error counters and recovery state
 */
void error_handler_integration_reset_state(void)
{
    if (!g_integration_state.initialized) {
        return;
    }

    LOG_INFO("Resetting error handler state");

    g_mutex_lock(&g_integration_state.state_mutex);

    g_integration_state.total_errors = 0;
    g_integration_state.recoverable_errors = 0;
    g_integration_state.fatal_errors = 0;

    g_mutex_unlock(&g_integration_state.state_mutex);

    keyboard_recovery_reset();
    window_recovery_reset(NULL);

    LOG_INFO("Error handler state reset completed");
}

/**
 * Cleanup unified error handling system
 */
void error_handler_integration_cleanup(void)
{
    if (!g_integration_state.initialized) {
        return;
    }

    LOG_INFO("Cleaning up error handling integration system");

    // Cleanup sub-systems
    keyboard_recovery_cleanup();
    window_recovery_cleanup();

    g_mutex_lock(&g_integration_state.state_mutex);

    g_integration_state.total_errors = 0;
    g_integration_state.recoverable_errors = 0;
    g_integration_state.fatal_errors = 0;

    g_mutex_unlock(&g_integration_state.state_mutex);
    g_mutex_clear(&g_integration_state.state_mutex);

    g_integration_state.initialized = FALSE;

    LOG_DEBUG("Error handling integration system cleaned up");
}

/**
 * Display a fatal error dialog and prepare for shutdown
 */
void error_handler_integration_fatal_error(const gchar *title, const gchar *message, gint exit_code)
{
    if (!title || !message) {
        LOG_ERROR("error_handler_integration_fatal_error: NULL parameters");
        return;
    }

    LOG_ERROR("FATAL: %s: %s (exit_code=%d)", title, message, exit_code);

    if (!g_integration_state.initialized) {
        error_handler_integration_init();
    }

    g_mutex_lock(&g_integration_state.state_mutex);
    g_integration_state.fatal_errors++;
    g_mutex_unlock(&g_integration_state.state_mutex);

    // Display fatal error dialog
    error_recovery_dialog_generic_fatal(title, message);

    // Log summary before exit
    gchar summary[256];
    error_handler_integration_get_summary(summary, sizeof(summary));
    LOG_ERROR("Error summary at shutdown: %s", summary);

    // Cleanup system
    error_handler_integration_cleanup();
}
