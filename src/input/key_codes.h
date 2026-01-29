/**
 * @file key_codes.h
 * @brief Key code definitions and mappings for keyboard input handling
 *
 * Provides macOS/platform-specific key code constants for keyboard input
 * processing. Maps physical keys to logical key numbers for the application.
 */

#ifndef KEY_CODES_H
#define KEY_CODES_H

#include <glib.h>

/**
 * @brief Logical key numbers for video looping
 *
 * Maps physical keyboard keys to logical recording keys.
 * Keys 1-9 are used for recording video loops.
 * Escape key is used for application quit.
 */
typedef enum {
    KEY_NUM_1 = 1,  // Key '1' - Maps to cell 2
    KEY_NUM_2 = 2,  // Key '2' - Maps to cell 3
    KEY_NUM_3 = 3,  // Key '3' - Maps to cell 4
    KEY_NUM_4 = 4,  // Key '4' - Maps to cell 5
    KEY_NUM_5 = 5,  // Key '5' - Maps to cell 6
    KEY_NUM_6 = 6,  // Key '6' - Maps to cell 7
    KEY_NUM_7 = 7,  // Key '7' - Maps to cell 8
    KEY_NUM_8 = 8,  // Key '8' - Maps to cell 9
    KEY_NUM_9 = 9,  // Key '9' - Maps to cell 10
    KEY_QUIT = -1,  // Escape or Ctrl+C - Quit application
    KEY_UNKNOWN = 0 // Unknown key code
} LogicalKeyNumber;

/**
 * @brief macOS/X11 key codes for physical keys
 *
 * Key code values depend on the windowing system:
 * - On macOS with Cocoa events: Native Cocoa key codes
 * - On X11/Wayland (if GTK+): GDK key codes
 *
 * This application currently targets macOS, so key codes are based on
 * the Cocoa NSEvent keyCode values.
 */
typedef enum {
    // Number keys (row above QWERTY)
    KEYCODE_1 = 18, // Physical key for '1'
    KEYCODE_2 = 19, // Physical key for '2'
    KEYCODE_3 = 20, // Physical key for '3'
    KEYCODE_4 = 21, // Physical key for '4'
    KEYCODE_5 = 23, // Physical key for '5'
    KEYCODE_6 = 22, // Physical key for '6'
    KEYCODE_7 = 26, // Physical key for '7'
    KEYCODE_8 = 28, // Physical key for '8'
    KEYCODE_9 = 25, // Physical key for '9'
    KEYCODE_0 = 29, // Physical key for '0'

    // Special keys
    KEYCODE_ESCAPE = 53, // ESC key - Application quit
    KEYCODE_C = 8,       // 'C' key (for Ctrl+C handling)

    // Unrecognized key
    KEYCODE_UNKNOWN = -1
} PhysicalKeyCode;

/**
 * @brief Map physical key code to logical key number
 *
 * Translates a physical key code (from the operating system or windowing
 * system) to a logical key number used by the application.
 *
 * Mapping rules:
 * - Physical keys 1-9 map to logical key numbers 1-9
 * - Escape key maps to KEY_QUIT (-1)
 * - All other keys map to KEY_UNKNOWN (0)
 *
 * @param physical_key_code The physical key code from the OS/windowing system
 * @return Logical key number (1-9 for recording, -1 for quit, 0 for unknown)
 *
 * @note This function is called from the keyboard event handler for each
 *       key press or release event.
 */
static inline LogicalKeyNumber key_code_to_logical_key(int physical_key_code)
{
    switch (physical_key_code) {
    case KEYCODE_1:
        return KEY_NUM_1;
    case KEYCODE_2:
        return KEY_NUM_2;
    case KEYCODE_3:
        return KEY_NUM_3;
    case KEYCODE_4:
        return KEY_NUM_4;
    case KEYCODE_5:
        return KEY_NUM_5;
    case KEYCODE_6:
        return KEY_NUM_6;
    case KEYCODE_7:
        return KEY_NUM_7;
    case KEYCODE_8:
        return KEY_NUM_8;
    case KEYCODE_9:
        return KEY_NUM_9;
    case KEYCODE_ESCAPE:
        return KEY_QUIT;
    default:
        return KEY_UNKNOWN;
    }
}

/**
 * @brief Check if a logical key number is a valid recording key
 *
 * Recording keys are logical keys 1-9. Returns false for quit key
 * and unknown keys.
 *
 * @param key_number Logical key number to check
 * @return TRUE if key is 1-9 (valid recording key), FALSE otherwise
 */
static inline gboolean key_is_recording_key(LogicalKeyNumber key_number)
{
    return key_number >= KEY_NUM_1 && key_number <= KEY_NUM_9;
}

/**
 * @brief Check if a logical key number is the quit key
 *
 * The quit key (Escape) terminates the application.
 *
 * @param key_number Logical key number to check
 * @return TRUE if key is quit key, FALSE otherwise
 */
static inline gboolean key_is_quit_key(LogicalKeyNumber key_number)
{
    return key_number == KEY_QUIT;
}

#endif // KEY_CODES_H
