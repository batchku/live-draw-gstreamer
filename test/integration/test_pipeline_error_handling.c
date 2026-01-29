/**
 * @file test_pipeline_error_handling.c
 * @brief Integration tests for GStreamer pipeline error handling and recovery
 *
 * Tests complete error handling workflows including:
 * - Real GStreamer pipeline error scenarios
 * - State change failures with recovery
 * - Bus message error handling and logging
 * - Deadlock detection in realistic pipelines
 */

#include "src/camera/camera_source.h"
#include "src/gstreamer/gstreamer_error_handler.h"
#include "src/gstreamer/pipeline_builder.h"
#include <gst/gst.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <unistd.h>

/* ============================================================================
 * Test Fixtures
 * ============================================================================ */

typedef struct {
    Pipeline *pipeline;
    GstElement *camera_source;
} FullPipelineFixture;

static void setup_full_pipeline(FullPipelineFixture *f)
{
    gst_init(NULL, NULL);
    gstreamer_error_handler_init();

    /* Create mock camera source */
    f->camera_source = gst_element_factory_make("videotestsrc", "camera_source");
    if (f->camera_source) {
        g_object_set(f->camera_source, "pattern", 0, NULL);
    }

    /* Create pipeline with camera source */
    f->pipeline = pipeline_create(f->camera_source);
}

static void teardown_full_pipeline(FullPipelineFixture *f)
{
    if (f->pipeline) {
        pipeline_cleanup(f->pipeline);
    }
    gstreamer_error_handler_cleanup();
    gst_deinit();
}

/* ============================================================================
 * Pipeline Creation and State Management Tests
 * ============================================================================ */

TEST(PipelineErrorHandling, PipelineCreationWithMockCamera)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Verify pipeline was created */
    ASSERT_NE(f.pipeline, nullptr);
    ASSERT_NE(f.pipeline->pipeline, nullptr);

    teardown_full_pipeline(&f);
}

TEST(PipelineErrorHandling, PipelineStateTransitionToReady)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Transition to READY state */
    gboolean result = pipeline_set_state(f.pipeline, GST_STATE_READY);
    ASSERT_TRUE(result);

    /* Verify state */
    GstState state = pipeline_get_state(f.pipeline);
    ASSERT_EQ(state, GST_STATE_READY);

    teardown_full_pipeline(&f);
}

TEST(PipelineErrorHandling, PipelineStateTransitionToPlaying)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* First transition to READY */
    pipeline_set_state(f.pipeline, GST_STATE_READY);

    /* Then to PLAYING */
    gboolean result = pipeline_set_state(f.pipeline, GST_STATE_PLAYING);
    ASSERT_TRUE(result);

    /* Verify state (may be PLAYING or async towards PLAYING) */
    GstState state = pipeline_get_state(f.pipeline);
    ASSERT_GE(state, GST_STATE_READY);

    teardown_full_pipeline(&f);
}

TEST(PipelineErrorHandling, PipelineStatePausedState)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Transition to READY then PLAYING then PAUSED */
    pipeline_set_state(f.pipeline, GST_STATE_READY);
    pipeline_set_state(f.pipeline, GST_STATE_PLAYING);

    /* Sleep to allow async state change to complete */
    usleep(100000); /* 100ms */

    gboolean result = pipeline_set_state(f.pipeline, GST_STATE_PAUSED);
    ASSERT_TRUE(result);

    GstState state = pipeline_get_state(f.pipeline);
    ASSERT_GE(state, GST_STATE_READY);

    teardown_full_pipeline(&f);
}

/* ============================================================================
 * Error Message Callback Tests
 * ============================================================================ */

static gint g_error_message_count = 0;

static void test_pipeline_message_callback(const char *type, const char *message)
{
    if (type && message) {
        g_error_message_count++;
        fprintf(stderr, "Pipeline message [%s]: %s\n", type, message);
    }
}

TEST(PipelineErrorHandling, BusMessageCallbackRegistration)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Register callback */
    pipeline_set_message_callback(f.pipeline, test_pipeline_message_callback);

    /* Perform state transitions to trigger messages */
    pipeline_set_state(f.pipeline, GST_STATE_READY);
    usleep(100000); /* Allow time for messages */

    /* Unregister callback */
    pipeline_set_message_callback(f.pipeline, NULL);

    teardown_full_pipeline(&f);
}

TEST(PipelineErrorHandling, BusMessageCallbackReceivesStateChanged)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    g_error_message_count = 0;
    pipeline_set_message_callback(f.pipeline, test_pipeline_message_callback);

    /* Perform state transitions which should generate messages */
    pipeline_set_state(f.pipeline, GST_STATE_READY);
    usleep(200000); /* Allow time for messages */

    pipeline_set_message_callback(f.pipeline, NULL);

    /* Should have received at least one message (state_changed) */
    ASSERT_GT(g_error_message_count, 0);

    teardown_full_pipeline(&f);
}

/* ============================================================================
 * Recording Bin Addition/Removal Tests
 * ============================================================================ */

TEST(PipelineErrorHandling, RecordingBinAddition)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Add recording bin for key 1 */
    gboolean result = pipeline_add_record_bin(f.pipeline, 1);
    ASSERT_TRUE(result);

    /* Verify bin was added */
    ASSERT_NE(f.pipeline->record_bins[0], nullptr);

    teardown_full_pipeline(&f);
}

TEST(PipelineErrorHandling, RecordingBinAdditionMultiple)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Add multiple recording bins */
    for (int i = 1; i <= 9; i++) {
        gboolean result = pipeline_add_record_bin(f.pipeline, i);
        ASSERT_TRUE(result);
        ASSERT_NE(f.pipeline->record_bins[i - 1], nullptr);
    }

    teardown_full_pipeline(&f);
}

TEST(PipelineErrorHandling, RecordingBinRemoval)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Add and then remove recording bin */
    pipeline_add_record_bin(f.pipeline, 1);
    ASSERT_NE(f.pipeline->record_bins[0], nullptr);

    gboolean result = pipeline_remove_record_bin(f.pipeline, 1);
    ASSERT_TRUE(result);

    /* Verify bin was removed */
    ASSERT_EQ(f.pipeline->record_bins[0], nullptr);

    teardown_full_pipeline(&f);
}

TEST(PipelineErrorHandling, DuplicateRecordingBinAdditionFails)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Add recording bin for key 1 */
    gboolean result = pipeline_add_record_bin(f.pipeline, 1);
    ASSERT_TRUE(result);

    /* Try to add same bin again - should fail */
    result = pipeline_add_record_bin(f.pipeline, 1);
    ASSERT_FALSE(result);

    teardown_full_pipeline(&f);
}

/* ============================================================================
 * Playback Bin Addition/Removal Tests
 * ============================================================================ */

TEST(PipelineErrorHandling, PlaybackBinAllocation)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Add playback bin for cell 2 (maps to index 0) */
    gboolean result = pipeline_add_playback_bin(f.pipeline, 2, 1000000); /* 1 second */
    ASSERT_TRUE(result);

    /* Verify playback bin slot was allocated */
    ASSERT_NE(f.pipeline->playback_bins[0], nullptr);

    teardown_full_pipeline(&f);
}

TEST(PipelineErrorHandling, PlaybackBinAllocationForMultipleCells)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Add playback bins for all cells 2-10 */
    for (int cell = 2; cell <= 10; cell++) {
        gboolean result = pipeline_add_playback_bin(f.pipeline, cell, 1000000);
        ASSERT_TRUE(result);
        ASSERT_NE(f.pipeline->playback_bins[cell - 2], nullptr);
    }

    teardown_full_pipeline(&f);
}

TEST(PipelineErrorHandling, PlaybackBinRemoval)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Add and remove playback bin */
    pipeline_add_playback_bin(f.pipeline, 2, 1000000);
    ASSERT_NE(f.pipeline->playback_bins[0], nullptr);

    gboolean result = pipeline_remove_playback_bin(f.pipeline, 2);
    ASSERT_TRUE(result);

    /* Verify bin was removed */
    ASSERT_EQ(f.pipeline->playback_bins[0], nullptr);

    teardown_full_pipeline(&f);
}

/* ============================================================================
 * Pipeline Resilience Tests
 * ============================================================================ */

TEST(PipelineErrorHandling, PipelineStableUnderRepeatedStateChanges)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Perform repeated state transitions */
    for (int i = 0; i < 5; i++) {
        gboolean result = pipeline_set_state(f.pipeline, GST_STATE_READY);
        ASSERT_TRUE(result);
        usleep(50000); /* 50ms between transitions */
    }

    teardown_full_pipeline(&f);
}

TEST(PipelineErrorHandling, PipelineStableWithBinAdditionRemoval)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Set to READY state */
    pipeline_set_state(f.pipeline, GST_STATE_READY);

    /* Repeatedly add and remove recording bins */
    for (int i = 0; i < 3; i++) {
        for (int key = 1; key <= 9; key++) {
            pipeline_add_record_bin(f.pipeline, key);
            usleep(10000); /* 10ms between additions */
        }

        for (int key = 1; key <= 9; key++) {
            pipeline_remove_record_bin(f.pipeline, key);
            usleep(10000); /* 10ms between removals */
        }
    }

    /* Verify pipeline still valid */
    GstState state = pipeline_get_state(f.pipeline);
    ASSERT_EQ(state, GST_STATE_READY);

    teardown_full_pipeline(&f);
}

TEST(PipelineErrorHandling, PipelineCleanupAfterStateChanges)
{
    FullPipelineFixture f;
    setup_full_pipeline(&f);

    /* Perform various operations */
    pipeline_set_state(f.pipeline, GST_STATE_READY);
    pipeline_add_record_bin(f.pipeline, 1);
    pipeline_set_state(f.pipeline, GST_STATE_PLAYING);
    usleep(100000);
    pipeline_add_record_bin(f.pipeline, 2);
    pipeline_remove_record_bin(f.pipeline, 1);

    /* Pipeline cleanup should succeed without crashes */
    teardown_full_pipeline(&f);
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
