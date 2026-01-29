#ifndef GSTREAMER_ERROR_HANDLER_H
#define GSTREAMER_ERROR_HANDLER_H

#include <glib.h>
#include <gst/gst.h>

/**
 * @file gstreamer_error_handler.h
 * @brief GStreamer error handling, deadlock detection, and recovery mechanisms
 *
 * Provides comprehensive error handling for GStreamer pipelines including:
 * - Bus error message handling with categorization
 * - State change failure detection and recovery
 * - Deadlock detection using timeout-based state change monitoring
 * - Automatic recovery attempts (state rollback, forced READY)
 * - Comprehensive logging at all error levels
 */

/**
 * @enum GStreamerErrorCategory
 * @brief Categories of GStreamer errors for appropriate handling
 */
typedef enum {
    GSTREAMER_ERROR_BUS_ERROR,            /* ERROR message on bus */
    GSTREAMER_ERROR_STATE_CHANGE_FAILURE, /* Failed state transition */
    GSTREAMER_ERROR_DEADLOCK_DETECTED,    /* State change timeout (deadlock) */
    GSTREAMER_ERROR_ELEMENT_MISSING,      /* Element not available */
    GSTREAMER_ERROR_NEGOTIATION,          /* Caps negotiation failure */
    GSTREAMER_ERROR_RESOURCE,             /* Resource exhaustion */
    GSTREAMER_ERROR_UNKNOWN,              /* Uncategorized error */
} GStreamerErrorCategory;

/**
 * @struct GStreamerErrorInfo
 * @brief Detailed error information from GStreamer
 */
typedef struct {
    GStreamerErrorCategory category;
    const gchar *message;
    const gchar *debug_info;
    const gchar *source_element;
    GstState failed_state; /* The state change that failed, or GST_STATE_NULL */
    guint64 timestamp_us;  /* When the error occurred */
} GStreamerErrorInfo;

/**
 * Callback function type for GStreamer error notifications
 *
 * @param error_info Detailed error information
 * @param pipeline   The affected pipeline (or NULL if global error)
 * @param user_data  Optional user data provided at registration
 */
typedef void (*GStreamerErrorCallback)(const GStreamerErrorInfo *error_info, GstElement *pipeline,
                                       gpointer user_data);

/**
 * Callback function type for recovery notifications
 *
 * @param action     Human-readable description of recovery action
 * @param success    TRUE if recovery succeeded, FALSE if recovery failed
 * @param user_data  Optional user data provided at registration
 */
typedef void (*GStreamerRecoveryCallback)(const gchar *action, gboolean success,
                                          gpointer user_data);

/**
 * Initialize the GStreamer error handler system
 *
 * Sets up global error handling infrastructure. Must be called once
 * during application startup after GStreamer initialization.
 *
 * @return TRUE if initialization successful, FALSE on error
 */
gboolean gstreamer_error_handler_init(void);

/**
 * Cleanup the GStreamer error handler system
 *
 * Releases all allocated resources. Must be called during application
 * shutdown before GStreamer cleanup.
 */
void gstreamer_error_handler_cleanup(void);

/**
 * Register a callback for GStreamer error notifications
 *
 * Only one error callback can be registered at a time.
 *
 * @param callback  Error callback function (NULL to unregister)
 * @param user_data Optional user data to pass to callback
 */
void gstreamer_error_handler_register_error_callback(GStreamerErrorCallback callback,
                                                     gpointer user_data);

/**
 * Register a callback for recovery action notifications
 *
 * Only one recovery callback can be registered at a time.
 *
 * @param callback  Recovery callback function (NULL to unregister)
 * @param user_data Optional user data to pass to callback
 */
void gstreamer_error_handler_register_recovery_callback(GStreamerRecoveryCallback callback,
                                                        gpointer user_data);

/**
 * Monitor a pipeline for deadlocks during state changes
 *
 * Sets up timeout-based deadlock detection for the given pipeline.
 * If a state change takes longer than the timeout, it's treated as
 * a deadlock and recovery is attempted.
 *
 * @param pipeline      GStreamer pipeline to monitor
 * @param timeout_ms    Timeout in milliseconds (recommended: 5000-10000)
 * @return              TRUE if monitoring started, FALSE on error
 */
gboolean gstreamer_error_handler_enable_deadlock_detection(GstElement *pipeline, guint timeout_ms);

/**
 * Disable deadlock detection for a pipeline
 *
 * @param pipeline Pipeline to stop monitoring
 */
void gstreamer_error_handler_disable_deadlock_detection(GstElement *pipeline);

/**
 * Attempt recovery from a failed state change
 *
 * Recovery strategy:
 * 1. If state change to PLAYING/PAUSED failed, try reverting to previous state
 * 2. If revert fails, force transition to READY state
 * 3. If READY fails, force transition to NULL (stop)
 *
 * @param pipeline      GStreamer pipeline to recover
 * @param target_state  The state that was being transitioned to
 * @param previous_state The previous stable state
 * @return              TRUE if recovery succeeded, FALSE if pipeline is unrecoverable
 */
gboolean gstreamer_error_handler_attempt_recovery(GstElement *pipeline, GstState target_state,
                                                  GstState previous_state);

/**
 * Get the last recorded error information
 *
 * @return Pointer to error info, or NULL if no error recorded
 */
const GStreamerErrorInfo *gstreamer_error_handler_get_last_error(void);

/**
 * Clear the last recorded error
 */
void gstreamer_error_handler_clear_last_error(void);

/**
 * Get a human-readable string for an error category
 *
 * @param category Error category
 * @return         Static string describing the category
 */
const gchar *gstreamer_error_handler_category_to_string(GStreamerErrorCategory category);

/**
 * Perform state change with deadlock detection
 *
 * Wrapper around gst_element_set_state() that includes built-in
 * deadlock detection and automatic recovery attempts.
 *
 * @param pipeline      GStreamer pipeline
 * @param target_state  Target state to transition to
 * @param timeout_ms    Timeout in milliseconds (0 = no timeout, use default)
 * @return              TRUE if state change succeeded, FALSE on failure
 */
gboolean gstreamer_error_handler_set_state_with_detection(GstElement *pipeline,
                                                          GstState target_state, guint timeout_ms);

#endif /* GSTREAMER_ERROR_HANDLER_H */
