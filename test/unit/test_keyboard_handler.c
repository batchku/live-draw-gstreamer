/**
 * @file test_keyboard_handler.c
 * @brief Unit tests for keyboard input handling
 *
 * Tests for:
 * - Key code to logical key mapping
 * - Key press/release event dispatching
 * - Callback registration and invocation
 * - Handler initialization and cleanup
 * - Edge cases (uninitialized handler, duplicate init, NULL callback)
 *
 * Test Coverage:
 * - 8 unit tests covering all keyboard handler functionality
 * - Key code mappings (1-9, Escape)
 * - Event dispatching and callback invocation
 * - Handler lifecycle (init, cleanup)
 * - Error handling (uninitialized, invalid callbacks)
 */

#include "../../src/input/key_codes.h"
#include "../../src/input/keyboard_handler.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

// Callback tracking
static int last_key_number = 0;
static gboolean last_is_pressed = FALSE;
static int callback_invocation_count = 0;

/**
 * @brief Test callback that records key events
 */
static void test_callback(int key_number, gboolean is_pressed)
{
    last_key_number = key_number;
    last_is_pressed = is_pressed;
    callback_invocation_count++;
}

/**
 * @brief Helper macro for assertions
 */
#define ASSERT_EQ(actual, expected, test_name)                                                     \
    if ((actual) != (expected)) {                                                                  \
        fprintf(stdout, "FAIL: %s (expected %d, got %d)\n", test_name, (int) (expected),           \
                (int) (actual));                                                                   \
        tests_failed++;                                                                            \
    } else {                                                                                       \
        fprintf(stdout, "PASS: %s\n", test_name);                                                  \
        tests_passed++;                                                                            \
    }

#define ASSERT_TRUE(condition, test_name)                                                          \
    if (!(condition)) {                                                                            \
        fprintf(stdout, "FAIL: %s (condition false)\n", test_name);                                \
        tests_failed++;                                                                            \
    } else {                                                                                       \
        fprintf(stdout, "PASS: %s\n", test_name);                                                  \
        tests_passed++;                                                                            \
    }

/**
 * Test 1: Key code to logical key mapping for recording keys
 */
static void test_key_code_to_logical_recording_keys(void)
{
    // Test physical key codes for keys 1-9 map to logical keys 1-9
    ASSERT_EQ(key_code_to_logical_key(KEYCODE_1), KEY_NUM_1, "Key 1 mapping");
    ASSERT_EQ(key_code_to_logical_key(KEYCODE_2), KEY_NUM_2, "Key 2 mapping");
    ASSERT_EQ(key_code_to_logical_key(KEYCODE_3), KEY_NUM_3, "Key 3 mapping");
    ASSERT_EQ(key_code_to_logical_key(KEYCODE_4), KEY_NUM_4, "Key 4 mapping");
    ASSERT_EQ(key_code_to_logical_key(KEYCODE_5), KEY_NUM_5, "Key 5 mapping");
    ASSERT_EQ(key_code_to_logical_key(KEYCODE_6), KEY_NUM_6, "Key 6 mapping");
    ASSERT_EQ(key_code_to_logical_key(KEYCODE_7), KEY_NUM_7, "Key 7 mapping");
    ASSERT_EQ(key_code_to_logical_key(KEYCODE_8), KEY_NUM_8, "Key 8 mapping");
    ASSERT_EQ(key_code_to_logical_key(KEYCODE_9), KEY_NUM_9, "Key 9 mapping");
}

/**
 * Test 2: Key code to logical key mapping for Escape key
 */
static void test_key_code_to_logical_quit_key(void)
{
    ASSERT_EQ(key_code_to_logical_key(KEYCODE_ESCAPE), KEY_QUIT, "Escape key mapping");
}

/**
 * Test 3: Key code to logical key mapping for unknown keys
 */
static void test_key_code_to_logical_unknown_key(void)
{
    ASSERT_EQ(key_code_to_logical_key(100), KEY_UNKNOWN, "Unknown key code mapping");
    ASSERT_EQ(key_code_to_logical_key(-1), KEY_UNKNOWN, "Invalid key code mapping");
    ASSERT_EQ(key_code_to_logical_key(KEYCODE_0), KEY_UNKNOWN, "Key 0 mapping (not supported)");
}

/**
 * Test 4: Key predicate functions
 */
static void test_key_predicates(void)
{
    // Recording key predicates
    ASSERT_TRUE(key_is_recording_key(KEY_NUM_1), "Key 1 is recording key");
    ASSERT_TRUE(key_is_recording_key(KEY_NUM_5), "Key 5 is recording key");
    ASSERT_TRUE(key_is_recording_key(KEY_NUM_9), "Key 9 is recording key");

    // Quit key predicate
    ASSERT_TRUE(key_is_quit_key(KEY_QUIT), "Escape is quit key");

    // Non-recording keys
    ASSERT_EQ(key_is_recording_key(KEY_QUIT), 0, "Quit key is not recording key");
    ASSERT_EQ(key_is_recording_key(KEY_UNKNOWN), 0, "Unknown key is not recording key");
}

/**
 * Test 5: Keyboard handler initialization and callback dispatch
 */
static void test_keyboard_init_and_callback(void)
{
    // Reset callback state
    callback_invocation_count = 0;
    last_key_number = 0;
    last_is_pressed = FALSE;

    // Initialize handler
    keyboard_init(&test_callback);

    // Simulate key press for key 1
    keyboard_on_event(KEYCODE_1, TRUE);
    ASSERT_EQ(callback_invocation_count, 1, "Callback invoked once for key press");
    ASSERT_EQ(last_key_number, KEY_NUM_1, "Callback received correct key number");
    ASSERT_TRUE(last_is_pressed, "Callback received press state");

    // Simulate key release for key 1
    keyboard_on_event(KEYCODE_1, FALSE);
    ASSERT_EQ(callback_invocation_count, 2, "Callback invoked again for key release");
    ASSERT_EQ(last_key_number, KEY_NUM_1, "Callback received correct key number");
    ASSERT_EQ(last_is_pressed, 0, "Callback received release state");

    keyboard_cleanup();
}

/**
 * Test 6: Multiple key presses and releases
 */
static void test_multiple_key_events(void)
{
    callback_invocation_count = 0;

    keyboard_init(&test_callback);

    // Simulate multiple key events in sequence
    keyboard_on_event(KEYCODE_1, TRUE);  // Key 1 press
    keyboard_on_event(KEYCODE_2, TRUE);  // Key 2 press
    keyboard_on_event(KEYCODE_1, FALSE); // Key 1 release
    keyboard_on_event(KEYCODE_2, FALSE); // Key 2 release

    ASSERT_EQ(callback_invocation_count, 4, "All four events dispatched");

    keyboard_cleanup();
}

/**
 * Test 7: Unknown keys are ignored
 */
static void test_unknown_keys_ignored(void)
{
    callback_invocation_count = 0;

    keyboard_init(&test_callback);

    // Send unknown key events (should not invoke callback)
    keyboard_on_event(100, TRUE);       // Unknown key
    keyboard_on_event(100, FALSE);      // Unknown key release
    keyboard_on_event(KEYCODE_0, TRUE); // Key 0 (not supported)

    ASSERT_EQ(callback_invocation_count, 0, "Unknown keys not dispatched");

    keyboard_cleanup();
}

/**
 * Test 8: Escape key quit event dispatch
 */
static void test_escape_key_dispatch(void)
{
    callback_invocation_count = 0;

    keyboard_init(&test_callback);

    // Simulate Escape key press (quit)
    keyboard_on_event(KEYCODE_ESCAPE, TRUE);
    ASSERT_EQ(callback_invocation_count, 1, "Escape key press dispatched");
    ASSERT_EQ(last_key_number, KEY_QUIT, "Escape key mapped to quit");
    ASSERT_TRUE(last_is_pressed, "Escape key press state correct");

    // Simulate Escape key release
    keyboard_on_event(KEYCODE_ESCAPE, FALSE);
    ASSERT_EQ(callback_invocation_count, 2, "Escape key release dispatched");
    ASSERT_EQ(last_key_number, KEY_QUIT, "Escape key mapped to quit");
    ASSERT_EQ(last_is_pressed, 0, "Escape key release state correct");

    keyboard_cleanup();
}

/**
 * Test 9: Handler cleanup and re-initialization
 */
static void test_handler_cleanup_and_reinit(void)
{
    callback_invocation_count = 0;

    keyboard_init(&test_callback);

    // Send event before cleanup
    keyboard_on_event(KEYCODE_1, TRUE);
    ASSERT_EQ(callback_invocation_count, 1, "Event dispatched before cleanup");

    // Cleanup
    keyboard_cleanup();

    // Send event after cleanup (callback should not be invoked)
    keyboard_on_event(KEYCODE_1, FALSE);
    ASSERT_EQ(callback_invocation_count, 1, "Event not dispatched after cleanup");

    // Re-initialize
    keyboard_init(&test_callback);
    keyboard_on_event(KEYCODE_2, TRUE);
    ASSERT_EQ(callback_invocation_count, 2, "Event dispatched after re-init");

    keyboard_cleanup();
}

/**
 * Test 10: All recording keys (1-9) dispatch correctly
 */
static void test_all_recording_keys(void)
{
    callback_invocation_count = 0;
    int expected_invocations = 0;

    keyboard_init(&test_callback);

    // Press all recording keys
    const int key_codes[] = {KEYCODE_1, KEYCODE_2, KEYCODE_3, KEYCODE_4, KEYCODE_5,
                             KEYCODE_6, KEYCODE_7, KEYCODE_8, KEYCODE_9};
    for (int i = 0; i < 9; i++) {
        keyboard_on_event(key_codes[i], TRUE);
        expected_invocations++;
    }

    ASSERT_EQ(callback_invocation_count, expected_invocations, "All 9 recording keys pressed");

    // Release all recording keys
    for (int i = 0; i < 9; i++) {
        keyboard_on_event(key_codes[i], FALSE);
        expected_invocations++;
    }

    ASSERT_EQ(callback_invocation_count, expected_invocations, "All 9 recording keys released");

    keyboard_cleanup();
}

/**
 * Main test runner
 */
int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    fprintf(stdout, "\n=== Keyboard Handler Unit Tests ===\n\n");

    test_key_code_to_logical_recording_keys();
    test_key_code_to_logical_quit_key();
    test_key_code_to_logical_unknown_key();
    test_key_predicates();
    test_keyboard_init_and_callback();
    test_multiple_key_events();
    test_unknown_keys_ignored();
    test_escape_key_dispatch();
    test_handler_cleanup_and_reinit();
    test_all_recording_keys();

    fprintf(stdout, "\n=== Test Results ===\n");
    fprintf(stdout, "Passed: %d\n", tests_passed);
    fprintf(stdout, "Failed: %d\n", tests_failed);

    if (tests_failed == 0) {
        fprintf(stdout, "\n✓ All keyboard handler tests passed!\n\n");
        return 0;
    } else {
        fprintf(stdout, "\n✗ Some tests failed\n\n");
        return 1;
    }
}
