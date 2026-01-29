/**
 * @file keyboard_event_recovery.c
 * @brief Keyboard event error recovery and resilience implementation
 *
 * Implements error recovery mechanisms for keyboard event handling.
 * Tracks errors, schedules recovery attempts, and gracefully handles
 * keyboard handler failures.
 */

#include "keyboard_event_recovery.h"
#include "../utils/logging.h"
#include "../utils/timing.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

/**
 * Global keyboard recovery state
 */
static KeyboardRecoveryState g_recovery_state = {
    .error_count = 0, .recovery_attempts = 0, .last_error_time = 0, .should_reinitialize = FALSE};

static GMutex g_recovery_mutex;
static gboolean g_recovery_initialized = FALSE;

/**
 * Recovery constants
 */
#define KEYBOARD_ERROR_THRESHOLD 5          /* Errors before recovery triggered */
#define KEYBOARD_RECOVERY_WINDOW_US 1000000 /* 1 second window for error counting */
#define KEYBOARD_MAX_RECOVERY_ATTEMPTS 3    /* Max recovery attempts before giving up */

/**
 * Initialize keyboard event recovery system
 */
void keyboard_recovery_init(void)
{
    if (g_recovery_initialized) {
        return;
    }

    g_mutex_init(&g_recovery_mutex);
    memset(&g_recovery_state, 0, sizeof(KeyboardRecoveryState));
    g_recovery_initialized = TRUE;

    LOG_DEBUG("Keyboard recovery system initialized");
}

/**
 * Handle keyboard handler error with recovery attempt
 */
gboolean keyboard_recovery_handle_error(int error_code, const gchar *error_message)
{
    if (!g_recovery_initialized) {
        keyboard_recovery_init();
    }

    g_mutex_lock(&g_recovery_mutex);

    guint64 current_time = timing_get_monotonic_us();

    // Check if we're in a new error window (> 1 second since last error)
    if (g_recovery_state.last_error_time > 0) {
        guint64 time_since_last_error = current_time - g_recovery_state.last_error_time;
        if (time_since_last_error > KEYBOARD_RECOVERY_WINDOW_US) {
            // Reset error counter for new window
            g_recovery_state.error_count = 0;
        }
    }

    // Increment error counter
    g_recovery_state.error_count++;
    g_recovery_state.last_error_time = current_time;

    LOG_WARNING("Keyboard error [%d]: %s (error_count=%d, recovery_attempts=%d)", error_code,
                error_message, g_recovery_state.error_count, g_recovery_state.recovery_attempts);

    // Check if recovery is needed
    gboolean needs_recovery = (g_recovery_state.error_count >= KEYBOARD_ERROR_THRESHOLD);

    if (needs_recovery && g_recovery_state.recovery_attempts < KEYBOARD_MAX_RECOVERY_ATTEMPTS) {
        // Mark for recovery on next event
        g_recovery_state.should_reinitialize = TRUE;
        g_recovery_state.recovery_attempts++;

        LOG_INFO("Scheduling keyboard handler recovery (attempt %d/%d)",
                 g_recovery_state.recovery_attempts, KEYBOARD_MAX_RECOVERY_ATTEMPTS);

        g_mutex_unlock(&g_recovery_mutex);
        return TRUE;
    } else if (needs_recovery) {
        LOG_ERROR("Keyboard handler recovery failed after %d attempts; giving up",
                  KEYBOARD_MAX_RECOVERY_ATTEMPTS);
        g_mutex_unlock(&g_recovery_mutex);
        return FALSE;
    }

    g_mutex_unlock(&g_recovery_mutex);
    return TRUE;
}

/**
 * Schedule keyboard handler reinitialization
 */
gboolean keyboard_recovery_schedule_reinitialize(KeyboardErrorRecoveryStrategy recovery_strategy)
{
    if (!g_recovery_initialized) {
        keyboard_recovery_init();
    }

    g_mutex_lock(&g_recovery_mutex);

    const gchar *strategy_name = "UNKNOWN";
    switch (recovery_strategy) {
    case KEYBOARD_RECOVERY_REINITIALIZE:
        strategy_name = "REINITIALIZE";
        break;
    case KEYBOARD_RECOVERY_FALLBACK:
        strategy_name = "FALLBACK";
        break;
    case KEYBOARD_RECOVERY_RETRY:
        strategy_name = "RETRY";
        break;
    case KEYBOARD_RECOVERY_IGNORE:
        strategy_name = "IGNORE";
        break;
    }

    if (g_recovery_state.should_reinitialize) {
        LOG_DEBUG("Recovery already scheduled (strategy: %s)", strategy_name);
        g_mutex_unlock(&g_recovery_mutex);
        return FALSE;
    }

    g_recovery_state.should_reinitialize = TRUE;
    LOG_INFO("Keyboard recovery scheduled (strategy: %s)", strategy_name);

    g_mutex_unlock(&g_recovery_mutex);
    return TRUE;
}

/**
 * Check if keyboard handler needs recovery
 */
gboolean keyboard_recovery_is_needed(void)
{
    if (!g_recovery_initialized) {
        return FALSE;
    }

    g_mutex_lock(&g_recovery_mutex);

    // Check if recovery is scheduled
    if (g_recovery_state.should_reinitialize) {
        g_mutex_unlock(&g_recovery_mutex);
        return TRUE;
    }

    // Check if error threshold exceeded
    if (g_recovery_state.error_count >= KEYBOARD_ERROR_THRESHOLD) {
        g_mutex_unlock(&g_recovery_mutex);
        return TRUE;
    }

    g_mutex_unlock(&g_recovery_mutex);
    return FALSE;
}

/**
 * Get keyboard recovery state
 */
KeyboardRecoveryState keyboard_recovery_get_state(void)
{
    if (!g_recovery_initialized) {
        memset(&g_recovery_state, 0, sizeof(KeyboardRecoveryState));
        return g_recovery_state;
    }

    g_mutex_lock(&g_recovery_mutex);
    KeyboardRecoveryState state = g_recovery_state;
    g_mutex_unlock(&g_recovery_mutex);

    return state;
}

/**
 * Reset keyboard recovery state
 */
void keyboard_recovery_reset(void)
{
    if (!g_recovery_initialized) {
        return;
    }

    g_mutex_lock(&g_recovery_mutex);

    g_recovery_state.error_count = 0;
    g_recovery_state.recovery_attempts = 0;
    g_recovery_state.last_error_time = 0;
    g_recovery_state.should_reinitialize = FALSE;

    LOG_DEBUG("Keyboard recovery state reset");

    g_mutex_unlock(&g_recovery_mutex);
}

/**
 * Cleanup keyboard event recovery system
 */
void keyboard_recovery_cleanup(void)
{
    if (!g_recovery_initialized) {
        return;
    }

    g_mutex_lock(&g_recovery_mutex);

    memset(&g_recovery_state, 0, sizeof(KeyboardRecoveryState));

    g_mutex_unlock(&g_recovery_mutex);
    g_mutex_clear(&g_recovery_mutex);

    g_recovery_initialized = FALSE;

    LOG_DEBUG("Keyboard recovery system cleaned up");
}
