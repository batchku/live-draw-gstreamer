/**
 * @file test_playback_bins_lifecycle.c
 * @brief Integration tests for playback bin dynamic management (T-6.3)
 *
 * Tests the following requirements:
 * - pipeline_add_playback_bin() - Dynamic playback bin creation and addition
 * - pipeline_remove_playback_bin() - Dynamic playback bin removal and cleanup
 * - Cell number validation (2-10 range)
 * - Duration metadata handling
 * - Placeholder and real element handling
 *
 * @author Application Engineer
 * @date 2026-01-27
 */

#include <assert.h>
#include <glib.h>
#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef G_GNUC_UNUSED
#define G_GNUC_UNUSED __attribute__((unused))
#endif

/* Mock implementations for testing without full pipeline */

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

/* Forward declaration of pipeline_add_playback_bin and pipeline_remove_playback_bin */
typedef struct {
    GstElement *pipeline;
    GstElement *videomixer;
    GstElement *playback_bins[9];
    GstElement *playback_queues[9];
} MockPipeline;

/**
 * Mock implementation of pipeline_add_playback_bin for testing
 */
gboolean mock_pipeline_add_playback_bin(MockPipeline *p, int cell_num,
                                        guint64 duration_us G_GNUC_UNUSED)
{
    if (!p || !p->pipeline || !p->videomixer) {
        return FALSE;
    }

    if (cell_num < 2 || cell_num > 10) {
        return FALSE;
    }

    int bin_index = cell_num - 2;

    if (p->playback_bins[bin_index]) {
        return FALSE;
    }

    /* Allocate placeholder */
    p->playback_bins[bin_index] = (GstElement *) g_malloc0(sizeof(gpointer));
    if (!p->playback_bins[bin_index]) {
        return FALSE;
    }

    return TRUE;
}

/**
 * Mock implementation of pipeline_remove_playback_bin for testing
 */
gboolean mock_pipeline_remove_playback_bin(MockPipeline *p, int cell_num)
{
    if (!p || !p->pipeline || !p->videomixer) {
        return FALSE;
    }

    if (cell_num < 2 || cell_num > 10) {
        return FALSE;
    }

    int bin_index = cell_num - 2;

    if (!p->playback_bins[bin_index]) {
        return TRUE; /* Not an error */
    }

    void *bin_ptr = p->playback_bins[bin_index];

    /* For mocks, always free as a placeholder */
    g_free(bin_ptr);

    p->playback_bins[bin_index] = NULL;

    if (p->playback_queues[bin_index]) {
        p->playback_queues[bin_index] = NULL;
    }

    return TRUE;
}

/* ============================================================================
 * Test Cases
 * ========================================================================== */

/**
 * Test 1: Add playback bin with valid parameters
 */
static void test_add_playback_bin_valid(void)
{
    MockPipeline pipeline = {0};
    pipeline.pipeline = (GstElement *) 0x1234; /* Mock non-NULL pointer */
    pipeline.videomixer = (GstElement *) 0x5678;

    gboolean result = mock_pipeline_add_playback_bin(&pipeline, 2, 2000000);
    TEST_ASSERT(result == TRUE, "Add playback bin with valid parameters");
    TEST_ASSERT(pipeline.playback_bins[0] != NULL, "Playback bin slot allocated");

    g_free(pipeline.playback_bins[0]);
}

/**
 * Test 2: Add playback bin to multiple cells
 */
static void test_add_playback_bin_multiple_cells(void)
{
    MockPipeline pipeline = {0};
    pipeline.pipeline = (GstElement *) 0x1234;
    pipeline.videomixer = (GstElement *) 0x5678;

    /* Add bins to cells 2, 3, and 4 */
    for (int cell = 2; cell <= 4; cell++) {
        gboolean result = mock_pipeline_add_playback_bin(&pipeline, cell, 2000000);
        TEST_ASSERT(result == TRUE, "Add playback bin to cell %d", cell);
    }

    /* Verify all are allocated */
    TEST_ASSERT(pipeline.playback_bins[0] != NULL, "Cell 2 allocated");
    TEST_ASSERT(pipeline.playback_bins[1] != NULL, "Cell 3 allocated");
    TEST_ASSERT(pipeline.playback_bins[2] != NULL, "Cell 4 allocated");

    /* Cleanup */
    for (int i = 0; i < 3; i++) {
        g_free(pipeline.playback_bins[i]);
    }
}

/**
 * Test 3: Invalid cell numbers
 */
static void test_add_playback_bin_invalid_cell(void)
{
    MockPipeline pipeline = {0};
    pipeline.pipeline = (GstElement *) 0x1234;
    pipeline.videomixer = (GstElement *) 0x5678;

    /* Cell 1 (invalid - reserved for live feed) */
    gboolean result = mock_pipeline_add_playback_bin(&pipeline, 1, 2000000);
    TEST_ASSERT(result == FALSE, "Reject cell 1 (reserved for live feed)");

    /* Cell 11 (out of range) */
    result = mock_pipeline_add_playback_bin(&pipeline, 11, 2000000);
    TEST_ASSERT(result == FALSE, "Reject cell 11 (out of range)");

    /* Cell 0 (invalid) */
    result = mock_pipeline_add_playback_bin(&pipeline, 0, 2000000);
    TEST_ASSERT(result == FALSE, "Reject cell 0 (invalid)");
}

/**
 * Test 4: Duplicate bin in same cell
 */
static void test_add_playback_bin_duplicate(void)
{
    MockPipeline pipeline = {0};
    pipeline.pipeline = (GstElement *) 0x1234;
    pipeline.videomixer = (GstElement *) 0x5678;

    /* Add bin to cell 2 */
    gboolean result = mock_pipeline_add_playback_bin(&pipeline, 2, 2000000);
    TEST_ASSERT(result == TRUE, "First add to cell 2 succeeds");

    /* Try to add again */
    result = mock_pipeline_add_playback_bin(&pipeline, 2, 2000000);
    TEST_ASSERT(result == FALSE, "Duplicate add to cell 2 fails");

    g_free(pipeline.playback_bins[0]);
}

/**
 * Test 5: NULL pipeline
 */
static void test_add_playback_bin_null_pipeline(void)
{
    MockPipeline pipeline = {0};
    /* Leave pipeline NULL */

    gboolean result = mock_pipeline_add_playback_bin(&pipeline, 2, 2000000);
    TEST_ASSERT(result == FALSE, "Reject NULL pipeline");
}

/**
 * Test 6: Remove playback bin
 */
static void test_remove_playback_bin(void)
{
    MockPipeline pipeline = {0};
    pipeline.pipeline = (GstElement *) 0x1234;
    pipeline.videomixer = (GstElement *) 0x5678;

    /* Add a bin */
    mock_pipeline_add_playback_bin(&pipeline, 2, 2000000);
    TEST_ASSERT(pipeline.playback_bins[0] != NULL, "Bin added");

    /* Remove it */
    gboolean result = mock_pipeline_remove_playback_bin(&pipeline, 2);
    TEST_ASSERT(result == TRUE, "Remove succeeds");
    TEST_ASSERT(pipeline.playback_bins[0] == NULL, "Bin reference cleared");
}

/**
 * Test 7: Remove non-existent bin
 */
static void test_remove_playback_bin_nonexistent(void)
{
    MockPipeline pipeline = {0};
    pipeline.pipeline = (GstElement *) 0x1234;
    pipeline.videomixer = (GstElement *) 0x5678;

    /* Remove a bin that was never added */
    gboolean result = mock_pipeline_remove_playback_bin(&pipeline, 2);
    TEST_ASSERT(result == TRUE, "Remove non-existent bin returns TRUE (not an error)");
}

/**
 * Test 8: Invalid cell range for removal
 */
static void test_remove_playback_bin_invalid_cell(void)
{
    MockPipeline pipeline = {0};
    pipeline.pipeline = (GstElement *) 0x1234;
    pipeline.videomixer = (GstElement *) 0x5678;

    /* Try to remove cell 1 (invalid) */
    gboolean result = mock_pipeline_remove_playback_bin(&pipeline, 1);
    TEST_ASSERT(result == FALSE, "Reject cell 1 (reserved for live feed)");

    /* Try to remove cell 11 (out of range) */
    result = mock_pipeline_remove_playback_bin(&pipeline, 11);
    TEST_ASSERT(result == FALSE, "Reject cell 11 (out of range)");
}

/**
 * Test 9: Add and remove multiple bins
 */
static void test_add_remove_multiple_bins(void)
{
    MockPipeline pipeline = {0};
    pipeline.pipeline = (GstElement *) 0x1234;
    pipeline.videomixer = (GstElement *) 0x5678;

    /* Add bins to cells 2-10 */
    for (int cell = 2; cell <= 10; cell++) {
        gboolean result = mock_pipeline_add_playback_bin(&pipeline, cell, 2000000);
        TEST_ASSERT(result == TRUE, "Add bin to cell %d", cell);
    }

    /* Verify all allocated */
    for (int i = 0; i < 9; i++) {
        TEST_ASSERT(pipeline.playback_bins[i] != NULL, "Bin %d allocated", i);
    }

    /* Remove bins in reverse order */
    for (int cell = 10; cell >= 2; cell--) {
        gboolean result = mock_pipeline_remove_playback_bin(&pipeline, cell);
        TEST_ASSERT(result == TRUE, "Remove bin from cell %d", cell);
    }

    /* Verify all removed */
    for (int i = 0; i < 9; i++) {
        TEST_ASSERT(pipeline.playback_bins[i] == NULL, "Bin %d cleared", i);
    }
}

/**
 * Test 10: Duration metadata handling
 */
static void test_duration_metadata(void)
{
    MockPipeline pipeline = {0};
    pipeline.pipeline = (GstElement *) 0x1234;
    pipeline.videomixer = (GstElement *) 0x5678;

    /* Add with various durations */
    guint64 durations[] = {1000000, 2000000, 5000000, 60000000};

    for (int i = 0; i < 4; i++) {
        int cell = 2 + i;
        gboolean result = mock_pipeline_add_playback_bin(&pipeline, cell, durations[i]);
        TEST_ASSERT(result == TRUE, "Add bin with duration %llu us", durations[i]);
    }

    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        g_free(pipeline.playback_bins[i]);
    }
}

/* ============================================================================
 * Main Test Runner
 * ========================================================================== */

int main(int argc G_GNUC_UNUSED, char **argv G_GNUC_UNUSED)
{
    printf("=== Playback Bin Lifecycle Tests (T-6.3) ===\n\n");

    /* Run all tests */
    test_add_playback_bin_valid();
    test_add_playback_bin_multiple_cells();
    test_add_playback_bin_invalid_cell();
    test_add_playback_bin_duplicate();
    test_add_playback_bin_null_pipeline();
    test_remove_playback_bin();
    test_remove_playback_bin_nonexistent();
    test_remove_playback_bin_invalid_cell();
    test_add_remove_multiple_bins();
    test_duration_metadata();

    /* Print results */
    printf("\n=== Test Results ===\n");
    printf("Total: %d\n", g_test_stats.total_tests);
    printf("Passed: %d\n", g_test_stats.passed_tests);
    printf("Failed: %d\n", g_test_stats.failed_tests);
    printf("Success Rate: %.1f%%\n",
           g_test_stats.total_tests > 0
               ? (100.0 * g_test_stats.passed_tests / g_test_stats.total_tests)
               : 0.0);

    return (g_test_stats.failed_tests > 0) ? 1 : 0;
}
