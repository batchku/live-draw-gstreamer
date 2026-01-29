/**
 * @file window_event_recovery.c
 * @brief OS X window event error recovery and resilience implementation
 *
 * Implements error recovery mechanisms for window events.
 * Tracks window visibility, rendering status, and schedules recovery attempts.
 */

#include "window_event_recovery.h"
#include "../utils/logging.h"
#include "../utils/timing.h"
#include <glib.h>
#include <stdlib.h>
#include <string.h>

/**
 * Global window recovery state
 */
typedef struct {
    WindowRecoveryState *states; /* Per-window recovery states */
    guint states_capacity;       /* Allocated capacity */
    guint states_count;          /* Current count */
    GMutex mutex;                /* Thread safety */
    gboolean initialized;        /* Initialization flag */
} WindowRecoveryGlobalState;

static WindowRecoveryGlobalState g_window_recovery = {
    .states = NULL, .states_capacity = 0, .states_count = 0, .initialized = FALSE};

/**
 * Recovery constants
 */
#define WINDOW_ERROR_THRESHOLD 3          /* Errors before recovery triggered */
#define WINDOW_RECOVERY_WINDOW_US 5000000 /* 5 second window for error counting */
#define WINDOW_MAX_RECOVERY_ATTEMPTS 2    /* Max recovery attempts before giving up */
#define WINDOW_INITIAL_STATES_CAPACITY 4  /* Initial capacity for window states */

/**
 * Initialize window event recovery system
 */
void window_recovery_init(void)
{
    if (g_window_recovery.initialized) {
        return;
    }

    g_mutex_init(&g_window_recovery.mutex);
    g_window_recovery.states_capacity = WINDOW_INITIAL_STATES_CAPACITY;
    g_window_recovery.states =
        g_malloc0(g_window_recovery.states_capacity * sizeof(WindowRecoveryState));
    g_window_recovery.states_count = 0;
    g_window_recovery.initialized = TRUE;

    LOG_DEBUG("Window recovery system initialized");
}

/**
 * Find or allocate recovery state for a window
 */
static WindowRecoveryState *window_recovery_get_or_allocate_state(OSXWindow *win)
{
    if (!win || !g_window_recovery.initialized) {
        return NULL;
    }

    g_mutex_lock(&g_window_recovery.mutex);

    // Linear search for existing window state (keyed by pointer value)
    for (guint i = 0; i < g_window_recovery.states_count; i++) {
        // Note: In this simplified implementation, we use index 0 for the primary window
        if (i == 0) {
            g_mutex_unlock(&g_window_recovery.mutex);
            return &g_window_recovery.states[0];
        }
    }

    // Allocate new state if capacity allows
    if (g_window_recovery.states_count < g_window_recovery.states_capacity) {
        WindowRecoveryState *state = &g_window_recovery.states[g_window_recovery.states_count];
        memset(state, 0, sizeof(WindowRecoveryState));
        g_window_recovery.states_count++;

        g_mutex_unlock(&g_window_recovery.mutex);
        return state;
    }

    // Expand capacity
    guint new_capacity = g_window_recovery.states_capacity * 2;
    WindowRecoveryState *new_states =
        g_realloc(g_window_recovery.states, new_capacity * sizeof(WindowRecoveryState));

    g_window_recovery.states = new_states;
    g_window_recovery.states_capacity = new_capacity;

    WindowRecoveryState *state = &g_window_recovery.states[g_window_recovery.states_count];
    memset(state, 0, sizeof(WindowRecoveryState));
    g_window_recovery.states_count++;

    g_mutex_unlock(&g_window_recovery.mutex);
    return state;
}

/**
 * Handle window error with recovery attempt
 */
gboolean window_recovery_handle_error(int error_code, const gchar *error_message, OSXWindow *win)
{
    if (!g_window_recovery.initialized) {
        window_recovery_init();
    }

    WindowRecoveryState *state = window_recovery_get_or_allocate_state(win);
    if (!state) {
        LOG_ERROR("Failed to allocate window recovery state");
        return FALSE;
    }

    g_mutex_lock(&g_window_recovery.mutex);

    guint64 current_time = timing_get_monotonic_us();

    // Check if we're in a new error window (> 5 seconds since last error)
    if (state->last_error_time > 0) {
        guint64 time_since_last_error = current_time - state->last_error_time;
        if (time_since_last_error > WINDOW_RECOVERY_WINDOW_US) {
            // Reset error counter for new window
            state->error_count = 0;
        }
    }

    // Increment error counter
    state->error_count++;
    state->last_error_time = current_time;

    const gchar *error_name = "UNKNOWN";
    switch (error_code) {
    case WINDOW_ERROR_WINDOW_NOT_FOUND:
        error_name = "WINDOW_NOT_FOUND";
        break;
    case WINDOW_ERROR_VIDEOSINK_MISSING:
        error_name = "VIDEOSINK_MISSING";
        break;
    case WINDOW_ERROR_VISIBILITY_LOST:
        error_name = "VISIBILITY_LOST";
        state->visibility_lost = TRUE;
        break;
    case WINDOW_ERROR_RENDERING_FAILED:
        error_name = "RENDERING_FAILED";
        state->rendering_failed = TRUE;
        break;
    case WINDOW_ERROR_RESIZE_FAILED:
        error_name = "RESIZE_FAILED";
        break;
    case WINDOW_ERROR_FRAME_UPDATE_FAILED:
        error_name = "FRAME_UPDATE_FAILED";
        break;
    default:
        break;
    }

    LOG_WARNING("Window error [%s]: %s (error_count=%d, recovery_attempts=%d)", error_name,
                error_message, state->error_count, state->recovery_attempts);

    // Check if recovery is needed
    gboolean needs_recovery = (state->error_count >= WINDOW_ERROR_THRESHOLD);

    if (needs_recovery && state->recovery_attempts < WINDOW_MAX_RECOVERY_ATTEMPTS) {
        // Mark for recovery on next event
        state->should_recreate = TRUE;
        state->recovery_attempts++;

        LOG_INFO("Scheduling window recovery (attempt %d/%d)", state->recovery_attempts,
                 WINDOW_MAX_RECOVERY_ATTEMPTS);

        g_mutex_unlock(&g_window_recovery.mutex);
        return TRUE;
    } else if (needs_recovery) {
        LOG_ERROR("Window recovery failed after %d attempts; giving up",
                  WINDOW_MAX_RECOVERY_ATTEMPTS);
        g_mutex_unlock(&g_window_recovery.mutex);
        return FALSE;
    }

    g_mutex_unlock(&g_window_recovery.mutex);
    return TRUE;
}

/**
 * Check if window visibility has been lost
 */
gboolean window_recovery_check_visibility(OSXWindow *win)
{
    if (!win) {
        return FALSE;
    }

    gboolean visible = window_is_visible(win);
    if (!visible) {
        LOG_WARNING("Window visibility lost; scheduling recovery");
        window_recovery_handle_error(WINDOW_ERROR_VISIBILITY_LOST, "Window not visible", win);
        return TRUE;
    }

    return FALSE;
}

/**
 * Attempt to restore window visibility
 */
gboolean window_recovery_restore_visibility(OSXWindow *win)
{
    if (!win || !win->nswindow) {
        LOG_ERROR("Cannot restore visibility: invalid window");
        return FALSE;
    }

    // Note: Actual visibility restoration would be implemented in window.m
    // via Objective-C NSWindow methods: makeKeyAndOrderFront:, orderFrontRegardless, etc.
    LOG_INFO("Attempting to restore window visibility");

    // For now, log the attempt; actual implementation in window.m
    window_request_render(win);

    return TRUE;
}

/**
 * Schedule window recovery
 */
gboolean window_recovery_schedule_recovery(OSXWindow *win,
                                           WindowErrorRecoveryStrategy recovery_strategy)
{
    if (!win || !g_window_recovery.initialized) {
        return FALSE;
    }

    WindowRecoveryState *state = window_recovery_get_or_allocate_state(win);
    if (!state) {
        return FALSE;
    }

    g_mutex_lock(&g_window_recovery.mutex);

    const gchar *strategy_name = "UNKNOWN";
    switch (recovery_strategy) {
    case WINDOW_RECOVERY_RECREATE:
        strategy_name = "RECREATE";
        state->should_recreate = TRUE;
        break;
    case WINDOW_RECOVERY_RESTORE_VISIBILITY:
        strategy_name = "RESTORE_VISIBILITY";
        state->visibility_lost = TRUE;
        break;
    case WINDOW_RECOVERY_RESET_RENDERING:
        strategy_name = "RESET_RENDERING";
        state->rendering_failed = TRUE;
        break;
    case WINDOW_RECOVERY_IGNORE:
        strategy_name = "IGNORE";
        break;
    }

    LOG_INFO("Window recovery scheduled (strategy: %s)", strategy_name);

    g_mutex_unlock(&g_window_recovery.mutex);
    return TRUE;
}

/**
 * Check if window needs recovery
 */
gboolean window_recovery_is_needed(OSXWindow *win)
{
    if (!win || !g_window_recovery.initialized) {
        return FALSE;
    }

    WindowRecoveryState *state = window_recovery_get_or_allocate_state(win);
    if (!state) {
        return FALSE;
    }

    g_mutex_lock(&g_window_recovery.mutex);

    gboolean needs_recovery = state->should_recreate || state->visibility_lost ||
                              (state->error_count >= WINDOW_ERROR_THRESHOLD);

    g_mutex_unlock(&g_window_recovery.mutex);

    return needs_recovery;
}

/**
 * Get window recovery state
 */
WindowRecoveryState window_recovery_get_state(OSXWindow *win)
{
    WindowRecoveryState empty_state;
    memset(&empty_state, 0, sizeof(WindowRecoveryState));

    if (!win || !g_window_recovery.initialized) {
        return empty_state;
    }

    WindowRecoveryState *state = window_recovery_get_or_allocate_state(win);
    if (!state) {
        return empty_state;
    }

    g_mutex_lock(&g_window_recovery.mutex);
    WindowRecoveryState copy = *state;
    g_mutex_unlock(&g_window_recovery.mutex);

    return copy;
}

/**
 * Reset window recovery state
 */
void window_recovery_reset(OSXWindow *win)
{
    if (!win || !g_window_recovery.initialized) {
        return;
    }

    WindowRecoveryState *state = window_recovery_get_or_allocate_state(win);
    if (!state) {
        return;
    }

    g_mutex_lock(&g_window_recovery.mutex);

    state->error_count = 0;
    state->recovery_attempts = 0;
    state->last_error_time = 0;
    state->visibility_lost = FALSE;
    state->rendering_failed = FALSE;
    state->should_recreate = FALSE;

    LOG_DEBUG("Window recovery state reset");

    g_mutex_unlock(&g_window_recovery.mutex);
}

/**
 * Cleanup window event recovery system
 */
void window_recovery_cleanup(void)
{
    if (!g_window_recovery.initialized) {
        return;
    }

    g_mutex_lock(&g_window_recovery.mutex);

    if (g_window_recovery.states) {
        g_free(g_window_recovery.states);
        g_window_recovery.states = NULL;
    }

    g_window_recovery.states_capacity = 0;
    g_window_recovery.states_count = 0;

    g_mutex_unlock(&g_window_recovery.mutex);
    g_mutex_clear(&g_window_recovery.mutex);

    g_window_recovery.initialized = FALSE;

    LOG_DEBUG("Window recovery system cleaned up");
}
