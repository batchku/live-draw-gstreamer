/**
 * @file keyboard_event_recovery.h
 * @brief Keyboard event error recovery and resilience
 *
 * Implements error recovery and resilience mechanisms for keyboard event handling.
 * When the keyboard handler encounters errors (missing callbacks, uninitialized state),
 * this module attempts recovery and graceful degradation.
 *
 * Recovery strategies:
 * - Automatic reinitialize on next event if handler fails
 * - Fallback event dispatching if primary callback fails
 * - Detailed logging of recovery attempts
 */

#ifndef KEYBOARD_EVENT_RECOVERY_H
#define KEYBOARD_EVENT_RECOVERY_H

#include "keyboard_handler.h"
#include <glib.h>

/**
 * @enum KeyboardErrorRecoveryStrategy
 * @brief Strategy for recovering from keyboard handler failures
 */
typedef enum {
    KEYBOARD_RECOVERY_REINITIALIZE, /**< Attempt to reinitialize handler */
    KEYBOARD_RECOVERY_FALLBACK,     /**< Use fallback event dispatch */
    KEYBOARD_RECOVERY_RETRY,        /**< Retry failed operation */
    KEYBOARD_RECOVERY_IGNORE,       /**< Log and continue without recovery */
} KeyboardErrorRecoveryStrategy;

/**
 * @struct KeyboardRecoveryState
 * @brief Tracks keyboard recovery state and attempts
 */
typedef struct {
    gint error_count;             /**< Number of errors encountered */
    gint recovery_attempts;       /**< Number of recovery attempts */
    guint64 last_error_time;      /**< Timestamp of last error (microseconds) */
    gboolean should_reinitialize; /**< Flag to reinitialize on next event */
} KeyboardRecoveryState;

/**
 * Initialize keyboard event recovery system
 *
 * Allocates and initializes the recovery state tracker.
 * Should be called once during application startup.
 *
 * @note Called internally by keyboard_init()
 */
void keyboard_recovery_init(void);

/**
 * Handle keyboard handler error with recovery attempt
 *
 * Implements error recovery logic when keyboard handler encounters errors.
 * Logs the error, increments error counter, and determines recovery strategy.
 *
 * Recovery strategies:
 * - If handler not initialized: Mark for reinitialization on next event
 * - If callback missing: Log warning, attempt fallback dispatch
 * - If dispatch fails: Log error, schedule recovery retry
 *
 * @param error_code Error code from keyboard handler
 * @param error_message Descriptive error message
 * @return TRUE if recovery was attempted successfully, FALSE if unrecoverable
 *
 * @note This function is called automatically by keyboard_on_event()
 */
gboolean keyboard_recovery_handle_error(int error_code, const gchar *error_message);

/**
 * Attempt to recover keyboard handler on next event
 *
 * Called when a non-fatal keyboard error is detected. Sets a flag to
 * reinitialize the handler on the next keyboard event.
 *
 * @param recovery_strategy Strategy to use for recovery
 * @return TRUE if recovery was scheduled, FALSE if not needed
 */
gboolean keyboard_recovery_schedule_reinitialize(KeyboardErrorRecoveryStrategy recovery_strategy);

/**
 * Check if keyboard handler needs recovery
 *
 * Examines the recovery state and determines if recovery is needed.
 * Returns TRUE if the handler has encountered too many errors in a short time.
 *
 * Recovery threshold: > 5 errors in 1 second triggers recovery
 *
 * @return TRUE if recovery is needed, FALSE otherwise
 */
gboolean keyboard_recovery_is_needed(void);

/**
 * Get keyboard recovery state
 *
 * Returns a copy of the current recovery state for diagnostics/logging.
 *
 * @return KeyboardRecoveryState with current error and recovery counters
 */
KeyboardRecoveryState keyboard_recovery_get_state(void);

/**
 * Reset keyboard recovery state
 *
 * Clears error counters and recovery flags. Called after successful recovery
 * or during handler cleanup.
 */
void keyboard_recovery_reset(void);

/**
 * Cleanup keyboard event recovery system
 *
 * Releases recovery state resources and resets recovery tracking.
 * Should be called during application shutdown.
 */
void keyboard_recovery_cleanup(void);

#endif /* KEYBOARD_EVENT_RECOVERY_H */
