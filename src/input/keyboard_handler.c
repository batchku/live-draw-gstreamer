/**
 * @file keyboard_handler.c
 * @brief Keyboard input capture and event dispatch
 *
 * This module handles low-level keyboard input events and translates them
 * into application-level key events. It maintains a registry of key states
 * and dispatches press/release events to registered callbacks.
 *
 * Key mappings (macOS key codes):
 * - Keys 1-9: Video recording keys (mapped to cells 2-10)
 * - Escape: Application quit
 *
 * Architecture:
 * - keyboard_init() registers a callback for key events
 * - keyboard_on_event() is called by the GTK+ event handler for each key event
 * - Key codes are translated to logical key numbers via key_codes.h
 * - Callbacks are dispatched immediately for press/release events
 *
 * Thread Safety:
 * - All functions must be called from the main GTK+ event loop thread
 * - The callback is invoked synchronously from keyboard_on_event()
 * - No internal locking is used; GTK+ event loop is single-threaded
 */

#include "keyboard_handler.h"
#include "../utils/logging.h"
#include "key_codes.h"
#include <glib.h>
#include <string.h>

/**
 * @brief Global keyboard handler state
 *
 * Stores the registered callback and maintains key press state.
 */
typedef struct {
    KeyEventCallback callback; // Callback for key events
    gboolean key_pressed[256]; // Track which keys are currently pressed (key codes 0-255)
    gint handler_initialized;  // Initialization guard (atomic int)
} KeyboardHandlerState;

static KeyboardHandlerState g_keyboard_state = {.callback = NULL, .handler_initialized = FALSE};

/**
 * @brief Initialize keyboard input handler
 *
 * Registers a callback function that will be invoked when key events occur.
 * The callback receives:
 * - key_number: Logical key number (1-9 for recording, -1 for quit)
 * - is_pressed: TRUE for key press, FALSE for key release
 *
 * Can only be called once; subsequent calls are ignored with a warning.
 *
 * @param on_key_event Callback function to handle key events.
 *                     Must not be NULL.
 *
 * @note This function must be called before the GTK+ event loop starts
 * @note The callback is invoked from the GTK+ event loop thread
 */
void keyboard_init(KeyEventCallback on_key_event)
{
    if (on_key_event == NULL) {
        LOG_ERROR("keyboard_init: on_key_event callback cannot be NULL");
        return;
    }

    if (g_atomic_int_get(&g_keyboard_state.handler_initialized)) {
        LOG_WARNING("keyboard_init: already initialized, ignoring duplicate call");
        return;
    }

    g_keyboard_state.callback = on_key_event;
    memset(g_keyboard_state.key_pressed, FALSE, sizeof(g_keyboard_state.key_pressed));
    g_atomic_int_set(&g_keyboard_state.handler_initialized, TRUE);

    LOG_INFO("Keyboard handler initialized (keys 1-9, Escape)");
}

/**
 * @brief Process a keyboard event
 *
 * Called by the GTK+ event handler for each keyboard event. Translates the
 * physical key code to a logical key number and dispatches the event to the
 * registered callback.
 *
 * This function:
 * 1. Translates physical key code to logical key number
 * 2. Checks if the callback is registered
 * 3. Filters for valid recording keys (1-9) and quit key (Escape)
 * 4. Invokes the callback with the logical key number and press/release state
 * 5. Logs the event at DEBUG level for diagnostics
 *
 * Unknown keys are silently ignored (not logged, not dispatched).
 *
 * @param key_code Physical key code from the operating system
 *                 (macOS NSEvent keyCode or equivalent)
 * @param is_pressed TRUE if key was pressed, FALSE if released
 *
 * @note Thread-safe: must be called from the main GTK+ event loop thread
 * @note Unknown keys are ignored silently (no log message)
 * @note No error handling needed; invalid inputs are handled gracefully
 */
void keyboard_on_event(int key_code, gboolean is_pressed)
{
    // Check if handler is initialized
    if (!g_atomic_int_get(&g_keyboard_state.handler_initialized)) {
        LOG_WARNING("keyboard_on_event: handler not initialized");
        return;
    }

    if (g_keyboard_state.callback == NULL) {
        LOG_WARNING("keyboard_on_event: no callback registered");
        return;
    }

    // Translate physical key code to logical key number
    LogicalKeyNumber key_number = key_code_to_logical_key(key_code);

    // Ignore unknown keys silently
    if (key_number == KEY_UNKNOWN) {
        return;
    }

    // Track press/release state for logging
    if (key_code < 256) {
        if (is_pressed) {
            g_keyboard_state.key_pressed[key_code] = TRUE;
        } else {
            g_keyboard_state.key_pressed[key_code] = FALSE;
        }
    }

    // Log the event (DEBUG level for high-frequency events)
    if (key_is_recording_key(key_number)) {
        LOG_DEBUG("Key %d %s (recording key)", key_number, is_pressed ? "pressed" : "released");
    } else if (key_is_quit_key(key_number)) {
        LOG_DEBUG("Escape key %s (quit)", is_pressed ? "pressed" : "released");
    }

    // Dispatch the event to the registered callback
    g_keyboard_state.callback((int) key_number, is_pressed);
}

/**
 * @brief Clean up keyboard input handler
 *
 * Unregisters the keyboard event callback and resets the handler state.
 * Can be called multiple times; subsequent calls are no-ops.
 *
 * @note Must be called before application shutdown
 * @note After cleanup, keyboard_init() can be called again to re-register
 */
void keyboard_cleanup(void)
{
    if (!g_atomic_int_get(&g_keyboard_state.handler_initialized)) {
        return; // Already cleaned up
    }

    g_keyboard_state.callback = NULL;
    memset(g_keyboard_state.key_pressed, FALSE, sizeof(g_keyboard_state.key_pressed));
    g_atomic_int_set(&g_keyboard_state.handler_initialized, FALSE);

    LOG_DEBUG("Keyboard handler cleaned up");
}
