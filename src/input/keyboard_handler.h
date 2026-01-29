/**
 * @file keyboard_handler.h
 * @brief Keyboard input capture and event dispatch
 *
 * Low-level keyboard input handling module that captures keyboard events
 * and dispatches them to registered callbacks.
 *
 * This module:
 * - Initializes the keyboard event handler with a callback function
 * - Processes key press/release events from the GTK+ event loop
 * - Translates physical key codes to logical key numbers
 * - Dispatches events (press/release) to the registered callback
 *
 * Key mappings:
 * - Physical keys 1-0 → Logical keys 1-10 (for video recording)
 * - Shift+1-0 → Logical keys 11-20 (for video recording)
 * - Escape key → Quit signal
 * - All other keys are ignored
 *
 * Latency targets:
 * - Key press event dispatched within <50ms of user action
 * - Processing is synchronous from keyboard_on_event()
 *
 * Thread safety:
 * - All functions must be called from the main GTK+ event loop thread
 * - No internal threading or synchronization
 */

#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

#include <glib.h>

/**
 * @brief Keyboard event callback function type
 *
 * Invoked when a keyboard key is pressed or released.
 *
 * @param key_number Logical key number:
 *                   - 1-20: Recording keys (layers)
 *                   - -1: Quit key (Escape)
 * @param is_pressed TRUE if key was pressed, FALSE if released
 *
 * @note Callback is invoked synchronously from keyboard_on_event()
 * @note Callback is invoked from the GTK+ event loop thread
 */
typedef void (*KeyEventCallback)(int key_number, gboolean is_pressed);

/**
 * @brief Initialize keyboard input handler
 *
 * Registers a callback function for keyboard events. The callback will be
 * invoked for all key press and release events matching the supported keys
 * (1-20 via 1-0 and Shift+1-0, Escape).
 *
 * This function must be called during application startup, before the GTK+
 * main event loop begins.
 *
 * @param on_key_event Callback function for key events.
 *                     Must not be NULL.
 *                     Will be invoked with key number and press/release state.
 *
 * @note Can only be called once; subsequent calls are logged as warnings
 * @note Must be called before GTK+ main loop starts
 * @note Callback is invoked from the GTK+ event loop thread
 */
void keyboard_init(KeyEventCallback on_key_event);

/**
 * @brief Process a keyboard event
 *
 * Called by the GTK+ event handler for each keyboard event. Translates the
 * physical key code to a logical key number and dispatches the event if it
 * matches a supported key.
 *
 * Supported keys:
 * - 1-20: Recording keys (logical key numbers 1-20)
 * - Escape: Quit key (logical key number -1)
 * - All other keys: Ignored silently
 *
 * Key press/release states are tracked and logged for diagnostics.
 *
 * @param key_code Physical key code from the operating system.
 *                 On macOS, this is the NSEvent keyCode value.
 * @param is_shifted TRUE if shift modifier is active
 * @param is_pressed TRUE if key was pressed, FALSE if released
 *
 * @note This function must be called from the GTK+ event loop thread
 * @note Unknown keys are ignored without logging
 * @note If handler not initialized, logs a warning and returns
 * @note If no callback registered, logs a warning and returns
 *
 * Typical event latency: <50ms from user key press to callback invocation
 */
void keyboard_on_event(int key_code, gboolean is_shifted, gboolean is_pressed);

/**
 * @brief Clean up keyboard input handler
 *
 * Unregisters the keyboard event callback and releases handler resources.
 * The handler can be re-initialized with keyboard_init() after cleanup.
 *
 * Can be called multiple times; subsequent calls are no-ops.
 *
 * @note Must be called during application shutdown
 * @note Safe to call even if keyboard_init() was not called
 */
void keyboard_cleanup(void);

#endif // KEYBOARD_HANDLER_H
