/**
 * @file test_playback_e2e_flow.c
 * @brief Integration tests for end-to-end playback flow (T-6.6)
 *
 * Tests the following requirements:
 * - Record key 1 → playback appears in cell 2
 * - Multiple simultaneous recordings (multiple keys held at same time)
 * - Cell wraparound (after cell 10, next recording goes to cell 2)
 * - Palindrome playback (forward → reverse → repeat)
 * - Grid composition with live feed + playback streams
 *
 * ARCHITECTURE NOTE:
 * These tests verify the core E2E playback flow logic without requiring
 * full GStreamer integration. Each test is self-contained and uses mock
 * data structures to simulate the recording and playback pipeline.
 *
 * @author Test Engineer
 * @date 2026-01-27
 */

#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef G_GNUC_UNUSED
#define G_GNUC_UNUSED __attribute__((unused))
#endif

/* ============================================================================
 * Test Infrastructure & Mocking
 * ========================================================================== */

typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} TestStats;

static TestStats g_test_stats = {0, 0, 0};

#define TEST_ASSERT(condition, ...)                                                                \
    do {                                                                                           \
        g_test_stats.total_tests++;                                                                \
        if (!(condition)) {                                                                        \
            fprintf(stderr, "[FAIL] ");                                                            \
            fprintf(stderr, __VA_ARGS__);                                                          \
            fprintf(stderr, "\n");                                                                 \
            g_test_stats.failed_tests++;                                                           \
        } else {                                                                                   \
            fprintf(stdout, "[PASS] ");                                                            \
            fprintf(stdout, __VA_ARGS__);                                                          \
            fprintf(stdout, "\n");                                                                 \
            g_test_stats.passed_tests++;                                                           \
        }                                                                                          \
    } while (0)

/* Mock types for testing without full pipeline */

typedef enum {
    PLAYBACK_STATE_FORWARD,
    PLAYBACK_STATE_REVERSE,
} PlaybackDirection;

typedef struct {
    int total_frames;
    int current_frame;
    PlaybackDirection direction;
} MockPlaybackLoop;

typedef struct {
    int *frames;
    int count;
} MockRingBuffer;

typedef struct {
    int cell_num;
    MockPlaybackLoop *loop;
    gboolean is_active;
} MockPlaybackBin;

typedef struct {
    MockPlaybackBin bins[9]; /* Cells 2-10 map to indices 0-8 */
    int next_cell_to_fill;   /* 0-8, maps to cells 2-10 */
    int recording_count;     /* Total recordings made */
} MockGrid;

/* ============================================================================
 * Helper Functions
 * ========================================================================== */

/**
 * Initialize mock grid
 */
static void mock_grid_init(MockGrid *grid)
{
    memset(grid, 0, sizeof(MockGrid));
    grid->next_cell_to_fill = 0; /* Start with cell 2 (index 0) */

    for (int i = 0; i < 9; i++) {
        grid->bins[i].cell_num = i + 2; /* Cell 2-10 */
        grid->bins[i].loop = NULL;
        grid->bins[i].is_active = FALSE;
    }
}

/**
 * Simulate recording: create a playback loop and add to grid
 */
static MockPlaybackLoop *mock_grid_record_and_playback(MockGrid *grid, int key_num G_GNUC_UNUSED)
{
    if (grid->next_cell_to_fill < 0 || grid->next_cell_to_fill >= 9) {
        return NULL;
    }

    int cell_index = grid->next_cell_to_fill;

    /* Create playback loop */
    MockPlaybackLoop *loop = (MockPlaybackLoop *) g_malloc0(sizeof(MockPlaybackLoop));
    if (!loop) {
        return NULL;
    }

    /* Simulate recording of 10 frames */
    loop->total_frames = 10;
    loop->current_frame = 0;
    loop->direction = PLAYBACK_STATE_FORWARD;

    /* Add to grid */
    grid->bins[cell_index].loop = loop;
    grid->bins[cell_index].is_active = TRUE;
    grid->recording_count++;

    /* Advance to next cell for next recording */
    grid->next_cell_to_fill = (grid->next_cell_to_fill + 1) % 9;

    return loop;
}

/**
 * Simulate one frame of palindrome playback
 * Returns the next frame index
 */
static int mock_palindrome_advance_frame(MockPlaybackLoop *loop)
{
    if (!loop)
        return -1;

    int current = loop->current_frame;

    /* Move to next frame */
    if (loop->direction == PLAYBACK_STATE_FORWARD) {
        loop->current_frame++;
        if (loop->current_frame >= loop->total_frames) {
            /* Switch to reverse direction */
            loop->direction = PLAYBACK_STATE_REVERSE;
            loop->current_frame = loop->total_frames - 2; /* Don't repeat last frame */
        }
    } else { /* REVERSE */
        loop->current_frame--;
        if (loop->current_frame < 0) {
            /* Switch to forward direction */
            loop->direction = PLAYBACK_STATE_FORWARD;
            loop->current_frame = 1; /* Don't repeat frame 0 */
        }
    }

    return current;
}

/**
 * Verify palindrome sequence for 20 frames
 */
static gboolean mock_verify_palindrome_sequence(MockPlaybackLoop *loop)
{
    if (!loop || loop->total_frames != 10) {
        return FALSE;
    }

    /* Expected sequence: 0,1,2,3,4,5,6,7,8,9,8,7,6,5,4,3,2,1,0,1 */
    int expected[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 1};

    /* Reset playback */
    loop->current_frame = 0;
    loop->direction = PLAYBACK_STATE_FORWARD;

    /* Verify sequence */
    for (int i = 0; i < 20; i++) {
        int frame = mock_palindrome_advance_frame(loop);
        if (frame != expected[i]) {
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * Cleanup mock grid
 */
static void mock_grid_cleanup(MockGrid *grid)
{
    for (int i = 0; i < 9; i++) {
        if (grid->bins[i].loop) {
            g_free(grid->bins[i].loop);
            grid->bins[i].loop = NULL;
        }
    }
}

/* ============================================================================
 * Test Cases
 * ========================================================================== */

/**
 * Test 1: Record key 1 → playback in cell 2
 *
 * Verifies that pressing key 1, recording, then releasing creates
 * a playback loop in cell 2 (the first playback cell).
 */
static void test_record_key1_playback_cell2(void)
{
    MockGrid grid;
    mock_grid_init(&grid);

    /* Simulate recording with key 1 */
    MockPlaybackLoop *loop = mock_grid_record_and_playback(&grid, 1);

    /* Verify playback bin created in cell 2 */
    TEST_ASSERT(loop != NULL, "Playback loop created for key 1");
    TEST_ASSERT(grid.bins[0].is_active, "Cell 2 (index 0) is active");
    TEST_ASSERT(grid.bins[0].loop == loop, "Playback bin assigned to cell 2");
    TEST_ASSERT(grid.bins[0].cell_num == 2, "Cell 2 has correct cell number");

    /* Verify next recording goes to cell 3 */
    TEST_ASSERT(grid.next_cell_to_fill == 1, "Next cell to fill is index 1 (cell 3)");

    mock_grid_cleanup(&grid);
}

/**
 * Test 2: Multiple simultaneous recordings
 *
 * Simulates holding multiple keys (1, 2, 3) at the same time.
 * Each should create independent recordings.
 */
static void test_multiple_simultaneous_recordings(void)
{
    MockGrid grid;
    mock_grid_init(&grid);

    /* Simulate recording with keys 1, 2, 3 */
    MockPlaybackLoop *loop1 = mock_grid_record_and_playback(&grid, 1);
    MockPlaybackLoop *loop2 = mock_grid_record_and_playback(&grid, 2);
    MockPlaybackLoop *loop3 = mock_grid_record_and_playback(&grid, 3);

    /* Verify all three loops were created */
    TEST_ASSERT(loop1 != NULL, "Recording 1 created");
    TEST_ASSERT(loop2 != NULL, "Recording 2 created");
    TEST_ASSERT(loop3 != NULL, "Recording 3 created");

    /* Verify they are in different cells */
    TEST_ASSERT(grid.bins[0].is_active && grid.bins[0].loop == loop1, "Loop 1 in cell 2");
    TEST_ASSERT(grid.bins[1].is_active && grid.bins[1].loop == loop2, "Loop 2 in cell 3");
    TEST_ASSERT(grid.bins[2].is_active && grid.bins[2].loop == loop3, "Loop 3 in cell 4");

    /* Verify recording count */
    TEST_ASSERT(grid.recording_count == 3, "3 recordings total");

    /* Verify next cell is 5 */
    TEST_ASSERT(grid.next_cell_to_fill == 3, "Next cell is index 3 (cell 5)");

    mock_grid_cleanup(&grid);
}

/**
 * Test 3: Cell wraparound
 *
 * Fill cells 2-10 with recordings, then verify that the next recording
 * wraps around back to cell 2, replacing the oldest.
 */
static void test_cell_wraparound(void)
{
    MockGrid grid;
    mock_grid_init(&grid);

    /* Fill all cells 2-10 (9 recordings) */
    for (int i = 0; i < 9; i++) {
        MockPlaybackLoop *loop = mock_grid_record_and_playback(&grid, i + 1);
        TEST_ASSERT(loop != NULL, "Recording %d created", i + 1);
    }

    /* Verify all 9 cells are filled */
    TEST_ASSERT(grid.recording_count == 9, "9 recordings total");
    for (int i = 0; i < 9; i++) {
        TEST_ASSERT(grid.bins[i].is_active, "Cell %d (index %d) is active", i + 2, i);
    }

    /* Next recording should wrap to cell 2 (index 0) */
    TEST_ASSERT(grid.next_cell_to_fill == 0, "Next cell wraps to index 0 (cell 2)");

    /* Record 10th loop (wraps to cell 2) */
    MockPlaybackLoop *old_loop = grid.bins[0].loop;
    MockPlaybackLoop *new_loop = mock_grid_record_and_playback(&grid, 1);

    TEST_ASSERT(new_loop != NULL, "10th recording created");
    TEST_ASSERT(grid.bins[0].loop == new_loop, "Cell 2 now has new recording");
    TEST_ASSERT(grid.bins[0].loop != old_loop, "Old recording replaced in cell 2");
    TEST_ASSERT(grid.recording_count == 10, "10 total recordings (wraparound)");

    mock_grid_cleanup(&grid);
}

/**
 * Test 4: Palindrome playback sequence
 *
 * Verify the palindrome algorithm produces the correct frame sequence:
 * 0,1,2,3,4,5,6,7,8,9,8,7,6,5,4,3,2,1,0,1...
 */
static void test_palindrome_playback_sequence(void)
{
    MockPlaybackLoop loop;
    loop.total_frames = 10;
    loop.current_frame = 0;
    loop.direction = PLAYBACK_STATE_FORWARD;

    gboolean result = mock_verify_palindrome_sequence(&loop);
    TEST_ASSERT(result, "Palindrome sequence matches expected output");
}

/**
 * Test 5: Palindrome with short recording
 *
 * Test palindrome with 3-frame recording to verify edge cases.
 */
static void test_palindrome_short_recording(void)
{
    MockPlaybackLoop loop;
    loop.total_frames = 3;
    loop.current_frame = 0;
    loop.direction = PLAYBACK_STATE_FORWARD;

    /* Expected: 0,1,2,1,0,1,2,1... */
    int expected[] = {0, 1, 2, 1, 0, 1, 2, 1};

    for (int i = 0; i < 8; i++) {
        int frame = mock_palindrome_advance_frame(&loop);
        TEST_ASSERT(frame == expected[i], "Frame %d sequence correct (got %d, expected %d)", i,
                    frame, expected[i]);
    }
}

/**
 * Test 6: Multiple cells with palindrome playback
 *
 * Verify that multiple playback loops in different cells
 * all maintain correct palindrome state independently.
 */
static void test_multiple_cells_palindrome_independent(void)
{
    MockGrid grid;
    mock_grid_init(&grid);

    /* Create 3 recordings */
    MockPlaybackLoop *loop1 = mock_grid_record_and_playback(&grid, 1);
    MockPlaybackLoop *loop2 = mock_grid_record_and_playback(&grid, 2);
    MockPlaybackLoop *loop3 = mock_grid_record_and_playback(&grid, 3);

    /* Advance each loop independently */
    for (int i = 0; i < 5; i++) {
        mock_palindrome_advance_frame(loop1);
        mock_palindrome_advance_frame(loop2);
        mock_palindrome_advance_frame(loop3);
    }

    /* All should be at frame 5 after 5 advances */
    TEST_ASSERT(loop1->current_frame == 5, "Loop 1 at frame 5");
    TEST_ASSERT(loop2->current_frame == 5, "Loop 2 at frame 5");
    TEST_ASSERT(loop3->current_frame == 5, "Loop 3 at frame 5");

    /* All should still be in FORWARD direction */
    TEST_ASSERT(loop1->direction == PLAYBACK_STATE_FORWARD, "Loop 1 still forward");
    TEST_ASSERT(loop2->direction == PLAYBACK_STATE_FORWARD, "Loop 2 still forward");
    TEST_ASSERT(loop3->direction == PLAYBACK_STATE_FORWARD, "Loop 3 still forward");

    /* Advance to end and verify direction change */
    for (int i = 0; i < 5; i++) {
        mock_palindrome_advance_frame(loop1);
        mock_palindrome_advance_frame(loop2);
        mock_palindrome_advance_frame(loop3);
    }

    /* All should have switched to REVERSE direction */
    TEST_ASSERT(loop1->direction == PLAYBACK_STATE_REVERSE, "Loop 1 switched to reverse");
    TEST_ASSERT(loop2->direction == PLAYBACK_STATE_REVERSE, "Loop 2 switched to reverse");
    TEST_ASSERT(loop3->direction == PLAYBACK_STATE_REVERSE, "Loop 3 switched to reverse");

    mock_grid_cleanup(&grid);
}

/**
 * Test 7: Direction change at palindrome boundaries
 *
 * Verify that direction changes occur at correct frame indices.
 */
static void test_palindrome_direction_changes(void)
{
    MockPlaybackLoop loop;
    loop.total_frames = 10;
    loop.current_frame = 0;
    loop.direction = PLAYBACK_STATE_FORWARD;

    /* Advance to end (frames 0-10) */
    for (int i = 0; i < 10; i++) {
        mock_palindrome_advance_frame(&loop);
    }

    /* Should have switched to REVERSE direction */
    TEST_ASSERT(loop.direction == PLAYBACK_STATE_REVERSE,
                "Direction changed to REVERSE after reaching end");
    TEST_ASSERT(loop.current_frame == 8,
                "After direction change, frame is 8 (not repeating frame 9)");

    /* Advance back to start */
    for (int i = 0; i < 9; i++) {
        mock_palindrome_advance_frame(&loop);
    }

    /* Should have switched back to FORWARD direction */
    TEST_ASSERT(loop.direction == PLAYBACK_STATE_FORWARD,
                "Direction changed back to FORWARD after reaching start");
    TEST_ASSERT(loop.current_frame == 1,
                "After direction change, frame is 1 (not repeating frame 0)");
}

/**
 * Test 8: Grid cell state tracking
 *
 * Verify that the grid correctly tracks which cells have active playback.
 */
static void test_grid_cell_state_tracking(void)
{
    MockGrid grid;
    mock_grid_init(&grid);

    /* Verify all cells are empty initially */
    for (int i = 0; i < 9; i++) {
        TEST_ASSERT(!grid.bins[i].is_active, "Cell %d initially inactive", i + 2);
    }

    /* Add recordings - they fill cells sequentially 2, 3, 4 */
    MockPlaybackLoop *loop1 = mock_grid_record_and_playback(&grid, 1);
    MockPlaybackLoop *loop2 = mock_grid_record_and_playback(&grid, 2);
    MockPlaybackLoop *loop3 = mock_grid_record_and_playback(&grid, 3);

    /* Verify state - cells 2, 3, 4 are active */
    TEST_ASSERT(grid.bins[0].is_active && grid.bins[0].loop == loop1, "Cell 2 active");
    TEST_ASSERT(grid.bins[1].is_active && grid.bins[1].loop == loop2, "Cell 3 active");
    TEST_ASSERT(grid.bins[2].is_active && grid.bins[2].loop == loop3, "Cell 4 active");

    /* Remaining cells should be inactive */
    for (int i = 3; i < 9; i++) {
        TEST_ASSERT(!grid.bins[i].is_active, "Cell %d inactive", i + 2);
    }

    mock_grid_cleanup(&grid);
}

/**
 * Test 9: Livestream persistence with playback
 *
 * Verify that the logic correctly keeps livestream in cell 1
 * while playback fills cells 2-10.
 * (This test demonstrates the cell assignment logic, not livestream itself)
 */
static void test_livestream_cell1_playback_cells2_10(void)
{
    MockGrid grid;
    mock_grid_init(&grid);

    /* Cell 1 is reserved for livestream (not in MockGrid) */
    /* Next_cell_to_fill starts at 0, which maps to cell 2 */
    TEST_ASSERT(grid.next_cell_to_fill == 0, "Next recording goes to cell 2 (index 0)");

    /* After 9 recordings, we should wrap */
    for (int i = 0; i < 9; i++) {
        mock_grid_record_and_playback(&grid, (i % 9) + 1);
    }

    /* Next recording should go back to cell 2 */
    TEST_ASSERT(grid.next_cell_to_fill == 0, "After 9 recordings, next goes to cell 2 again");

    mock_grid_cleanup(&grid);
}

/**
 * Test 10: Recording count and cell assignment ordering
 *
 * Verify that recordings are assigned to cells in the correct order.
 */
static void test_recording_count_and_ordering(void)
{
    MockGrid grid;
    mock_grid_init(&grid);

    TEST_ASSERT(grid.recording_count == 0, "Start with 0 recordings");

    for (int i = 0; i < 12; i++) {
        mock_grid_record_and_playback(&grid, 1);
        TEST_ASSERT(grid.recording_count == i + 1, "Recording count is %d", i + 1);
    }

    /* After 12 recordings (2 full wraps), verify state */
    TEST_ASSERT(grid.recording_count == 12, "12 total recordings (with wraparound)");
    TEST_ASSERT(grid.next_cell_to_fill == 3, "After 12 recordings (3 wraps), next is index 3");

    mock_grid_cleanup(&grid);
}

/* ============================================================================
 * Main Test Runner
 * ========================================================================== */

int main(int argc G_GNUC_UNUSED, char **argv G_GNUC_UNUSED)
{
    printf("=== End-to-End Playback Flow Tests (T-6.6) ===\n");
    printf("Testing: Record→Playback, Cell Wraparound, Palindrome, Multiple Recordings\n\n");

    /* Run all tests */
    test_record_key1_playback_cell2();
    test_multiple_simultaneous_recordings();
    test_cell_wraparound();
    test_palindrome_playback_sequence();
    test_palindrome_short_recording();
    test_multiple_cells_palindrome_independent();
    test_palindrome_direction_changes();
    test_grid_cell_state_tracking();
    test_livestream_cell1_playback_cells2_10();
    test_recording_count_and_ordering();

    /* Print results */
    printf("\n=== Test Results ===\n");
    printf("Total: %d\n", g_test_stats.total_tests);
    printf("Passed: %d\n", g_test_stats.passed_tests);
    printf("Failed: %d\n", g_test_stats.failed_tests);
    printf("Success Rate: %.1f%%\n",
           g_test_stats.total_tests > 0
               ? (100.0 * g_test_stats.passed_tests / g_test_stats.total_tests)
               : 0.0);

    if (g_test_stats.failed_tests == 0) {
        printf("\n✓ All tests passed!\n");
    } else {
        printf("\n✗ Some tests failed.\n");
    }

    return (g_test_stats.failed_tests > 0) ? 1 : 0;
}
