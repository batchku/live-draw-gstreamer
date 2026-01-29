/**
 * @file cleanup_handlers.c
 * @brief Cleanup and atexit handlers implementation
 *
 * Implements cleanup routines for GStreamer pipeline, window, camera,
 * and memory deallocation. Provides atexit handlers to ensure cleanup
 * occurs even during abnormal termination.
 */

#include "cleanup_handlers.h"
#include "../app/e2e_coordinator.h"
#include "../camera/camera_source.h"
#include "../gstreamer/pipeline_builder.h"
#include "../input/keyboard_handler.h"
#include "../osx/window.h"
#include "../recording/recording_state.h"
#include "../utils/logging.h"
#include "../utils/memory.h"
#include "app_context.h"
#include "app_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Maximum number of custom cleanup callbacks to support */
#define MAX_CLEANUP_CALLBACKS 16

/**
 * @struct CleanupHandlerState
 * @brief Internal state for cleanup handler management
 */
typedef struct {
    /* Array of registered cleanup callbacks */
    void (*callbacks[MAX_CLEANUP_CALLBACKS])(void);

    /* Number of registered callbacks */
    guint callback_count;

    /* Flag indicating if cleanup has been executed */
    gboolean has_executed;

    /* Mutex to protect concurrent access */
    GMutex *mutex;
} CleanupHandlerState;

/* Global cleanup handler state */
static CleanupHandlerState g_cleanup_state = {
    .callback_count = 0, .has_executed = FALSE, .mutex = NULL};

/**
 * Initialize the cleanup state mutex (must be called before use)
 */
static void cleanup_handlers_init_mutex(void)
{
    if (g_cleanup_state.mutex == NULL) {
        g_cleanup_state.mutex = g_new(GMutex, 1);
        g_mutex_init(g_cleanup_state.mutex);
    }
}

/**
 * Internal cleanup function for GStreamer pipeline
 *
 * Performs safe cleanup of the GStreamer pipeline, stopping playback
 * and freeing all GStreamer resources.
 *
 * @param pipeline Pipeline structure to cleanup
 */
static void cleanup_pipeline(Pipeline *pipeline)
{
    if (!pipeline || !pipeline->pipeline) {
        return;
    }

    LOG_DEBUG("Cleanup: Stopping GStreamer pipeline");

    /* Set pipeline to NULL state (stops playback and frees resources) */
    GstStateChangeReturn ret = gst_element_set_state(pipeline->pipeline, GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        LOG_WARNING("Cleanup: Failed to set pipeline to NULL state");
    }

    /* Unref pipeline element */
    if (G_IS_OBJECT(pipeline->pipeline)) {
        gst_object_unref(pipeline->pipeline);
    }

    LOG_INFO("Cleanup: GStreamer pipeline cleaned up");
}

/**
 * Internal cleanup function for macOS window
 *
 * Safely closes the Cocoa window and releases all window resources.
 *
 * @param window OSXWindow structure to cleanup
 */
static void cleanup_window(OSXWindow *window)
{
    if (!window) {
        return;
    }

    LOG_DEBUG("Cleanup: Closing OS X window");

    window_cleanup(window);

    LOG_INFO("Cleanup: OS X window cleaned up");
}

/**
 * Internal cleanup function for camera
 *
 * Safely disconnects from the camera and releases AVFoundation resources.
 *
 * @param camera CameraSource structure to cleanup
 */
static void cleanup_camera(CameraSource *camera)
{
    if (!camera) {
        return;
    }

    LOG_DEBUG("Cleanup: Disconnecting camera");

    camera_source_cleanup(camera);

    LOG_INFO("Cleanup: Camera cleaned up");
}

/**
 * Internal cleanup function for recording state
 *
 * Safely frees the recording state structure.
 *
 * @param state RecordingState structure to cleanup
 */
static void cleanup_recording_state(RecordingState *state)
{
    if (!state) {
        return;
    }

    LOG_DEBUG("Cleanup: Releasing recording state");

    recording_state_cleanup(state);

    LOG_INFO("Cleanup: Recording state cleaned up");
}

/**
 * Internal cleanup function for E2E coordinator
 *
 * Safely frees E2E coordinator and associated playback loops/buffers.
 */
static void cleanup_e2e_coordinator(void)
{
    LOG_DEBUG("Cleanup: Releasing E2E coordinator");

    e2e_coordinator_cleanup();

    LOG_INFO("Cleanup: E2E coordinator cleaned up");
}

/**
 * Internal cleanup function for keyboard handler
 *
 * Safely stops capturing keyboard input.
 */
static void cleanup_keyboard(void)
{
    LOG_DEBUG("Cleanup: Releasing keyboard handler");

    keyboard_cleanup();

    LOG_INFO("Cleanup: Keyboard handler cleaned up");
}

/**
 * Internal cleanup function for application context
 *
 * Safely frees the main application context structure.
 *
 * @param ctx AppContext structure to cleanup
 */
static void cleanup_app_context(AppContext *ctx)
{
    if (!ctx) {
        return;
    }

    LOG_DEBUG("Cleanup: Releasing application context");

    /* Free main loop if it exists */
    if (ctx->main_loop) {
        g_main_loop_unref(ctx->main_loop);
        ctx->main_loop = NULL;
    }

    /* Free application context */
    app_context_cleanup(ctx);

    LOG_INFO("Cleanup: Application context cleaned up");
}

/**
 * Internal cleanup function for utilities (logging, memory tracking)
 *
 * Safely deinitializes logging and memory tracking utilities.
 */
static void cleanup_utilities(void)
{
    LOG_DEBUG("Cleanup: Releasing utilities (memory, logging)");

    mem_cleanup();
    logging_cleanup();

    LOG_INFO("Cleanup: Utilities cleaned up");
}

/**
 * Execute all registered cleanup handlers in LIFO order
 *
 * Internal implementation that executes custom callbacks and standard cleanup.
 */
static void execute_cleanup_internal(void)
{
    cleanup_handlers_init_mutex();
    g_mutex_lock(g_cleanup_state.mutex);

    /* Guard against multiple executions */
    if (g_cleanup_state.has_executed) {
        g_mutex_unlock(g_cleanup_state.mutex);
        return;
    }

    g_cleanup_state.has_executed = TRUE;
    g_mutex_unlock(g_cleanup_state.mutex);

    LOG_INFO("============================================");
    LOG_INFO("Executing cleanup handlers");
    LOG_INFO("============================================");

    /* Execute custom cleanup callbacks in LIFO order */
    g_mutex_lock(g_cleanup_state.mutex);
    for (gint i = (gint) g_cleanup_state.callback_count - 1; i >= 0; --i) {
        if (g_cleanup_state.callbacks[i]) {
            LOG_DEBUG("Executing custom cleanup callback %d/%u", g_cleanup_state.callback_count - i,
                      g_cleanup_state.callback_count);
            g_cleanup_state.callbacks[i]();
        }
    }
    g_mutex_unlock(g_cleanup_state.mutex);

    /* Get application context and perform standard cleanup */
    AppContext *app_ctx = app_context_get();

    /* Cleanup in reverse initialization order */
    if (app_ctx) {
        /* Cleanup E2E coordinator (frees recording buffers and playback loops) */
        cleanup_e2e_coordinator();

        /* Cleanup keyboard handler (stops capturing input) */
        cleanup_keyboard();

        /* Cleanup recording state */
        if (app_ctx->recording_state) {
            cleanup_recording_state(app_ctx->recording_state);
            app_ctx->recording_state = NULL;
        }

        /* Cleanup pipeline */
        if (app_ctx->gst_pipeline) {
            cleanup_pipeline(app_ctx->gst_pipeline);
            app_ctx->gst_pipeline = NULL;
            app_ctx->pipeline = NULL;
        }

        /* Cleanup window */
        if (app_ctx->window) {
            cleanup_window(app_ctx->window);
            app_ctx->window = NULL;
        }

        /* Cleanup camera */
        if (app_ctx->camera) {
            cleanup_camera(app_ctx->camera);
            app_ctx->camera = NULL;
        }

        /* Cleanup application context */
        cleanup_app_context(app_ctx);
    }

    /* Deinitialize GStreamer before utilities */
    LOG_DEBUG("Cleanup: Deinitializing GStreamer");
    gst_deinit();

    /* Cleanup utilities (memory tracking, logging) */
    cleanup_utilities();

    LOG_INFO("============================================");
    LOG_INFO("Cleanup handlers completed");
    LOG_INFO("============================================");
}

/**
 * atexit handler - called automatically during program termination
 *
 * This handler is registered with atexit() and is called automatically
 * when the program exits, even in abnormal termination scenarios
 * (e.g., unhandled signals, abort(), exit()).
 *
 * It ensures that GStreamer pipelines are properly stopped and all
 * resources are released.
 */
static void on_atexit_handler(void)
{
    /* Suppress logging output in atexit handler to avoid issues */
    fprintf(stderr, "\n[atexit] Video Looper atexit handler executing\n");
    execute_cleanup_internal();
    fprintf(stderr, "[atexit] Video Looper cleanup complete\n");
}

/**
 * Initialize cleanup handlers and register atexit handler
 *
 * @return TRUE on success, FALSE on failure
 */
gboolean cleanup_handlers_init(void)
{
    cleanup_handlers_init_mutex();
    g_mutex_lock(g_cleanup_state.mutex);

    /* Register atexit handler */
    if (atexit(on_atexit_handler) != 0) {
        LOG_ERROR("Failed to register atexit handler");
        g_mutex_unlock(g_cleanup_state.mutex);
        return FALSE;
    }

    LOG_INFO("Cleanup handlers initialized");
    LOG_DEBUG("atexit handler registered for graceful shutdown");

    g_mutex_unlock(g_cleanup_state.mutex);
    return TRUE;
}

/**
 * Register a custom cleanup callback
 *
 * @param callback Function to call during cleanup
 * @return TRUE on success, FALSE on failure
 */
gboolean cleanup_handlers_register_callback(void (*callback)(void))
{
    if (!callback) {
        LOG_WARNING("cleanup_handlers_register_callback: NULL callback pointer");
        return FALSE;
    }

    cleanup_handlers_init_mutex();
    g_mutex_lock(g_cleanup_state.mutex);

    /* Check if callback is already registered */
    for (guint i = 0; i < g_cleanup_state.callback_count; ++i) {
        if (g_cleanup_state.callbacks[i] == callback) {
            LOG_WARNING("Cleanup callback already registered");
            g_mutex_unlock(g_cleanup_state.mutex);
            return FALSE;
        }
    }

    /* Check if there's space for another callback */
    if (g_cleanup_state.callback_count >= MAX_CLEANUP_CALLBACKS) {
        LOG_WARNING("Maximum number of cleanup callbacks reached (%d)", MAX_CLEANUP_CALLBACKS);
        g_mutex_unlock(g_cleanup_state.mutex);
        return FALSE;
    }

    /* Register the callback */
    g_cleanup_state.callbacks[g_cleanup_state.callback_count] = callback;
    g_cleanup_state.callback_count++;

    LOG_DEBUG("Cleanup callback registered (%u/%d callbacks registered)",
              g_cleanup_state.callback_count, MAX_CLEANUP_CALLBACKS);

    g_mutex_unlock(g_cleanup_state.mutex);
    return TRUE;
}

/**
 * Unregister a cleanup callback
 *
 * @param callback Function to unregister
 * @return TRUE if found and removed, FALSE otherwise
 */
gboolean cleanup_handlers_unregister_callback(void (*callback)(void))
{
    if (!callback) {
        return FALSE;
    }

    cleanup_handlers_init_mutex();
    g_mutex_lock(g_cleanup_state.mutex);

    for (guint i = 0; i < g_cleanup_state.callback_count; ++i) {
        if (g_cleanup_state.callbacks[i] == callback) {
            /* Shift remaining callbacks down */
            for (guint j = i; j < g_cleanup_state.callback_count - 1; ++j) {
                g_cleanup_state.callbacks[j] = g_cleanup_state.callbacks[j + 1];
            }
            g_cleanup_state.callbacks[g_cleanup_state.callback_count - 1] = NULL;
            g_cleanup_state.callback_count--;

            LOG_DEBUG("Cleanup callback unregistered (%u callbacks remain)",
                      g_cleanup_state.callback_count);

            g_mutex_unlock(g_cleanup_state.mutex);
            return TRUE;
        }
    }

    g_mutex_unlock(g_cleanup_state.mutex);
    return FALSE;
}

/**
 * Execute all cleanup handlers
 */
void cleanup_handlers_execute(void)
{
    execute_cleanup_internal();
}

/**
 * Check if cleanup has been executed
 *
 * @return TRUE if cleanup has already been executed, FALSE otherwise
 */
gboolean cleanup_handlers_has_executed(void)
{
    cleanup_handlers_init_mutex();
    g_mutex_lock(g_cleanup_state.mutex);
    gboolean result = g_cleanup_state.has_executed;
    g_mutex_unlock(g_cleanup_state.mutex);
    return result;
}
