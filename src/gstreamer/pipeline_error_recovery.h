#ifndef PIPELINE_ERROR_RECOVERY_H
#define PIPELINE_ERROR_RECOVERY_H

#include <glib.h>
#include <gst/gst.h>

/**
 * @file pipeline_error_recovery.h
 * @brief Pipeline-level error recovery and fault tolerance mechanisms
 *
 * Provides higher-level error recovery for the video looper pipeline,
 * coordinating with GStreamer error handler for coherent recovery strategies.
 */

/**
 * @enum PipelineRecoveryStrategy
 * @brief Strategies for recovering from various pipeline failures
 */
typedef enum {
    PIPELINE_RECOVERY_NONE,         /* No recovery attempted */
    PIPELINE_RECOVERY_STATE_REVERT, /* Revert to previous state */
    PIPELINE_RECOVERY_FORCE_READY,  /* Force to READY state */
    PIPELINE_RECOVERY_FULL_RESET,   /* Reset entire pipeline to NULL */
} PipelineRecoveryStrategy;

/**
 * Callback for pipeline recovery events
 *
 * @param strategy    Recovery strategy used
 * @param success     TRUE if recovery succeeded, FALSE if pipeline is unrecoverable
 * @param user_data   Optional user data
 */
typedef void (*PipelineRecoveryCallback)(PipelineRecoveryStrategy strategy, gboolean success,
                                         gpointer user_data);

/**
 * Attempt to recover a pipeline from a state change failure
 *
 * Uses increasingly aggressive recovery strategies:
 * 1. Revert to previous state
 * 2. Force to READY state
 * 3. Complete reset to NULL
 *
 * @param pipeline    GStreamer pipeline to recover
 * @param target_state The state that was being transitioned to
 * @param prev_state  The previous stable state
 * @param callback    Callback for recovery progress (may be NULL)
 * @return            TRUE if recovery successful, FALSE if unrecoverable
 */
gboolean pipeline_error_attempt_recovery(GstElement *pipeline, GstState target_state,
                                         GstState prev_state, PipelineRecoveryCallback callback);

/**
 * Get recovery strategy string representation
 *
 * @param strategy    Recovery strategy
 * @return            Human-readable string
 */
const gchar *pipeline_error_strategy_to_string(PipelineRecoveryStrategy strategy);

#endif /* PIPELINE_ERROR_RECOVERY_H */
