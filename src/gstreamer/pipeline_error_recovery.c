/**
 * @file pipeline_error_recovery.c
 * @brief Pipeline-level error recovery and fault tolerance implementation
 */

#include "pipeline_error_recovery.h"
#include "../app/app_error.h"
#include "../utils/logging.h"
#include "gstreamer_error_handler.h"
#include <string.h>

gboolean pipeline_error_attempt_recovery(GstElement *pipeline, GstState target_state,
                                         GstState prev_state, PipelineRecoveryCallback callback)
{
    if (!pipeline) {
        LOG_ERROR("Invalid pipeline for recovery");
        if (callback) {
            callback(PIPELINE_RECOVERY_NONE, FALSE, NULL);
        }
        return FALSE;
    }

    LOG_WARNING("Attempting pipeline error recovery");
    LOG_WARNING("Failed state transition: %s → %s", gst_element_state_get_name(prev_state),
                gst_element_state_get_name(target_state));

    /* Strategy 1: Revert to previous state */
    LOG_INFO("Recovery Strategy 1: Reverting to %s state", gst_element_state_get_name(prev_state));

    GstStateChangeReturn ret = gst_element_set_state(pipeline, prev_state);
    if (ret != GST_STATE_CHANGE_FAILURE) {
        LOG_INFO("Recovery Strategy 1 SUCCESSFUL: Pipeline reverted to %s",
                 gst_element_state_get_name(prev_state));
        if (callback) {
            callback(PIPELINE_RECOVERY_STATE_REVERT, TRUE, NULL);
        }
        return TRUE;
    }

    LOG_WARNING("Recovery Strategy 1 FAILED: Could not revert to %s",
                gst_element_state_get_name(prev_state));
    if (callback) {
        callback(PIPELINE_RECOVERY_STATE_REVERT, FALSE, NULL);
    }

    /* Strategy 2: Force to READY state (stop processing, retain resources) */
    LOG_INFO("Recovery Strategy 2: Forcing pipeline to READY state");

    ret = gst_element_set_state(pipeline, GST_STATE_READY);
    if (ret != GST_STATE_CHANGE_FAILURE) {
        LOG_INFO("Recovery Strategy 2 SUCCESSFUL: Pipeline forced to READY");
        if (callback) {
            callback(PIPELINE_RECOVERY_FORCE_READY, TRUE, NULL);
        }
        return TRUE;
    }

    LOG_ERROR("Recovery Strategy 2 FAILED: Could not force to READY state");
    if (callback) {
        callback(PIPELINE_RECOVERY_FORCE_READY, FALSE, NULL);
    }

    /* Strategy 3: Complete reset to NULL (stop, release resources) */
    LOG_ERROR("Recovery Strategy 3: Full reset - forcing pipeline to NULL state");

    ret = gst_element_set_state(pipeline, GST_STATE_NULL);
    if (ret != GST_STATE_CHANGE_FAILURE) {
        LOG_ERROR("Recovery Strategy 3 COMPLETED: Pipeline reset to NULL state");
        LOG_ERROR("WARNING: Pipeline will require full restart to continue operation");

        if (callback) {
            callback(PIPELINE_RECOVERY_FULL_RESET, TRUE, NULL);
        }

        app_log_error(APP_ERROR_PIPELINE_STATE_CHANGE_FAILED,
                      "Pipeline error recovery required full reset; restart recommended");

        return TRUE;
    }

    LOG_ERROR("UNRECOVERABLE ERROR: All recovery strategies failed");
    LOG_ERROR("Pipeline is in an inconsistent state and cannot be recovered");

    if (callback) {
        callback(PIPELINE_RECOVERY_FULL_RESET, FALSE, NULL);
    }

    app_log_error(APP_ERROR_PIPELINE_STATE_CHANGE_FAILED,
                  "CRITICAL: Pipeline is unrecoverable and requires application restart");

    return FALSE;
}

const gchar *pipeline_error_strategy_to_string(PipelineRecoveryStrategy strategy)
{
    switch (strategy) {
    case PIPELINE_RECOVERY_NONE:
        return "No Recovery";
    case PIPELINE_RECOVERY_STATE_REVERT:
        return "Revert to Previous State";
    case PIPELINE_RECOVERY_FORCE_READY:
        return "Force to READY State";
    case PIPELINE_RECOVERY_FULL_RESET:
        return "Full Reset to NULL";
    default:
        return "Unknown Strategy";
    }
}
