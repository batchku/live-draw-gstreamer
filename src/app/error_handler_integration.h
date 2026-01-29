/**
 * @file error_handler_integration.h
 * @brief Unified error handling and recovery integration
 *
 * Integrates keyboard and window event error recovery with user-friendly
 * error dialogs. Provides a central point for handling all application errors
 * and coordinating recovery attempts.
 *
 * Architecture:
 * - error_handler_integration_init() initializes recovery systems
 * - Components (keyboard, window, camera, GStreamer) report errors via callbacks
 * - Recovery is attempted based on error type and severity
 * - Fatal errors display user dialogs
 * - Recoverable errors schedule automatic recovery
 */

#ifndef ERROR_HANDLER_INTEGRATION_H
#define ERROR_HANDLER_INTEGRATION_H

#include <glib.h>

/**
 * @enum ErrorSeverity
 * @brief Severity level of an error
 */
typedef enum {
    ERROR_SEVERITY_INFO,    /**< Informational message */
    ERROR_SEVERITY_WARNING, /**< Warning, non-critical */
    ERROR_SEVERITY_ERROR,   /**< Error, operation failed */
    ERROR_SEVERITY_FATAL,   /**< Fatal error, recovery not possible */
} ErrorSeverity;

/**
 * @enum ErrorComponent
 * @brief Component that reported the error
 */
typedef enum {
    ERROR_COMPONENT_KEYBOARD,
    ERROR_COMPONENT_WINDOW,
    ERROR_COMPONENT_CAMERA,
    ERROR_COMPONENT_GSTREAMER,
    ERROR_COMPONENT_APPLICATION,
} ErrorComponent;

/**
 * @struct ErrorHandlerContext
 * @brief Context for integrated error handling
 */
typedef struct {
    ErrorComponent component;   /**< Component that reported error */
    ErrorSeverity severity;     /**< Error severity level */
    gint error_code;            /**< Component-specific error code */
    gchar *error_message;       /**< Descriptive error message */
    guint64 timestamp;          /**< When error occurred (microseconds) */
    gpointer component_context; /**< Component-specific context (window, etc) */
} ErrorHandlerContext;

/**
 * Initialize unified error handling and recovery system
 *
 * Initializes all sub-systems:
 * - Keyboard recovery
 * - Window recovery
 * - Error dialogs
 * - Error logging
 *
 * Should be called once during application startup, after basic logging is initialized.
 */
void error_handler_integration_init(void);

/**
 * Handle error with integrated recovery
 *
 * Central error handling function that:
 * 1. Logs the error
 * 2. Determines if recovery is possible
 * 3. Schedules recovery if needed
 * 4. Displays error dialogs for fatal errors
 *
 * @param context Error context with component, severity, and message
 * @return TRUE if error was handled (recovery scheduled or logged), FALSE if fatal
 *
 * @note This function is called by component error handlers
 */
gboolean error_handler_integration_handle_error(const ErrorHandlerContext *context);

/**
 * Check if error recovery is needed for any component
 *
 * Scans all recovery systems and returns TRUE if any component needs recovery.
 *
 * @return TRUE if recovery is needed, FALSE otherwise
 */
gboolean error_handler_integration_recovery_needed(void);

/**
 * Attempt recovery for all components
 *
 * Calls recovery functions for all components that need recovery.
 * Returns TRUE if all recoveries were successful, FALSE if recovery failed.
 *
 * @return TRUE if recovery was successful, FALSE if recovery failed
 */
gboolean error_handler_integration_attempt_recovery(void);

/**
 * Get summary of errors and recovery attempts
 *
 * Returns a string summary of all errors encountered and recovery attempts.
 * Useful for logging and diagnostics.
 *
 * @param out_summary Output buffer for summary string (must be pre-allocated)
 * @param max_len Maximum length of summary string
 */
void error_handler_integration_get_summary(gchar *out_summary, guint max_len);

/**
 * Reset error counters and recovery state
 *
 * Clears error counters and recovery flags for all components.
 * Called after successful recovery or at certain system checkpoints.
 */
void error_handler_integration_reset_state(void);

/**
 * Cleanup unified error handling system
 *
 * Releases all resources allocated by error_handler_integration_init().
 * Should be called during application shutdown.
 */
void error_handler_integration_cleanup(void);

/**
 * Display a fatal error dialog and prepare for shutdown
 *
 * Shows a user-friendly fatal error dialog and prepares the application
 * for graceful shutdown. The application should exit shortly after calling
 * this function.
 *
 * @param title Dialog title
 * @param message Dialog message body
 * @param exit_code Exit code to use when exiting (passed to app for exit())
 */
void error_handler_integration_fatal_error(const gchar *title, const gchar *message,
                                           gint exit_code);

#endif /* ERROR_HANDLER_INTEGRATION_H */
