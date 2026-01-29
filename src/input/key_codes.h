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
 * Keys 1-10 are used for recording video loops.
 * Shift+1-10 map to layers 11-20.
 * Escape key is used for application quit.
 */
typedef enum {
    KEY_NUM_1 = 1,   // Key '1' - Layer 1
    KEY_NUM_2 = 2,   // Key '2' - Layer 2
    KEY_NUM_3 = 3,   // Key '3' - Layer 3
    KEY_NUM_4 = 4,   // Key '4' - Layer 4
    KEY_NUM_5 = 5,   // Key '5' - Layer 5
    KEY_NUM_6 = 6,   // Key '6' - Layer 6
    KEY_NUM_7 = 7,   // Key '7' - Layer 7
    KEY_NUM_8 = 8,   // Key '8' - Layer 8
    KEY_NUM_9 = 9,   // Key '9' - Layer 9
    KEY_NUM_10 = 10, // Key '0' - Layer 10
    KEY_NUM_11 = 11, // Shift+1 - Layer 11
    KEY_NUM_12 = 12, // Shift+2 - Layer 12
    KEY_NUM_13 = 13, // Shift+3 - Layer 13
    KEY_NUM_14 = 14, // Shift+4 - Layer 14
    KEY_NUM_15 = 15, // Shift+5 - Layer 15
    KEY_NUM_16 = 16, // Shift+6 - Layer 16
    KEY_NUM_17 = 17, // Shift+7 - Layer 17
    KEY_NUM_18 = 18, // Shift+8 - Layer 18
    KEY_NUM_19 = 19, // Shift+9 - Layer 19
    KEY_NUM_20 = 20, // Shift+0 - Layer 20
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
 * - Physical key 0 maps to logical key number 10
 * - Shift+1-9 map to logical key numbers 11-19
 * - Shift+0 maps to logical key number 20
 * - Escape key maps to KEY_QUIT (-1)
 * - All other keys map to KEY_UNKNOWN (0)
 *
 * @param physical_key_code The physical key code from the OS/windowing system
 * @param is_shifted TRUE if shift modifier is active
 * @return Logical key number (1-20 for recording, -1 for quit, 0 for unknown)
 *
 * @note This function is called from the keyboard event handler for each
 *       key press or release event.
 */
static inline LogicalKeyNumber key_code_to_logical_key(int physical_key_code, gboolean is_shifted)
{
    switch (physical_key_code) {
    case KEYCODE_1:
        return is_shifted ? KEY_NUM_11 : KEY_NUM_1;
    case KEYCODE_2:
        return is_shifted ? KEY_NUM_12 : KEY_NUM_2;
    case KEYCODE_3:
        return is_shifted ? KEY_NUM_13 : KEY_NUM_3;
    case KEYCODE_4:
        return is_shifted ? KEY_NUM_14 : KEY_NUM_4;
    case KEYCODE_5:
        return is_shifted ? KEY_NUM_15 : KEY_NUM_5;
    case KEYCODE_6:
        return is_shifted ? KEY_NUM_16 : KEY_NUM_6;
    case KEYCODE_7:
        return is_shifted ? KEY_NUM_17 : KEY_NUM_7;
    case KEYCODE_8:
        return is_shifted ? KEY_NUM_18 : KEY_NUM_8;
    case KEYCODE_9:
        return is_shifted ? KEY_NUM_19 : KEY_NUM_9;
    case KEYCODE_0:
        return is_shifted ? KEY_NUM_20 : KEY_NUM_10;
    case KEYCODE_ESCAPE:
        return KEY_QUIT;
    default:
        return KEY_UNKNOWN;
    }
}

/**
 * @brief Check if a logical key number is a valid recording key
 *
 * Recording keys are logical keys 1-20. Returns false for quit key
 * and unknown keys.
 *
 * @param key_number Logical key number to check
 * @return TRUE if key is 1-20 (valid recording key), FALSE otherwise
 */
static inline gboolean key_is_recording_key(LogicalKeyNumber key_number)
{
    return key_number >= KEY_NUM_1 && key_number <= KEY_NUM_20;
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
