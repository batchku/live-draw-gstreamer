/**
 * @file gstreamer_error_handler.c
 * @brief GStreamer error handling, deadlock detection, and recovery implementation
 *
 * Implements comprehensive error handling for GStreamer pipelines including
 * bus error message handling, state change failure detection, deadlock detection
 * with automatic recovery, and detailed logging at all error levels.
 */

#include "gstreamer_error_handler.h"
#include "../app/app_error.h"
#include "../utils/logging.h"
#include "../utils/timing.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>

/**
 * @struct DeadlockDetectionContext
 * @brief Context for monitoring a pipeline for deadlocks
 */
typedef struct {
    GstElement *pipeline;
    GstState target_state;
    GstState previous_state;
    guint64 start_time_us;
    guint timeout_ms;
    guint timer_source_id;
    gboolean detection_active;
} DeadlockDetectionContext;

/**
 * Global error handler state
 */
static struct {
    GStreamerErrorCallback error_callback;
    gpointer error_callback_data;
    GStreamerRecoveryCallback recovery_callback;
    gpointer recovery_callback_data;
    GStreamerErrorInfo last_error;
    GHashTable *deadlock_contexts; /* Maps pipeline pointers to DeadlockDetectionContext */
    gboolean initialized;
} g_error_handler_state = {
    .error_callback = NULL,
    .error_callback_data = NULL,
    .recovery_callback = NULL,
    .recovery_callback_data = NULL,
    .deadlock_contexts = NULL,
    .initialized = FALSE,
};

/* ============================================================================
 * Internal Helper Functions
 * ============================================================================ */

/**
 * Categorize a GStreamer error based on message content
 */
__attribute__((unused))
static GStreamerErrorCategory categorize_gst_error(const gchar *message,
                                                   const gchar *source_element)
{
    (void) source_element;  /* unused parameter */

    if (!message) {
        return GSTREAMER_ERROR_UNKNOWN;
    }

    if (g_strrstr(message, "not found") || g_strrstr(message, "not available")) {
        return GSTREAMER_ERROR_ELEMENT_MISSING;
    }

    if (g_strrstr(message, "negotiation") || g_strrstr(message, "caps")) {
        return GSTREAMER_ERROR_NEGOTIATION;
    }

    if (g_strrstr(message, "resource") || g_strrstr(message, "memory") ||
        g_strrstr(message, "allocation")) {
        return GSTREAMER_ERROR_RESOURCE;
    }

    return GSTREAMER_ERROR_UNKNOWN;
}

/**
 * Record error information for later retrieval
 */
static void record_error_info(GStreamerErrorCategory category, const gchar *message,
                              const gchar *debug_info, const gchar *source_element,
                              GstState failed_state)
{
    g_error_handler_state.last_error.category = category;
    g_error_handler_state.last_error.message = message;
    g_error_handler_state.last_error.debug_info = debug_info;
    g_error_handler_state.last_error.source_element = source_element;
    g_error_handler_state.last_error.failed_state = failed_state;
    g_error_handler_state.last_error.timestamp_us = timing_get_time_us();
}

/**
 * Dispatch error to registered callback
 */
static void dispatch_error_callback(const GStreamerErrorInfo *error_info, GstElement *pipeline)
{
    if (g_error_handler_state.error_callback) {
        g_error_handler_state.error_callback(error_info, pipeline,
                                             g_error_handler_state.error_callback_data);
    }
}

/**
 * Dispatch recovery action to registered callback
 */
static void dispatch_recovery_callback(const gchar *action, gboolean success)
{
    if (g_error_handler_state.recovery_callback) {
        g_error_handler_state.recovery_callback(action, success,
                                                g_error_handler_state.recovery_callback_data);
    }
}

/**
 * Timeout callback for deadlock detection
 */
static gboolean deadlock_detection_timeout(gpointer user_data)
{
    DeadlockDetectionContext *ctx = (DeadlockDetectionContext *) user_data;

    if (!ctx || !ctx->pipeline) {
        return FALSE; /* Remove this timer */
    }

    guint64 elapsed_us = timing_get_time_us() - ctx->start_time_us;
    guint64 elapsed_ms = elapsed_us / 1000;

    /* Check if timeout has expired */
    if (elapsed_ms >= ctx->timeout_ms) {
        LOG_ERROR("DEADLOCK DETECTED in pipeline state change: "
                  "Transition to %s took %llu ms (timeout: %u ms)",
                  gst_element_state_get_name(ctx->target_state), (unsigned long long) elapsed_ms,
                  ctx->timeout_ms);

        /* Log error to application */
        app_log_error(APP_ERROR_PIPELINE_STATE_CHANGE_FAILED,
                      "Pipeline deadlock detected during state change to %s after %llu ms",
                      gst_element_state_get_name(ctx->target_state),
                      (unsigned long long) elapsed_ms);

        /* Record error information */
        GStreamerErrorInfo error_info;
        error_info.category = GSTREAMER_ERROR_DEADLOCK_DETECTED;
        error_info.message = "Deadlock detected: state change timeout";
        error_info.debug_info = NULL;
        error_info.source_element = GST_ELEMENT_NAME(ctx->pipeline);
        error_info.failed_state = ctx->target_state;
        error_info.timestamp_us = timing_get_time_us();
        record_error_info(error_info.category, error_info.message, error_info.debug_info,
                          error_info.source_element, error_info.failed_state);

        /* Notify application of error */
        dispatch_error_callback(&error_info, ctx->pipeline);

        /* Attempt recovery */
        LOG_INFO("Attempting deadlock recovery: reverting to READY state");
        gboolean recovery_success = gstreamer_error_handler_attempt_recovery(
            ctx->pipeline, ctx->target_state, ctx->previous_state);

        if (recovery_success) {
            LOG_INFO("Deadlock recovery succeeded");
            dispatch_recovery_callback("Reverted to READY state after timeout", TRUE);
        } else {
            LOG_ERROR("Deadlock recovery failed - pipeline may be unrecoverable");
            dispatch_recovery_callback("Failed to recover from deadlock", FALSE);
        }

        /* Mark detection as inactive and clean up */
        ctx->detection_active = FALSE;
        ctx->timer_source_id = 0;
        return FALSE; /* Remove this timer */
    }

    /* Check again in 100ms */
    return TRUE;
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

gboolean gstreamer_error_handler_init(void)
{
    if (g_error_handler_state.initialized) {
        LOG_WARNING("GStreamer error handler already initialized");
        return TRUE;
    }

    /* Create hash table for deadlock detection contexts */
    g_error_handler_state.deadlock_contexts = g_hash_table_new(g_direct_hash, g_direct_equal);
    if (!g_error_handler_state.deadlock_contexts) {
        LOG_ERROR("Failed to create deadlock detection hash table");
        return FALSE;
    }

    /* Initialize error info to zeros */
    memset(&g_error_handler_state.last_error, 0, sizeof(GStreamerErrorInfo));

    g_error_handler_state.initialized = TRUE;
    LOG_INFO("GStreamer error handler initialized");
    return TRUE;
}

void gstreamer_error_handler_cleanup(void)
{
    if (!g_error_handler_state.initialized) {
        return;
    }

    /* Clean up all deadlock detection contexts */
    if (g_error_handler_state.deadlock_contexts) {
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init(&iter, g_error_handler_state.deadlock_contexts);

        while (g_hash_table_iter_next(&iter, &key, &value)) {
            DeadlockDetectionContext *ctx = (DeadlockDetectionContext *) value;
            if (ctx) {
                if (ctx->timer_source_id > 0) {
                    g_source_remove(ctx->timer_source_id);
                }
                g_free(ctx);
            }
        }

        g_hash_table_destroy(g_error_handler_state.deadlock_contexts);
        g_error_handler_state.deadlock_contexts = NULL;
    }

    /* Clear callbacks */
    g_error_handler_state.error_callback = NULL;
    g_error_handler_state.error_callback_data = NULL;
    g_error_handler_state.recovery_callback = NULL;
    g_error_handler_state.recovery_callback_data = NULL;

    g_error_handler_state.initialized = FALSE;
    LOG_INFO("GStreamer error handler cleaned up");
}

void gstreamer_error_handler_register_error_callback(GStreamerErrorCallback callback,
                                                     gpointer user_data)
{
    g_error_handler_state.error_callback = callback;
    g_error_handler_state.error_callback_data = user_data;
    LOG_DEBUG("GStreamer error callback %s", callback ? "registered" : "unregistered");
}

void gstreamer_error_handler_register_recovery_callback(GStreamerRecoveryCallback callback,
                                                        gpointer user_data)
{
    g_error_handler_state.recovery_callback = callback;
    g_error_handler_state.recovery_callback_data = user_data;
    LOG_DEBUG("GStreamer recovery callback %s", callback ? "registered" : "unregistered");
}

gboolean gstreamer_error_handler_enable_deadlock_detection(GstElement *pipeline, guint timeout_ms)
{
    if (!pipeline) {
        LOG_ERROR("Invalid pipeline for deadlock detection");
        return FALSE;
    }

    if (!g_error_handler_state.initialized) {
        LOG_ERROR("Error handler not initialized");
        return FALSE;
    }

    if (timeout_ms == 0) {
        timeout_ms = 10000; /* Default 10 second timeout */
    }

    LOG_DEBUG("Enabling deadlock detection for pipeline (timeout=%u ms)", timeout_ms);

    /* Allocate and initialize context */
    DeadlockDetectionContext *ctx = g_malloc0(sizeof(DeadlockDetectionContext));
    if (!ctx) {
        LOG_ERROR("Failed to allocate deadlock detection context");
        return FALSE;
    }

    ctx->pipeline = pipeline;
    ctx->timeout_ms = timeout_ms;
    ctx->detection_active = FALSE;
    ctx->timer_source_id = 0;

    /* Store in hash table */
    g_hash_table_insert(g_error_handler_state.deadlock_contexts, pipeline, ctx);

    LOG_INFO("Deadlock detection enabled for pipeline %s", GST_ELEMENT_NAME(pipeline));
    return TRUE;
}

void gstreamer_error_handler_disable_deadlock_detection(GstElement *pipeline)
{
    if (!pipeline || !g_error_handler_state.deadlock_contexts) {
        return;
    }

    DeadlockDetectionContext *ctx = (DeadlockDetectionContext *) g_hash_table_lookup(
        g_error_handler_state.deadlock_contexts, pipeline);

    if (ctx) {
        if (ctx->timer_source_id > 0) {
            g_source_remove(ctx->timer_source_id);
            ctx->timer_source_id = 0;
        }

        g_hash_table_remove(g_error_handler_state.deadlock_contexts, pipeline);
        g_free(ctx);
        LOG_DEBUG("Deadlock detection disabled for pipeline");
    }
}

gboolean gstreamer_error_handler_attempt_recovery(GstElement *pipeline, GstState target_state,
                                                  GstState previous_state)
{
    if (!pipeline) {
        LOG_ERROR("Invalid pipeline for recovery");
        return FALSE;
    }

    LOG_INFO("Attempting recovery: reverting from %s to %s",
             gst_element_state_get_name(target_state), gst_element_state_get_name(previous_state));

    /* Strategy 1: Revert to previous state */
    GstStateChangeReturn ret = gst_element_set_state(pipeline, previous_state);
    if (ret != GST_STATE_CHANGE_FAILURE) {
        LOG_INFO("Recovery successful: reverted to %s state",
                 gst_element_state_get_name(previous_state));
        dispatch_recovery_callback("Reverted to previous state", TRUE);
        return TRUE;
    }

    LOG_WARNING("Failed to revert to previous state, attempting READY state");
    dispatch_recovery_callback("Revert failed, forcing READY", FALSE);

    /* Strategy 2: Force READY state */
    ret = gst_element_set_state(pipeline, GST_STATE_READY);
    if (ret != GST_STATE_CHANGE_FAILURE) {
        LOG_INFO("Recovery successful: forced to READY state");
        dispatch_recovery_callback("Forced to READY state", TRUE);
        return TRUE;
    }

    LOG_ERROR("Failed to force READY state, attempting NULL (stop)");
    dispatch_recovery_callback("READY failed, forcing NULL", FALSE);

    /* Strategy 3: Force NULL state (complete stop) */
    ret = gst_element_set_state(pipeline, GST_STATE_NULL);
    if (ret != GST_STATE_CHANGE_FAILURE) {
        LOG_INFO("Recovery successful: forced to NULL state (complete stop)");
        dispatch_recovery_callback("Forced to NULL state", TRUE);
        return TRUE;
    }

    LOG_ERROR("All recovery attempts failed - pipeline is unrecoverable");
    dispatch_recovery_callback("All recovery attempts failed", FALSE);
    return FALSE;
}

const GStreamerErrorInfo *gstreamer_error_handler_get_last_error(void)
{
    if (g_error_handler_state.last_error.message) {
        return &g_error_handler_state.last_error;
    }
    return NULL;
}

void gstreamer_error_handler_clear_last_error(void)
{
    memset(&g_error_handler_state.last_error, 0, sizeof(GStreamerErrorInfo));
    LOG_DEBUG("Last error cleared");
}

const gchar *gstreamer_error_handler_category_to_string(GStreamerErrorCategory category)
{
    switch (category) {
    case GSTREAMER_ERROR_BUS_ERROR:
        return "Bus Error";
    case GSTREAMER_ERROR_STATE_CHANGE_FAILURE:
        return "State Change Failure";
    case GSTREAMER_ERROR_DEADLOCK_DETECTED:
        return "Deadlock Detected";
    case GSTREAMER_ERROR_ELEMENT_MISSING:
        return "Element Missing";
    case GSTREAMER_ERROR_NEGOTIATION:
        return "Caps Negotiation Failure";
    case GSTREAMER_ERROR_RESOURCE:
        return "Resource Exhaustion";
    case GSTREAMER_ERROR_UNKNOWN:
    default:
        return "Unknown Error";
    }
}

gboolean gstreamer_error_handler_set_state_with_detection(GstElement *pipeline,
                                                          GstState target_state, guint timeout_ms)
{
    if (!pipeline) {
        LOG_ERROR("Invalid pipeline for state change");
        return FALSE;
    }

    if (timeout_ms == 0) {
        timeout_ms = 10000; /* Default 10 second timeout */
    }

    /* Get current state before change */
    GstState current_state;
    gst_element_get_state(pipeline, &current_state, NULL, 0);

    LOG_DEBUG("State change: %s → %s (timeout=%u ms)", gst_element_state_get_name(current_state),
              gst_element_state_get_name(target_state), timeout_ms);

    /* If no deadlock detection configured, just do state change */
    DeadlockDetectionContext *ctx = NULL;
    if (g_error_handler_state.deadlock_contexts) {
        ctx = (DeadlockDetectionContext *) g_hash_table_lookup(
            g_error_handler_state.deadlock_contexts, pipeline);
    }

    if (ctx) {
        /* Set up deadlock detection */
        ctx->target_state = target_state;
        ctx->previous_state = current_state;
        ctx->start_time_us = timing_get_time_us();
        ctx->detection_active = TRUE;

        /* Remove any existing timer */
        if (ctx->timer_source_id > 0) {
            g_source_remove(ctx->timer_source_id);
        }

        /* Start new timer for deadlock detection */
        ctx->timer_source_id = g_timeout_add(100, deadlock_detection_timeout, ctx);

        LOG_DEBUG("Deadlock detection timer started (100ms check interval)");
    }

    /* Perform state change */
    GstStateChangeReturn ret = gst_element_set_state(pipeline, target_state);

    /* If we set up deadlock detection, handle cleanup of timer on success */
    if (ctx) {
        if (ret != GST_STATE_CHANGE_FAILURE) {
            /* State change succeeded or is async, cancel deadlock detection */
            if (ctx->timer_source_id > 0) {
                g_source_remove(ctx->timer_source_id);
                ctx->timer_source_id = 0;
            }
            ctx->detection_active = FALSE;
        }
    }

    return (ret != GST_STATE_CHANGE_FAILURE);
}
