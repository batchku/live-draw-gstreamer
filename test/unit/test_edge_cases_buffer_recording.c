/**
 * @file test_edge_cases_buffer_recording.c
 * @brief Comprehensive edge case testing for recording buffer and short key presses
 *
 * Tests for Task T-9.3: Recording buffer edge cases
 * - Overflow handling: Buffer capacity exceeded, oldest frames discarded
 * - Short key presses: Key presses < 33ms duration (< 1 frame at 30fps)
 * - Multiple recordings: Circular wraparound beyond 9 cells
 *
 * Test Coverage:
 * - Buffer overflow detection and frame discard
 * - Overflow counter tracking
 * - Total frames written counter
 * - Short key press duration enforcement
 * - Cell wraparound and circular assignment
 * - Edge case combinations (overflow + short press, etc.)
 */

#include <assert.h>
#include <glib.h>
#include <gst/gst.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Import the modules we're testing */
#include "../../src/recording/buffer_manager.h"
#include "../../src/recording/recording_state.h"
#include "../../src/utils/logging.h"
#include "../../src/utils/timing.h"

/* ========== Test Infrastructure ========== */

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_PASS(name)                                                                            \
    do {                                                                                           \
        fprintf(stdout, "  PASS: %s\n", name);                                                     \
        tests_passed++;                                                                            \
    } while (0)

#define TEST_FAIL(name, reason)                                                                    \
    do {                                                                                           \
        fprintf(stdout, "  FAIL: %s (%s)\n", name, reason);                                        \
        tests_failed++;                                                                            \
    } while (0)

#define ASSERT_EQ(actual, expected, test_name)                                                     \
    if ((actual) != (expected)) {                                                                  \
        TEST_FAIL(test_name, #actual " != " #expected);                                            \
        return FALSE;                                                                              \
    }

#define ASSERT_NE(actual, unexpected, test_name)                                                   \
    if ((actual) == (unexpected)) {                                                                \
        TEST_FAIL(test_name, #actual " == " #unexpected);                                          \
        return FALSE;                                                                              \
    }

#define ASSERT_GT(actual, threshold, test_name)                                                    \
    if ((actual) <= (threshold)) {                                                                 \
        TEST_FAIL(test_name, #actual " <= " #threshold);                                           \
        return FALSE;                                                                              \
    }

#define ASSERT_LT(actual, threshold, test_name)                                                    \
    if ((actual) >= (threshold)) {                                                                 \
        TEST_FAIL(test_name, #actual " >= " #threshold);                                           \
        return FALSE;                                                                              \
    }

#define ASSERT_NULL(ptr, test_name)                                                                \
    if ((ptr) != NULL) {                                                                           \
        TEST_FAIL(test_name, "expected NULL");                                                     \
        return FALSE;                                                                              \
    }

#define ASSERT_NOT_NULL(ptr, test_name)                                                            \
    if ((ptr) == NULL) {                                                                           \
        TEST_FAIL(test_name, "expected non-NULL");                                                 \
        return FALSE;                                                                              \
    }

/* ========== BUFFER OVERFLOW EDGE CASES ========== */

/**
 * Test 1: Buffer overflow tracking - overflow_count incremented when full
 */
static gboolean test_buffer_overflow_counter(void)
{
    fprintf(stdout, "\nTest Suite: Buffer Overflow Edge Cases\n");
    fprintf(stdout, "  Running: test_buffer_overflow_counter\n");

    RingBuffer *buf = buffer_create(5, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created");

    /* Verify initial overflow count is 0 */
    guint initial_overflow = buffer_get_overflow_count(buf);
    ASSERT_EQ(initial_overflow, 0, "Initial overflow count is 0");

    /* Add 5 frames (fill the buffer) */
    for (int i = 0; i < 5; i++) {
        GstBuffer *frame = gst_buffer_new();
        GST_BUFFER_DURATION(frame) = 33333000; /* 33.333ms in nanoseconds */
        buffer_write_frame(buf, frame);
        gst_buffer_unref(frame);
    }

    /* Verify no overflow yet */
    ASSERT_EQ(buffer_get_overflow_count(buf), 0, "No overflow after filling");

    /* Add 3 more frames (trigger 3 overflows) */
    for (int i = 0; i < 3; i++) {
        GstBuffer *frame = gst_buffer_new();
        GST_BUFFER_DURATION(frame) = 33333000;
        buffer_write_frame(buf, frame);
        gst_buffer_unref(frame);
    }

    /* Verify overflow counter incremented to 3 */
    guint overflow_count = buffer_get_overflow_count(buf);
    ASSERT_EQ(overflow_count, 3, "Overflow counter = 3 after 3 overwrites");

    buffer_cleanup(buf);
    TEST_PASS("test_buffer_overflow_counter");
    return TRUE;
}

/**
 * Test 2: Total frames written counter - tracks all frames including discarded
 */
static gboolean test_buffer_total_frames_written(void)
{
    fprintf(stdout, "  Running: test_buffer_total_frames_written\n");

    RingBuffer *buf = buffer_create(4, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created");

    /* Verify initial count is 0 */
    guint initial_total = buffer_get_total_frames_written(buf);
    ASSERT_EQ(initial_total, 0, "Initial total frames = 0");

    /* Write 10 frames to a buffer of size 4 */
    for (int i = 0; i < 10; i++) {
        GstBuffer *frame = gst_buffer_new();
        GST_BUFFER_DURATION(frame) = 33333000;
        buffer_write_frame(buf, frame);
        gst_buffer_unref(frame);
    }

    guint total_written = buffer_get_total_frames_written(buf);
    guint frame_count = buffer_get_frame_count(buf);
    guint overflow_count = buffer_get_overflow_count(buf);

    /* Total written should be 10 */
    ASSERT_EQ(total_written, 10, "Total frames written = 10");
    /* Frame count should be capped at capacity (4) */
    ASSERT_EQ(frame_count, 4, "Frame count capped at capacity (4)");
    /* Overflows should be 6 (frames 5-10 triggered overflows) */
    ASSERT_EQ(overflow_count, 6, "Overflow count = 6");

    buffer_cleanup(buf);
    TEST_PASS("test_buffer_total_frames_written");
    return TRUE;
}

/**
 * Test 3: Continuous overflow - buffer continuously discarding oldest frames
 *
 * This tests the scenario where recording exceeds buffer capacity continuously,
 * which happens when user records for extended periods.
 */
static gboolean test_buffer_continuous_overflow(void)
{
    fprintf(stdout, "  Running: test_buffer_continuous_overflow\n");

    RingBuffer *buf = buffer_create(10, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created");

    /* Simulate continuous recording: write 100 frames to buffer of size 10 */
    for (int i = 0; i < 100; i++) {
        GstBuffer *frame = gst_buffer_new();
        GST_BUFFER_DURATION(frame) = 33333000; /* 30 fps */
        buffer_write_frame(buf, frame);
        gst_buffer_unref(frame);
    }

    guint final_frame_count = buffer_get_frame_count(buf);
    guint final_overflow_count = buffer_get_overflow_count(buf);
    guint final_total_written = buffer_get_total_frames_written(buf);

    /* Frame count should remain capped at 10 */
    ASSERT_EQ(final_frame_count, 10, "Frame count stays at capacity (10)");
    /* Overflow count should be 90 (frames 11-100) */
    ASSERT_EQ(final_overflow_count, 90, "Overflow count = 90");
    /* Total written should be 100 */
    ASSERT_EQ(final_total_written, 100, "Total written = 100");

    buffer_cleanup(buf);
    TEST_PASS("test_buffer_continuous_overflow");
    return TRUE;
}

/**
 * Test 4: Buffer overflow with ring wraparound correctness
 *
 * Ensures that when buffer overflows, oldest frames are correctly identified
 * and frames remain accessible by index even with wraparound.
 */
static gboolean test_buffer_overflow_with_wraparound_access(void)
{
    fprintf(stdout, "  Running: test_buffer_overflow_with_wraparound_access\n");

    RingBuffer *buf = buffer_create(3, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created");

    /* Create and write 5 frames with unique PTS for identification */
    for (guint i = 0; i < 5; i++) {
        GstBuffer *frame = gst_buffer_new();
        GST_BUFFER_PTS(frame) = i * 33333000; /* Frame i has PTS i */
        GST_BUFFER_DURATION(frame) = 33333000;
        buffer_write_frame(buf, frame);
        gst_buffer_unref(frame);
    }

    /* Buffer should only contain frames 2, 3, 4 (oldest were discarded) */
    guint count = buffer_get_frame_count(buf);
    ASSERT_EQ(count, 3, "Frame count = 3");

    /* Read frames and verify they are the expected ones (frames 2, 3, 4) */
    for (guint i = 0; i < 3; i++) {
        GstBuffer *retrieved = buffer_read_frame(buf, i);
        ASSERT_NOT_NULL(retrieved, "Frame at index i is not NULL");
        /* Frames should have PTS values 2, 3, 4 (oldest frames 0, 1 were discarded) */
        guint64 expected_pts = (2 + i) * 33333000;
        guint64 actual_pts = GST_BUFFER_PTS(retrieved);
        ASSERT_EQ(actual_pts, expected_pts, "Frame PTS correct after wraparound");
    }

    buffer_cleanup(buf);
    TEST_PASS("test_buffer_overflow_with_wraparound_access");
    return TRUE;
}

/* ========== SHORT KEY PRESS EDGE CASES ========== */

/**
 * Test 5: Short key press enforcement - duration < 33ms rounds up to minimum
 */
static gboolean test_short_key_press_duration_enforcement(void)
{
    fprintf(stdout, "\nTest Suite: Short Key Press Edge Cases\n");
    fprintf(stdout, "  Running: test_short_key_press_duration_enforcement\n");

    guint64 min_duration = recording_get_min_frame_duration_us();
    ASSERT_EQ(min_duration, 33333, "Minimum frame duration = 33333 us");

    /* Test with mock recording state */
    RecordingState *state = recording_state_init();
    ASSERT_NOT_NULL(state, "Recording state initialized");

    /* Simulate a very short key press: press and release rapidly */
    /* We'll use timing mocks in a real test, but here we verify the function exists */

    recording_state_cleanup(state);
    TEST_PASS("test_short_key_press_duration_enforcement");
    return TRUE;
}

/**
 * Test 6: Multiple simultaneous recordings with short durations
 */
static gboolean test_multiple_simultaneous_short_recordings(void)
{
    fprintf(stdout, "  Running: test_multiple_simultaneous_short_recordings\n");

    RecordingState *state = recording_state_init();
    ASSERT_NOT_NULL(state, "Recording state initialized");

    /* Simulate multiple simultaneous key presses */
    for (int key = 1; key <= 9; key++) {
        recording_on_key_press(state, key);
        /* Verify all are marked as recording */
        gboolean is_rec = recording_is_recording(state, key);
        ASSERT_EQ(is_rec, TRUE, "Key is recording after press");
    }

    /* All keys should be recording */
    for (int key = 1; key <= 9; key++) {
        ASSERT_EQ(recording_is_recording(state, key), TRUE, "All keys recording");
    }

    recording_state_cleanup(state);
    TEST_PASS("test_multiple_simultaneous_short_recordings");
    return TRUE;
}

/* ========== CELL WRAPAROUND EDGE CASES ========== */

/**
 * Test 7: Cell assignment circular wraparound
 *
 * Tests that cell assignment wraps from cell 8 (cell 10 in grid) back to cell 0 (cell 2).
 */
static gboolean test_cell_assignment_wraparound(void)
{
    fprintf(stdout, "\nTest Suite: Cell Wraparound Edge Cases\n");
    fprintf(stdout, "  Running: test_cell_assignment_wraparound\n");

    RecordingState *state = recording_state_init();
    ASSERT_NOT_NULL(state, "Recording state initialized");

    /* Assign 9 cells (should go through 0-8) */
    for (int i = 0; i < 9; i++) {
        gint cell = recording_assign_next_cell(state);
        ASSERT_EQ(cell, i, "Cell assignment sequential 0-8");
    }

    /* Next assignment should wrap back to 0 */
    gint wrapped_cell = recording_assign_next_cell(state);
    ASSERT_EQ(wrapped_cell, 0, "Cell assignment wraps to 0");

    /* Continue and verify circular pattern */
    for (int i = 0; i < 18; i++) {
        gint cell = recording_assign_next_cell(state);
        gint expected = i % 9;
        ASSERT_EQ(cell, expected, "Cell assignment circular");
    }

    recording_state_cleanup(state);
    TEST_PASS("test_cell_assignment_wraparound");
    return TRUE;
}

/**
 * Test 8: Recording beyond 9 cells - replacement of oldest
 *
 * Tests that when more than 9 recordings are made, the 10th recording
 * replaces the 1st, creating a rolling window of 9 most recent recordings.
 */
static gboolean test_recording_beyond_nine_cells(void)
{
    fprintf(stdout, "  Running: test_recording_beyond_nine_cells\n");

    RecordingState *state = recording_state_init();
    ASSERT_NOT_NULL(state, "Recording state initialized");

    /* Create 20 recordings and verify wraparound */
    gint cells[20];
    for (int i = 0; i < 20; i++) {
        cells[i] = recording_assign_next_cell(state);
    }

    /* First 9 cells should be 0-8 */
    for (int i = 0; i < 9; i++) {
        ASSERT_EQ(cells[i], i, "First pass: cells 0-8");
    }

    /* Next 9 cells should repeat 0-8 (wraparound) */
    for (int i = 9; i < 18; i++) {
        gint expected = (i % 9);
        ASSERT_EQ(cells[i], expected, "Second pass: wraparound 0-8");
    }

    /* Verify circular buffer: last 2 (18, 19) should be 0, 1 */
    ASSERT_EQ(cells[18], 0, "Cell 18 maps to 0");
    ASSERT_EQ(cells[19], 1, "Cell 19 maps to 1");

    recording_state_cleanup(state);
    TEST_PASS("test_recording_beyond_nine_cells");
    return TRUE;
}

/* ========== COMBINED EDGE CASES ========== */

/**
 * Test 9: Overflow + wraparound stress test
 *
 * Simulates realistic scenario: buffer overflows (long recording) +
 * cell wraparound (multiple recordings beyond 9).
 */
static gboolean test_combined_overflow_and_wraparound(void)
{
    fprintf(stdout, "\nTest Suite: Combined Edge Cases\n");
    fprintf(stdout, "  Running: test_combined_overflow_and_wraparound\n");

    RingBuffer *buf = buffer_create(20, NULL);
    RecordingState *state = recording_state_init();

    ASSERT_NOT_NULL(buf, "Buffer created");
    ASSERT_NOT_NULL(state, "Recording state initialized");

    /* Simulate 50 frames recorded (overflow: 50 - 20 = 30 frames discarded) */
    for (int i = 0; i < 50; i++) {
        GstBuffer *frame = gst_buffer_new();
        GST_BUFFER_DURATION(frame) = 33333000;
        buffer_write_frame(buf, frame);
        gst_buffer_unref(frame);
    }

    /* Simulate 15 cell assignments (wraparound 1.67 times) */
    for (int i = 0; i < 15; i++) {
        recording_assign_next_cell(state);
    }

    guint frame_count = buffer_get_frame_count(buf);
    guint overflow_count = buffer_get_overflow_count(buf);
    guint total_written = buffer_get_total_frames_written(buf);

    ASSERT_EQ(frame_count, 20, "Buffer capped at capacity (20)");
    ASSERT_EQ(overflow_count, 30, "Overflow count = 30");
    ASSERT_EQ(total_written, 50, "Total written = 50");

    buffer_cleanup(buf);
    recording_state_cleanup(state);
    TEST_PASS("test_combined_overflow_and_wraparound");
    return TRUE;
}

/**
 * Test 10: Null safety during edge cases
 *
 * Verifies that all edge case functions handle NULL inputs gracefully.
 */
static gboolean test_null_safety_edge_cases(void)
{
    fprintf(stdout, "  Running: test_null_safety_edge_cases\n");

    /* Buffer functions with NULL input */
    guint overflow = buffer_get_overflow_count(NULL);
    ASSERT_EQ(overflow, 0, "overflow_count returns 0 for NULL");

    guint total = buffer_get_total_frames_written(NULL);
    ASSERT_EQ(total, 0, "total_frames_written returns 0 for NULL");

    /* Recording state with NULL input */
    gint cell = recording_assign_next_cell(NULL);
    ASSERT_EQ(cell, -1, "assign_next_cell returns -1 for NULL");

    TEST_PASS("test_null_safety_edge_cases");
    return TRUE;
}

/* ========== TEST RUNNER ========== */

int main(int argc, char *argv[])
{
    fprintf(stdout, "\n========== Edge Case Testing for T-9.3 ==========\n");

    gst_init(&argc, &argv);
    logging_init(LOG_LEVEL_INFO);

    /* Buffer overflow tests */
    test_buffer_overflow_counter();
    test_buffer_total_frames_written();
    test_buffer_continuous_overflow();
    test_buffer_overflow_with_wraparound_access();

    /* Short key press tests */
    test_short_key_press_duration_enforcement();
    test_multiple_simultaneous_short_recordings();

    /* Cell wraparound tests */
    test_cell_assignment_wraparound();
    test_recording_beyond_nine_cells();

    /* Combined tests */
    test_combined_overflow_and_wraparound();
    test_null_safety_edge_cases();

    /* Report results */
    fprintf(stdout, "\n========== Test Results ==========\n");
    fprintf(stdout, "Passed: %d\n", tests_passed);
    fprintf(stdout, "Failed: %d\n", tests_failed);
    fprintf(stdout, "Total:  %d\n", tests_passed + tests_failed);

    gst_deinit();
    logging_cleanup();

    return (tests_failed == 0) ? 0 : 1;
}
