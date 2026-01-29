/*
 * test_recording_state.c - Unit tests for recording state machine
 *
 * Tests for keyboard input handling, recording state transitions, and cell assignment logic.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========== Type Definitions ========== */
typedef int gboolean;
typedef unsigned long long guint64;
typedef int gint;
#define TRUE 1
#define FALSE 0

/* ========== Mock Utilities ========== */

/* Mock the timing functions for testing */
static uint64_t mock_time = 1000000000000ULL;

uint64_t timing_get_time_us(void)
{
    return mock_time;
}

uint64_t timing_elapsed_us(uint64_t start_time, uint64_t end_time)
{
    return end_time > start_time ? end_time - start_time : 0;
}

double timing_us_to_ms(uint64_t microseconds)
{
    return (double) microseconds / 1000.0;
}

double timing_us_to_sec(uint64_t microseconds)
{
    return (double) microseconds / 1000000.0;
}

uint64_t timing_ms_to_us(double milliseconds)
{
    return (uint64_t) (milliseconds * 1000);
}

uint64_t timing_sec_to_us(double seconds)
{
    return (uint64_t) (seconds * 1000000);
}

char *timing_get_timestamp_string(char *buffer, size_t size)
{
    snprintf(buffer, size, "2026-01-27 10:00:00");
    return buffer;
}

double timing_measure_fps(uint64_t frame_timestamp)
{
    (void) frame_timestamp;
    return 0.0;
}

void timing_reset_fps_measurement(void) {}

/* Mock logging functions */
void logging_log(int level, const char *category, const char *format, ...)
{
    (void) level;
    (void) category;
    (void) format;
}

void logging_set_level(int level)
{
    (void) level;
}

/* Mock glib functions */
#define g_malloc0(n) calloc(1, (n))
#define g_free(p) free((p))

typedef struct _GstElement GstElement;

/* Mock for LogLevel type from logging.h */
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR = 3,
} LogLevel;

/* Include the actual implementation */
#include "../../src/recording/recording_state.c"

/* Test helper function */
static void reset_mock_time(void)
{
    mock_time = 1000000000000ULL;
}

static void advance_mock_time_us(uint64_t delta)
{
    mock_time += delta;
}

/* ========== Test Cases ========== */

static int test_state_initialization(void)
{
    printf("TEST: state_initialization... ");
    reset_mock_time();

    RecordingState *state = recording_state_init();
    assert(state != NULL);

    /* All keys should be non-recording */
    for (int i = 1; i <= 9; i++) {
        assert(recording_is_recording(state, i) == FALSE);
        assert(recording_get_duration(state, i) == 0);
    }

    /* Cell index should start at 0 */
    assert(state->current_cell_index == 0);

    recording_state_cleanup(state);
    printf("PASS\n");
    return 0;
}

static int test_single_key_press_release(void)
{
    printf("TEST: single_key_press_release... ");
    reset_mock_time();

    RecordingState *state = recording_state_init();
    assert(state != NULL);

    /* Press key 1 */
    recording_on_key_press(state, 1);
    assert(recording_is_recording(state, 1) == TRUE);
    assert(state->record_start_time[0] == 1000000000000ULL);

    /* Advance time by 500ms */
    advance_mock_time_us(500000);

    /* Release key 1 */
    recording_on_key_release(state, 1);
    assert(recording_is_recording(state, 1) == FALSE);
    assert(recording_get_duration(state, 1) == 500000);

    recording_state_cleanup(state);
    printf("PASS\n");
    return 0;
}

static int test_multiple_simultaneous_keys(void)
{
    printf("TEST: multiple_simultaneous_keys... ");
    reset_mock_time();

    RecordingState *state = recording_state_init();
    assert(state != NULL);

    /* Press keys 1, 3, 5 simultaneously */
    recording_on_key_press(state, 1);
    recording_on_key_press(state, 3);
    recording_on_key_press(state, 5);

    assert(recording_is_recording(state, 1) == TRUE);
    assert(recording_is_recording(state, 3) == TRUE);
    assert(recording_is_recording(state, 5) == TRUE);
    assert(recording_is_recording(state, 2) == FALSE);
    assert(recording_is_recording(state, 4) == FALSE);

    /* Advance time by 1 second */
    advance_mock_time_us(1000000);

    /* Release each key in sequence */
    recording_on_key_release(state, 3);
    assert(recording_is_recording(state, 3) == FALSE);
    assert(recording_get_duration(state, 3) == 1000000);

    advance_mock_time_us(500000);

    recording_on_key_release(state, 1);
    assert(recording_is_recording(state, 1) == FALSE);
    assert(recording_get_duration(state, 1) == 1500000);

    assert(recording_is_recording(state, 5) == TRUE);

    recording_state_cleanup(state);
    printf("PASS\n");
    return 0;
}

static int test_minimum_frame_duration(void)
{
    printf("TEST: minimum_frame_duration... ");
    reset_mock_time();

    RecordingState *state = recording_state_init();
    assert(state != NULL);

    /* Press and release key immediately (very short press) */
    recording_on_key_press(state, 2);

    /* Only advance by 5ms (less than 1 frame at 30fps = 33.333ms) */
    advance_mock_time_us(5000);

    recording_on_key_release(state, 2);

    /* Duration should be rounded up to minimum frame duration (33333 us) */
    guint64 duration = recording_get_duration(state, 2);
    assert(duration == 33333);

    recording_state_cleanup(state);
    printf("PASS\n");
    return 0;
}

static int test_circular_cell_assignment(void)
{
    printf("TEST: circular_cell_assignment... ");
    reset_mock_time();

    RecordingState *state = recording_state_init();
    assert(state != NULL);

    /* Cell assignment should start at 0 and wrap around */
    assert(recording_assign_next_cell(state) == 0);
    assert(state->current_cell_index == 1);

    assert(recording_assign_next_cell(state) == 1);
    assert(state->current_cell_index == 2);

    /* Advance through all 9 cells */
    for (int i = 2; i < 8; i++) {
        assert(recording_assign_next_cell(state) == i);
        assert(state->current_cell_index == i + 1);
    }

    /* After cell 8 (index 8), should wrap to 0 */
    assert(recording_assign_next_cell(state) == 8);
    assert(state->current_cell_index == 0);

    /* Verify wraparound continues */
    assert(recording_assign_next_cell(state) == 0);
    assert(state->current_cell_index == 1);

    recording_state_cleanup(state);
    printf("PASS\n");
    return 0;
}

static int test_invalid_key_numbers(void)
{
    printf("TEST: invalid_key_numbers... ");
    reset_mock_time();

    RecordingState *state = recording_state_init();
    assert(state != NULL);

    /* Try pressing invalid keys (0, 10, 100) */
    recording_on_key_press(state, 0);
    recording_on_key_press(state, 10);
    recording_on_key_press(state, 100);
    recording_on_key_press(state, -1);

    /* No keys should be recording */
    for (int i = 1; i <= 9; i++) {
        assert(recording_is_recording(state, i) == FALSE);
    }

    /* Query invalid keys should return FALSE / 0 */
    assert(recording_is_recording(state, 0) == FALSE);
    assert(recording_is_recording(state, 10) == FALSE);
    assert(recording_get_duration(state, 0) == 0);
    assert(recording_get_duration(state, 10) == 0);

    recording_state_cleanup(state);
    printf("PASS\n");
    return 0;
}

static int test_double_press_same_key(void)
{
    printf("TEST: double_press_same_key... ");
    reset_mock_time();

    RecordingState *state = recording_state_init();
    assert(state != NULL);

    /* Press key 5 */
    recording_on_key_press(state, 5);
    assert(recording_is_recording(state, 5) == TRUE);
    guint64 first_press_time = state->record_start_time[4];

    advance_mock_time_us(100000);

    /* Press key 5 again while already recording */
    recording_on_key_press(state, 5);

    /* Should still be recording with original start time */
    assert(recording_is_recording(state, 5) == TRUE);
    assert(state->record_start_time[4] == first_press_time);

    recording_state_cleanup(state);
    printf("PASS\n");
    return 0;
}

static int test_release_non_recording_key(void)
{
    printf("TEST: release_non_recording_key... ");
    reset_mock_time();

    RecordingState *state = recording_state_init();
    assert(state != NULL);

    /* Release key 3 without pressing it first */
    recording_on_key_release(state, 3);

    /* Should still be non-recording with 0 duration */
    assert(recording_is_recording(state, 3) == FALSE);
    assert(recording_get_duration(state, 3) == 0);

    recording_state_cleanup(state);
    printf("PASS\n");
    return 0;
}

static int test_null_state_handling(void)
{
    printf("TEST: null_state_handling... ");

    /* These should not crash */
    recording_on_key_press(NULL, 1);
    recording_on_key_release(NULL, 1);
    assert(recording_is_recording(NULL, 1) == FALSE);
    assert(recording_get_duration(NULL, 1) == 0);
    assert(recording_assign_next_cell(NULL) == -1);
    recording_start_capture(NULL, 0);
    recording_stop_capture(NULL, 0);
    recording_state_cleanup(NULL);

    printf("PASS\n");
    return 0;
}

static int test_long_recording_duration(void)
{
    printf("TEST: long_recording_duration... ");
    reset_mock_time();

    RecordingState *state = recording_state_init();
    assert(state != NULL);

    /* Press key 9 */
    recording_on_key_press(state, 9);

    /* Advance time by 30 seconds */
    advance_mock_time_us(30000000);

    recording_on_key_release(state, 9);

    guint64 duration = recording_get_duration(state, 9);
    assert(duration == 30000000);

    recording_state_cleanup(state);
    printf("PASS\n");
    return 0;
}

/* ========== Main Test Runner ========== */

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    int test_count = 0;
    int pass_count = 0;

#define RUN_TEST(test_func)                                                                        \
    do {                                                                                           \
        test_count++;                                                                              \
        if (test_func() == 0) {                                                                    \
            pass_count++;                                                                          \
        } else {                                                                                   \
            fprintf(stderr, "FAIL: %s\n", #test_func);                                             \
        }                                                                                          \
    } while (0)

    printf("\n========== Recording State Unit Tests ==========\n\n");

    RUN_TEST(test_state_initialization);
    RUN_TEST(test_single_key_press_release);
    RUN_TEST(test_multiple_simultaneous_keys);
    RUN_TEST(test_minimum_frame_duration);
    RUN_TEST(test_circular_cell_assignment);
    RUN_TEST(test_invalid_key_numbers);
    RUN_TEST(test_double_press_same_key);
    RUN_TEST(test_release_non_recording_key);
    RUN_TEST(test_null_state_handling);
    RUN_TEST(test_long_recording_duration);

    printf("\n========== Test Results ==========\n");
    printf("PASSED: %d/%d\n", pass_count, test_count);

    if (pass_count == test_count) {
        printf("\nALL TESTS PASSED ✓\n\n");
        return 0;
    } else {
        printf("\nSOME TESTS FAILED ✗\n\n");
        return 1;
    }
}
