/**
 * @file test_gstreamer_error_handler.c
 * @brief Unit tests for GStreamer error handling and deadlock detection
 *
 * Tests the comprehensive error handling system including:
 * - Bus error categorization and logging
 * - State change failure detection
 * - Deadlock detection and recovery mechanisms
 * - Error callback dispatch
 */

#include "src/gstreamer/gstreamer_error_handler.h"
#include "src/gstreamer/pipeline_error_recovery.h"
#include <gst/gst.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Test Fixtures
 * ============================================================================ */

typedef struct {
    GstElement *pipeline;
    GstElement *source;
    GstElement *sink;
} PipelineTestFixture;

static void setup_pipeline_fixture(PipelineTestFixture *f)
{
    gst_init(NULL, NULL);

    /* Create a simple test pipeline */
    f->pipeline = gst_pipeline_new("test-pipeline");
    f->source = gst_element_factory_make("videotestsrc", "source");
    f->sink = gst_element_factory_make("fakevideosink", "sink");

    if (f->source && f->sink) {
        gst_bin_add_many(GST_BIN(f->pipeline), f->source, f->sink, NULL);
        gst_element_link(f->source, f->sink);
    }
}

static void teardown_pipeline_fixture(PipelineTestFixture *f)
{
    if (f->pipeline) {
        gst_element_set_state(f->pipeline, GST_STATE_NULL);
        gst_object_unref(f->pipeline);
    }
    gst_deinit();
}

/* ============================================================================
 * Error Handler Initialization Tests
 * ============================================================================ */

TEST(GStreamerErrorHandler, Initialization)
{
    gst_init(NULL, NULL);

    /* Initialize error handler */
    gboolean result = gstreamer_error_handler_init();
    ASSERT_TRUE(result);

    /* Second initialization should not fail */
    result = gstreamer_error_handler_init();
    ASSERT_TRUE(result);

    /* Cleanup */
    gstreamer_error_handler_cleanup();
    gst_deinit();
}

TEST(GStreamerErrorHandler, CleanupAfterInit)
{
    gst_init(NULL, NULL);

    gstreamer_error_handler_init();
    gstreamer_error_handler_cleanup();

    /* Should be able to reinitialize after cleanup */
    gboolean result = gstreamer_error_handler_init();
    ASSERT_TRUE(result);

    gstreamer_error_handler_cleanup();
    gst_deinit();
}

/* ============================================================================
 * Error Category Conversion Tests
 * ============================================================================ */

TEST(GStreamerErrorHandler, ErrorCategoryConversion)
{
    gst_init(NULL, NULL);
    gstreamer_error_handler_init();

    const gchar *str;

    str = gstreamer_error_handler_category_to_string(GSTREAMER_ERROR_BUS_ERROR);
    ASSERT_STREQ("Bus Error", str);

    str = gstreamer_error_handler_category_to_string(GSTREAMER_ERROR_STATE_CHANGE_FAILURE);
    ASSERT_STREQ("State Change Failure", str);

    str = gstreamer_error_handler_category_to_string(GSTREAMER_ERROR_DEADLOCK_DETECTED);
    ASSERT_STREQ("Deadlock Detected", str);

    str = gstreamer_error_handler_category_to_string(GSTREAMER_ERROR_ELEMENT_MISSING);
    ASSERT_STREQ("Element Missing", str);

    str = gstreamer_error_handler_category_to_string(GSTREAMER_ERROR_NEGOTIATION);
    ASSERT_STREQ("Caps Negotiation Failure", str);

    str = gstreamer_error_handler_category_to_string(GSTREAMER_ERROR_RESOURCE);
    ASSERT_STREQ("Resource Exhaustion", str);

    gstreamer_error_handler_cleanup();
    gst_deinit();
}

/* ============================================================================
 * Error Callback Tests
 * ============================================================================ */

static gint g_callback_invocation_count = 0;
static GStreamerErrorCategory g_last_callback_category = GSTREAMER_ERROR_UNKNOWN;

static void test_error_callback(const GStreamerErrorInfo *error_info, GstElement *pipeline,
                                gpointer user_data)
{
    g_callback_invocation_count++;
    if (error_info) {
        g_last_callback_category = error_info->category;
    }
}

TEST(GStreamerErrorHandler, ErrorCallbackRegistration)
{
    gst_init(NULL, NULL);
    gstreamer_error_handler_init();

    g_callback_invocation_count = 0;

    /* Register callback */
    gstreamer_error_handler_register_error_callback(test_error_callback, NULL);

    /* Unregister callback */
    gstreamer_error_handler_register_error_callback(NULL, NULL);

    gstreamer_error_handler_cleanup();
    gst_deinit();
}

/* ============================================================================
 * Deadlock Detection Tests
 * ============================================================================ */

TEST(GStreamerErrorHandler, DeadlockDetectionRegistration)
{
    PipelineTestFixture f;
    setup_pipeline_fixture(&f);
    gstreamer_error_handler_init();

    /* Enable deadlock detection for pipeline */
    gboolean result = gstreamer_error_handler_enable_deadlock_detection(f.pipeline, 5000);
    ASSERT_TRUE(result);

    /* Disable deadlock detection */
    gstreamer_error_handler_disable_deadlock_detection(f.pipeline);

    /* Disabling again should be safe (no crash) */
    gstreamer_error_handler_disable_deadlock_detection(f.pipeline);

    gstreamer_error_handler_cleanup();
    teardown_pipeline_fixture(&f);
}

TEST(GStreamerErrorHandler, DeadlockDetectionWithInvalidPipeline)
{
    gst_init(NULL, NULL);
    gstreamer_error_handler_init();

    /* Attempt deadlock detection on NULL pipeline */
    gboolean result = gstreamer_error_handler_enable_deadlock_detection(NULL, 5000);
    ASSERT_FALSE(result);

    gstreamer_error_handler_cleanup();
    gst_deinit();
}

/* ============================================================================
 * State Change with Detection Tests
 * ============================================================================ */

TEST(GStreamerErrorHandler, StateChangeWithDetection)
{
    PipelineTestFixture f;
    setup_pipeline_fixture(&f);
    gstreamer_error_handler_init();

    /* Enable deadlock detection for pipeline */
    gstreamer_error_handler_enable_deadlock_detection(f.pipeline, 5000);

    /* Perform state change with detection */
    gboolean result =
        gstreamer_error_handler_set_state_with_detection(f.pipeline, GST_STATE_READY, 5000);
    ASSERT_TRUE(result);

    /* Verify pipeline is in READY state */
    GstState state;
    gst_element_get_state(f.pipeline, &state, NULL, 0);
    ASSERT_EQ(state, GST_STATE_READY);

    gstreamer_error_handler_cleanup();
    teardown_pipeline_fixture(&f);
}

TEST(GStreamerErrorHandler, StateChangePlayingState)
{
    PipelineTestFixture f;
    setup_pipeline_fixture(&f);
    gstreamer_error_handler_init();

    gstreamer_error_handler_enable_deadlock_detection(f.pipeline, 5000);

    /* Transition to READY first */
    gboolean result =
        gstreamer_error_handler_set_state_with_detection(f.pipeline, GST_STATE_READY, 5000);
    ASSERT_TRUE(result);

    /* Then to PLAYING */
    result = gstreamer_error_handler_set_state_with_detection(f.pipeline, GST_STATE_PLAYING, 5000);
    ASSERT_TRUE(result);

    GstState state;
    gst_element_get_state(f.pipeline, &state, NULL, 0);
    EXPECT_EQ(state, GST_STATE_PLAYING);

    gstreamer_error_handler_cleanup();
    teardown_pipeline_fixture(&f);
}

/* ============================================================================
 * Error Recovery Tests
 * ============================================================================ */

static gint g_recovery_invocation_count = 0;
static PipelineRecoveryStrategy g_last_recovery_strategy = PIPELINE_RECOVERY_NONE;

static void test_recovery_callback(PipelineRecoveryStrategy strategy, gboolean success,
                                   gpointer user_data)
{
    g_recovery_invocation_count++;
    g_last_recovery_strategy = strategy;
}

TEST(GStreamerErrorHandler, AttemptRecoveryFromFailedState)
{
    PipelineTestFixture f;
    setup_pipeline_fixture(&f);
    gstreamer_error_handler_init();

    /* Set pipeline to READY state first */
    gst_element_set_state(f.pipeline, GST_STATE_READY);

    /* Attempt recovery - should succeed reverting to previous state */
    gboolean result =
        gstreamer_error_handler_attempt_recovery(f.pipeline, GST_STATE_PLAYING, GST_STATE_READY);
    ASSERT_TRUE(result);

    /* Pipeline should still be valid */
    GstState state;
    GstStateChangeReturn ret = gst_element_get_state(f.pipeline, &state, NULL, 0);
    ASSERT_NE(ret, GST_STATE_CHANGE_FAILURE);

    gstreamer_error_handler_cleanup();
    teardown_pipeline_fixture(&f);
}

TEST(GStreamerErrorHandler, RecoveryStrategyString)
{
    ASSERT_STREQ("No Recovery", pipeline_error_strategy_to_string(PIPELINE_RECOVERY_NONE));
    ASSERT_STREQ("Revert to Previous State",
                 pipeline_error_strategy_to_string(PIPELINE_RECOVERY_STATE_REVERT));
    ASSERT_STREQ("Force to READY State",
                 pipeline_error_strategy_to_string(PIPELINE_RECOVERY_FORCE_READY));
    ASSERT_STREQ("Full Reset to NULL",
                 pipeline_error_strategy_to_string(PIPELINE_RECOVERY_FULL_RESET));
}

/* ============================================================================
 * Error Info Retrieval Tests
 * ============================================================================ */

TEST(GStreamerErrorHandler, LastErrorRetrieval)
{
    gst_init(NULL, NULL);
    gstreamer_error_handler_init();

    /* Initially no error */
    const GStreamerErrorInfo *error = gstreamer_error_handler_get_last_error();
    ASSERT_EQ(error, nullptr);

    /* Clear error (should be safe on empty state) */
    gstreamer_error_handler_clear_last_error();

    gstreamer_error_handler_cleanup();
    gst_deinit();
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
