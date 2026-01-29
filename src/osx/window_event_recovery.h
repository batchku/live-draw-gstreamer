/**
 * @file window_event_recovery.h
 * @brief OS X window event error recovery and resilience
 *
 * Implements error recovery and resilience mechanisms for window events.
 * Detects window visibility issues, rendering failures, and resize errors,
 * then attempts recovery through automatic reinitialization or fallback modes.
 *
 * Recovery strategies:
 * - Detect window visibility loss and attempt recovery
 * - Handle rendering context failures with fallback
 * - Manage window resize errors gracefully
 * - Track and limit recovery attempts
 */

#ifndef WINDOW_EVENT_RECOVERY_H
#define WINDOW_EVENT_RECOVERY_H

#include "window.h"
#include <glib.h>

/**
 * @enum WindowErrorRecoveryStrategy
 * @brief Strategy for recovering from window failures
 */
typedef enum {
    WINDOW_RECOVERY_RECREATE,           /**< Recreate the window */
    WINDOW_RECOVERY_RESTORE_VISIBILITY, /**< Make window visible again */
    WINDOW_RECOVERY_RESET_RENDERING,    /**< Reset rendering context */
    WINDOW_RECOVERY_IGNORE,             /**< Log and continue */
} WindowErrorRecoveryStrategy;

/**
 * @enum WindowErrorCode
 * @brief Window-specific error codes
 */
typedef enum {
    WINDOW_ERROR_WINDOW_NOT_FOUND = 1000,
    WINDOW_ERROR_VIDEOSINK_MISSING = 1001,
    WINDOW_ERROR_VISIBILITY_LOST = 1002,
    WINDOW_ERROR_RENDERING_FAILED = 1003,
    WINDOW_ERROR_RESIZE_FAILED = 1004,
    WINDOW_ERROR_FRAME_UPDATE_FAILED = 1005,
    WINDOW_ERROR_UNKNOWN = 1099,
} WindowErrorCode;

/**
 * @struct WindowRecoveryState
 * @brief Tracks window recovery state and health metrics
 */
typedef struct {
    gint error_count;          /**< Number of errors encountered */
    gint recovery_attempts;    /**< Number of recovery attempts */
    guint64 last_error_time;   /**< Timestamp of last error (microseconds) */
    gboolean visibility_lost;  /**< Flag: window visibility lost */
    gboolean rendering_failed; /**< Flag: rendering context failed */
    gboolean should_recreate;  /**< Flag: window should be recreated */
} WindowRecoveryState;

/**
 * Initialize window event recovery system
 *
 * Allocates and initializes the recovery state tracker.
 * Should be called once during window creation.
 *
 * @note Called internally by window_create()
 */
void window_recovery_init(void);

/**
 * Handle window error with recovery attempt
 *
 * Implements error recovery logic when window encounters errors.
 * Logs the error, increments error counter, and determines recovery strategy.
 *
 * @param error_code Window error code
 * @param error_message Descriptive error message
 * @param win OS X window context (may be NULL for initialization errors)
 * @return TRUE if recovery was attempted successfully, FALSE if unrecoverable
 *
 * @note This function is called automatically by window event handlers
 */
gboolean window_recovery_handle_error(int error_code, const gchar *error_message, OSXWindow *win);

/**
 * Check if window visibility has been lost
 *
 * Periodically checks if the window is still visible. Returns TRUE if the
 * window has been hidden, minimized, or is otherwise not visible to the user.
 *
 * @param win OS X window context
 * @return TRUE if visibility was lost, FALSE if still visible
 */
gboolean window_recovery_check_visibility(OSXWindow *win);

/**
 * Attempt to restore window visibility
 *
 * Makes the window visible again after visibility is lost (e.g., after minimize).
 * Brings the window to front and makes it active.
 *
 * @param win OS X window context
 * @return TRUE if visibility was restored successfully, FALSE otherwise
 */
gboolean window_recovery_restore_visibility(OSXWindow *win);

/**
 * Schedule window recovery on next event
 *
 * Marks the window for recovery (recreation, visibility restoration, etc.)
 * on the next window event. Used to schedule deferred recovery.
 *
 * @param win OS X window context
 * @param recovery_strategy Strategy to use for recovery
 * @return TRUE if recovery was scheduled, FALSE if not needed
 */
gboolean window_recovery_schedule_recovery(OSXWindow *win,
                                           WindowErrorRecoveryStrategy recovery_strategy);

/**
 * Check if window needs recovery
 *
 * Examines the recovery state and determines if recovery is needed.
 * Returns TRUE if the window has encountered too many errors or visibility loss.
 *
 * @param win OS X window context
 * @return TRUE if recovery is needed, FALSE otherwise
 */
gboolean window_recovery_is_needed(OSXWindow *win);

/**
 * Get window recovery state
 *
 * Returns a copy of the current recovery state for diagnostics/logging.
 *
 * @param win OS X window context
 * @return WindowRecoveryState with current error and recovery counters
 */
WindowRecoveryState window_recovery_get_state(OSXWindow *win);

/**
 * Reset window recovery state
 *
 * Clears error counters and recovery flags. Called after successful recovery
 * or during window cleanup.
 *
 * @param win OS X window context
 */
void window_recovery_reset(OSXWindow *win);

/**
 * Cleanup window event recovery system
 *
 * Releases recovery state resources and resets recovery tracking.
 * Should be called during window cleanup.
 */
void window_recovery_cleanup(void);

#endif /* WINDOW_EVENT_RECOVERY_H */
