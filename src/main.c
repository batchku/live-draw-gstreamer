/**
 * @file main.c
 * @brief Video Looper application entry point
 *
 * This is the main entry point for the Video Looper application.
 * It orchestrates the initialization of all components and runs the
 * main event loop.
 *
 * The initialization sequence is:
 * 1. Initialize logging and memory tracking
 * 2. Initialize GStreamer library
 * 3. Create application context
 * 4. Install signal handlers
 * 5. Create GLib main loop
 * 6. Initialize components (camera, window, pipeline, keyboard, recording, playback)
 * 7. Run main event loop
 * 8. Cleanup all components and resources
 *
 * Error Handling:
 * - All initialization errors are logged and cause graceful shutdown
 * - Signal handlers (SIGINT, SIGTERM) trigger graceful shutdown
 * - Application state is cleaned up in reverse initialization order
 */

#include <gst/gst.h>
#include <gst/gstmacos.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "app/app_context.h"
#include "app/app_error.h"
#include "app/cleanup_handlers.h"
#include "app/e2e_coordinator.h"
#include "app/error_dialog.h"
#include "camera/camera_source.h"
#include "gstreamer/pipeline_builder.h"
#include "input/keyboard_handler.h"
#include "osx/window.h"
#include "recording/recording_state.h"
#include "utils/logging.h"
#include "utils/memory.h"

/* Global flag for signal handling */
static volatile sig_atomic_t shutdown_requested = 0;

/* Forward declarations of static functions */
static void on_app_error(AppError *error, gpointer user_data);
static gboolean initialize_app_context(AppContext **app_ctx);
static gboolean setup_event_loop(AppContext *app_ctx);
static void install_signal_handlers(void);
static gboolean initialize_components(AppContext *app_ctx);
static void cleanup_components(AppContext *app_ctx);


/**
 * Global error handler callback
 *
 * Dispatches fatal errors to user-facing error dialogs and logs the error.
 * Called when a fatal error occurs during initialization or runtime.
 *
 * @param error Error structure containing code and message
 * @param user_data Optional user data (unused)
 */
static void on_app_error(AppError *error, gpointer user_data G_GNUC_UNUSED)
{
    if (!error) {
        return;
    }

    LOG_ERROR("Fatal error (code=%d): %s", error->code, error->message);

    /* Display user-friendly error dialogs based on error code */
    switch (error->code) {
    case APP_ERROR_CAMERA_NOT_FOUND:
        error_dialog_show_camera_not_found();
        break;
    case APP_ERROR_CAMERA_PERMISSION_DENIED:
        error_dialog_show_camera_permission_denied();
        break;
    case APP_ERROR_GSTREAMER_INIT_FAILED:
        error_dialog_show_gstreamer_init_failed(error->message);
        break;
    case APP_ERROR_WINDOW_CREATE_FAILED:
        error_dialog_show_generic("Window Creation Failed", "Failed to create application window. "
                                                            "Please check your display settings.");
        break;
    case APP_ERROR_PIPELINE_BUILD_FAILED:
        error_dialog_show_generic("Pipeline Build Failed", "Failed to build GStreamer pipeline. "
                                                           "Check GStreamer installation.");
        break;
    case APP_ERROR_PIPELINE_STATE_CHANGE_FAILED:
        error_dialog_show_generic("Pipeline Error", "Failed to set video pipeline state. "
                                                    "Try restarting the application.");
        break;
    case APP_ERROR_KEYBOARD_HANDLER_FAILED:
        error_dialog_show_generic("Keyboard Error", "Failed to initialize keyboard input. "
                                                    "Try restarting the application.");
        break;
    case APP_ERROR_MEMORY_ALLOCATION_FAILED:
        error_dialog_show_generic("Memory Error", "Insufficient memory to run Video Looper. "
                                                  "Please close other applications.");
        break;
    default:
        error_dialog_show_generic("Error", error->message);
        break;
    }
}

/**
 * Signal handler for SIGINT (Ctrl+C) and SIGTERM
 *
 * @param signal Signal number
 */
static void on_signal_interrupt(int signal)
{
    if (!shutdown_requested) {
        LOG_INFO("Signal %d received, initiating graceful shutdown", signal);
        shutdown_requested = 1;

        AppContext *ctx = app_context_get();
        if (ctx && ctx->main_loop) {
            g_main_loop_quit(ctx->main_loop);
        }
    }
}

/**
 * Initialize GStreamer and related systems
 *
 * @param argc Command line argument count
 * @param argv Command line arguments
 * @return TRUE on success, FALSE on failure
 */
static gboolean initialize_gstreamer(int *argc, char **argv[])
{
    LOG_DEBUG("Initializing GStreamer...");

    GError *gst_error = NULL;
    if (!gst_init_check(argc, argv, &gst_error)) {
        LOG_ERROR("GStreamer initialization failed: %s",
                  gst_error ? gst_error->message : "Unknown error");
        if (gst_error) {
            g_error_free(gst_error);
        }
        app_log_error(APP_ERROR_GSTREAMER_INIT_FAILED, "Failed to initialize GStreamer library");
        return FALSE;
    }

    LOG_INFO("GStreamer initialized successfully");
    LOG_DEBUG("GStreamer version: %s", gst_version_string());

    return TRUE;
}

/**
 * Initialize application context
 *
 * @param app_ctx Pointer to store application context
 * @return TRUE on success, FALSE on failure
 */
static gboolean initialize_app_context(AppContext **app_ctx)
{
    LOG_DEBUG("Creating application context...");

    *app_ctx = app_context_create();
    if (!*app_ctx) {
        LOG_ERROR("Failed to create application context");
        app_log_error(APP_ERROR_GSTREAMER_INIT_FAILED, "Failed to allocate application context");
        return FALSE;
    }

    app_context_set(*app_ctx);
    LOG_INFO("Application context created");

    return TRUE;
}

/**
 * Setup the GLib main event loop
 *
 * @param app_ctx Application context
 * @return TRUE on success, FALSE on failure
 */
static gboolean setup_event_loop(AppContext *app_ctx)
{
    LOG_DEBUG("Creating GLib main event loop...");

    app_ctx->main_loop = g_main_loop_new(NULL, FALSE);
    if (!app_ctx->main_loop) {
        LOG_ERROR("Failed to create GLib main loop");
        app_log_error(APP_ERROR_GSTREAMER_INIT_FAILED, "Failed to create main event loop");
        return FALSE;
    }

    LOG_INFO("Main event loop created successfully");

    return TRUE;
}

/**
 * Install signal handlers for graceful shutdown
 *
 * Registers handlers for SIGINT (Ctrl+C) and SIGTERM to allow graceful
 * shutdown of the application.
 */
static void install_signal_handlers(void)
{
    LOG_DEBUG("Installing signal handlers...");

    signal(SIGINT, on_signal_interrupt);
    signal(SIGTERM, on_signal_interrupt);

    LOG_INFO("Signal handlers installed");
}

/**
 * Initialize component: camera source
 *
 * Initializes the camera source with macOS AVFoundation integration,
 * requests camera permissions, and negotiates camera format.
 *
 * @param app_ctx Application context
 * @return TRUE on success, FALSE on failure
 */
static gboolean initialize_camera(AppContext *app_ctx)
{
    LOG_DEBUG("Initializing camera source...");

    /* Check camera permission */
    CameraPermissionStatus perm_status = camera_request_permission();
    if (perm_status == CAMERA_PERMISSION_DENIED) {
        LOG_ERROR("Camera permission denied by user");
        app_log_error(APP_ERROR_CAMERA_PERMISSION_DENIED, "User denied camera access permission");
        return FALSE;
    }

    if (perm_status == CAMERA_PERMISSION_NOT_DETERMINED) {
        LOG_DEBUG("Camera permission status not yet determined, waiting for user");
    }

    /* Initialize camera with format negotiation */
    CameraSource *camera = camera_source_init();
    if (!camera) {
        LOG_ERROR("Failed to initialize camera source");
        app_log_error(APP_ERROR_CAMERA_NOT_FOUND,
                      "Could not initialize camera - device may not be available");
        return FALSE;
    }

    /* Store camera in application context */
    app_ctx->camera = camera;

    /* Store camera dimensions for later use */
    app_ctx->camera_width = camera->width;
    app_ctx->camera_height = camera->height;
    if (camera->height > 0) {
        app_ctx->aspect_ratio = (gdouble) camera->width / (gdouble) camera->height;
    } else {
        app_ctx->aspect_ratio = 16.0 / 9.0; /* Default to 16:9 */
    }

    LOG_INFO("Camera source initialized successfully (%dx%d @ %d fps, aspect ratio %.2f)",
             camera->width, camera->height, camera->framerate, app_ctx->aspect_ratio);

    return TRUE;
}

/**
 * Initialize component: OS X window
 *
 * Creates native Cocoa NSWindow with video rendering view and
 * integrates osxvideosink for GStreamer video output.
 *
 * @param app_ctx Application context
 * @return TRUE on success, FALSE on failure
 */
static gboolean initialize_window(AppContext *app_ctx)
{
    LOG_DEBUG("Initializing OS X window...");

    /* Create window with 10-cell grid layout */
    OSXWindow *window = window_create(10); /* 10 cells for 10x1 grid */
    if (!window) {
        LOG_ERROR("Failed to create OS X window");
        app_log_error(APP_ERROR_WINDOW_CREATE_FAILED, "Could not create application window");
        return FALSE;
    }

    /* Set aspect ratio from camera */
    window_set_aspect_ratio(window, app_ctx->aspect_ratio);

    /* Store window reference and videosink */
    app_ctx->window = window;

    LOG_INFO("OS X window initialized successfully (3200×%d pixels, 10-cell grid)",
             (int) (320 / app_ctx->aspect_ratio));

    return TRUE;
}

/**
 * Initialize component: GStreamer pipeline
 *
 * Constructs the main GStreamer pipeline with camera source, live feed,
 * recording bins, videomixer compositor, and window rendering.
 *
 * @param app_ctx Application context
 * @return TRUE on success, FALSE on failure
 */
static gboolean initialize_pipeline(AppContext *app_ctx)
{
    LOG_DEBUG("Initializing GStreamer pipeline...");

    /* Verify camera was initialized */
    if (!app_ctx->camera) {
        LOG_ERROR("Cannot initialize pipeline: camera not initialized");
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED,
                      "Camera source must be initialized before pipeline");
        return FALSE;
    }

    /* Get camera source element */
    GstElement *camera_element = camera_source_create_element(app_ctx->camera);
    if (!camera_element) {
        LOG_ERROR("Failed to create camera GStreamer element");
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED, "Could not create camera GStreamer element");
        return FALSE;
    }

    /* Create main pipeline */
    Pipeline *pipeline = pipeline_create(camera_element);
    if (!pipeline) {
        LOG_ERROR("Failed to create GStreamer pipeline");
        gst_object_unref(camera_element);
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED, "Could not construct main video pipeline");
        return FALSE;
    }

    /* Associate window with pipeline for video overlay */
    pipeline_set_window(pipeline, app_ctx->window);

    /* Get osxvideosink from window and integrate with pipeline */
    GstElement *videosink = window_get_videosink(app_ctx->window);
    if (!videosink) {
        LOG_ERROR("Failed to get osxvideosink from window");
        pipeline_cleanup(pipeline);
        app_log_error(APP_ERROR_WINDOW_CREATE_FAILED, "Window does not have valid video sink");
        return FALSE;
    }

    /* Replace stub osxvideosink with real one from window */
    if (pipeline->osxvideosink && GST_IS_ELEMENT(pipeline->osxvideosink)) {
        /* Set pipeline to NULL state before making changes */
        gst_element_set_state(pipeline->pipeline, GST_STATE_NULL);

        /* Unlink composite_caps from old osxvideosink */
        gst_element_unlink(pipeline->composite_caps, pipeline->osxvideosink);

        /* Set old element to NULL state before removal */
        gst_element_set_state(pipeline->osxvideosink, GST_STATE_NULL);

        /* Remove old osxvideosink from bin */
        gst_bin_remove(GST_BIN(pipeline->pipeline), pipeline->osxvideosink);

        /* Don't unref here - the removal already handled it */
    }

    /* Set new videosink and add to pipeline */
    pipeline->osxvideosink = videosink;
    gst_object_ref(videosink);
    gst_bin_add(GST_BIN(pipeline->pipeline), videosink);

    /* Link composite_caps to osxvideosink */
    if (!gst_element_link(pipeline->composite_caps, videosink)) {
        LOG_ERROR("Failed to link composite_caps to osxvideosink");
        pipeline_cleanup(pipeline);
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED,
                      "Could not link video composition to output");
        return FALSE;
    }

    /* Set pipeline to READY state for operation */
    if (!pipeline_set_state(pipeline, GST_STATE_READY)) {
        LOG_ERROR("Failed to set pipeline to READY state");
        pipeline_cleanup(pipeline);
        app_log_error(APP_ERROR_PIPELINE_STATE_CHANGE_FAILED,
                      "Pipeline could not transition to READY state");
        return FALSE;
    }

    /* Store pipeline reference */
    app_ctx->gst_pipeline = pipeline;
    app_ctx->pipeline = pipeline->pipeline;

    LOG_INFO("GStreamer pipeline initialized successfully");

    return TRUE;
}

/**
 * Initialize component: keyboard input handler
 *
 * Initializes the keyboard input handler and registers the E2E coordinator
 * as the callback for keyboard events. This connects keyboard input to
 * the recording→playback→display flow.
 *
 * @param app_ctx Application context
 * @return TRUE on success, FALSE on failure
 */
static gboolean initialize_keyboard(AppContext *app_ctx G_GNUC_UNUSED)
{
    LOG_DEBUG("Initializing keyboard input handler...");

    /* Initialize keyboard handler with E2E coordinator as callback */
    keyboard_init((KeyEventCallback) e2e_on_key_event);

    LOG_INFO("Keyboard input handler initialized");

    return TRUE;
}

/**
 * Initialize component: recording state manager
 *
 * Initializes the recording state manager which tracks keyboard input
 * and manages recording state for all 9 keys. The recording state is used
 * by the E2E coordinator to control the record bins.
 *
 * @param app_ctx Application context
 * @return TRUE on success, FALSE on failure
 */
static gboolean initialize_recording_state(AppContext *app_ctx)
{
    LOG_DEBUG("Initializing recording state manager...");

    app_ctx->recording_state = recording_state_init();
    if (!app_ctx->recording_state) {
        LOG_ERROR("Failed to initialize recording state");
        return FALSE;
    }

    LOG_INFO("Recording state manager initialized");

    return TRUE;
}

/**
 * Initialize component: E2E coordinator
 *
 * Initializes the end-to-end coordinator that manages the complete flow:
 * keyboard → recording_state → record_bin → buffer → playback_bin → videomixer → display
 *
 * This must be called after all other components (pipeline, recording_state) are initialized.
 *
 * @param app_ctx Application context
 * @return TRUE on success, FALSE on failure
 */
static gboolean initialize_e2e_coordinator(AppContext *app_ctx)
{
    LOG_DEBUG("Initializing E2E coordinator...");

    if (!e2e_coordinator_init(app_ctx)) {
        LOG_ERROR("Failed to initialize E2E coordinator");
        return FALSE;
    }

    LOG_INFO("E2E coordinator initialized");

    return TRUE;
}

/**
 * Initialize all application components
 *
 * Orchestrates initialization of all major components in the correct order:
 * 1. Camera (provides aspect ratio for window sizing)
 * 2. Window (provides video sink for pipeline)
 * 3. Pipeline (orchestrates video processing)
 * 4. Recording state (translates keyboard to recording commands)
 * 5. E2E coordinator (manages recording→playback→display flow)
 * 6. Keyboard handler (captures user input and routes to E2E coordinator)
 *
 * @param app_ctx Application context
 * @return TRUE if all components initialized, FALSE if any failed
 */
static gboolean initialize_components(AppContext *app_ctx)
{
    LOG_INFO("Beginning component initialization sequence...");

    if (!initialize_camera(app_ctx)) {
        LOG_ERROR("Camera initialization failed");
        return FALSE;
    }

    if (!initialize_window(app_ctx)) {
        LOG_ERROR("Window initialization failed");
        return FALSE;
    }

    if (!initialize_pipeline(app_ctx)) {
        LOG_ERROR("Pipeline initialization failed");
        return FALSE;
    }

    if (!initialize_recording_state(app_ctx)) {
        LOG_ERROR("Recording state initialization failed");
        return FALSE;
    }

    if (!initialize_e2e_coordinator(app_ctx)) {
        LOG_ERROR("E2E coordinator initialization failed");
        return FALSE;
    }

    if (!initialize_keyboard(app_ctx)) {
        LOG_ERROR("Keyboard initialization failed");
        return FALSE;
    }

    LOG_INFO("All components initialized successfully");

    return TRUE;
}

/**
 * Cleanup all application components
 *
 * Performs cleanup in reverse initialization order to ensure proper
 * dependency unwinding.
 *
 * Cleanup order (reverse of initialization):
 * 1. E2E coordinator (frees recording buffers and playback loops)
 * 2. Keyboard handler (stops capturing input)
 * 3. Recording state (frees state structure)
 * 4. Pipeline (stops GStreamer pipeline and frees elements)
 * 5. Window (closes Cocoa window)
 * 6. Camera (disconnects from camera)
 *
 * @param app_ctx Application context
 */
static void cleanup_components(AppContext *app_ctx)
{
    if (!app_ctx) {
        return;
    }

    LOG_INFO("Beginning component cleanup sequence...");

    /* Cleanup E2E coordinator (frees recording buffers and playback loops) */
    LOG_DEBUG("Cleaning up E2E coordinator...");
    e2e_coordinator_cleanup();

    /* Cleanup keyboard handler (stops capturing input) */
    LOG_DEBUG("Cleaning up keyboard handler...");
    keyboard_cleanup();

    /* Cleanup recording state */
    if (app_ctx->recording_state) {
        LOG_DEBUG("Cleaning up recording state...");
        recording_state_cleanup(app_ctx->recording_state);
        app_ctx->recording_state = NULL;
    }

    /* Cleanup pipeline */
    if (app_ctx->gst_pipeline) {
        LOG_DEBUG("Cleaning up GStreamer pipeline...");
        pipeline_cleanup(app_ctx->gst_pipeline);
        app_ctx->gst_pipeline = NULL;
        app_ctx->pipeline = NULL;
    }

    /* Cleanup window */
    if (app_ctx->window) {
        LOG_DEBUG("Cleaning up OS X window...");
        window_cleanup(app_ctx->window);
        app_ctx->window = NULL;
    }

    /* Cleanup camera */
    if (app_ctx->camera) {
        LOG_DEBUG("Cleaning up camera source...");
        camera_source_cleanup(app_ctx->camera);
        app_ctx->camera = NULL;
    }

    LOG_INFO("Component cleanup complete");
}

/**
 * Application main function (runs on secondary thread via gst_macos_main)
 *
 * This function contains the actual application logic. It's called by
 * gst_macos_main() on a secondary thread while NSApplication/NSRunLoop
 * runs on the main thread, enabling proper osxvideosink rendering.
 *
 * @param argc Command line argument count
 * @param argv Command line arguments
 * @param user_data User data (unused)
 * @return Exit code (0 for success, non-zero for error)
 */
static int app_main(int argc, char *argv[], gpointer user_data)
{
    (void)user_data;
    /* Print startup banner */
    fprintf(stdout, "Video Looper v1.0.0\n");
    fprintf(stdout, "GPU-Accelerated Real-Time Video Looping for macOS\n");
    fprintf(stdout, "---\n");

    /* Initialize core systems first */
    logging_init();
    mem_init();

    LOG_INFO("==================================");
    LOG_INFO("Video Looper v1.0.0 starting...");
    LOG_INFO("==================================");
    LOG_INFO("Platform: macOS (OS X)");
    LOG_INFO("Build date: %s %s", __DATE__, __TIME__);

    /* Register global error handler for user-facing error dialogs */
    app_register_error_handler(on_app_error, NULL);
    LOG_DEBUG("Error handler registered");

    /* Initialize cleanup handlers (must be done early to protect all resources) */
    if (!cleanup_handlers_init()) {
        LOG_WARNING("Failed to initialize cleanup handlers - application may not clean up properly "
                    "on abnormal exit");
    }

    /* Initialize GStreamer */
    if (!initialize_gstreamer(&argc, &argv)) {
        LOG_ERROR("Failed to initialize GStreamer");
        mem_cleanup();
        logging_cleanup();
        return 1;
    }

    /* Create application context */
    AppContext *app_ctx = NULL;
    if (!initialize_app_context(&app_ctx)) {
        LOG_ERROR("Failed to initialize application context");
        gst_deinit();
        mem_cleanup();
        logging_cleanup();
        return 1;
    }

    /* Setup event loop */
    if (!setup_event_loop(app_ctx)) {
        LOG_ERROR("Failed to setup event loop");
        app_context_cleanup(app_ctx);
        gst_deinit();
        mem_cleanup();
        logging_cleanup();
        return 1;
    }

    /* Install signal handlers */
    install_signal_handlers();

    /* Initialize all application components (including NSWindow on main thread) */
    if (!initialize_components(app_ctx)) {
        LOG_ERROR("Failed to initialize application components");
        cleanup_components(app_ctx);
        if (app_ctx->main_loop) {
            g_main_loop_unref(app_ctx->main_loop);
        }
        app_context_cleanup(app_ctx);
        gst_deinit();
        mem_cleanup();
        logging_cleanup();
        return 1;
    }

    LOG_INFO("==================================");
    LOG_INFO("Application initialization complete");
    LOG_INFO("Ready for video looping");
    LOG_INFO("Press 1-9 to record, Escape to quit");
    LOG_INFO("==================================");

    /* Start the pipeline playing to display live video */
    LOG_DEBUG("Starting pipeline playback...");
    Pipeline *gst_pipeline = (Pipeline *)app_ctx->gst_pipeline;
    if (!pipeline_set_state(gst_pipeline, GST_STATE_PLAYING)) {
        LOG_ERROR("Failed to start pipeline playback");
        cleanup_components(app_ctx);
        if (app_ctx->main_loop) {
            g_main_loop_unref(app_ctx->main_loop);
        }
        app_context_cleanup(app_ctx);
        gst_deinit();
        mem_cleanup();
        logging_cleanup();
        return 1;
    }
    LOG_INFO("Pipeline started - live video should be visible");

    /* Run main event loop on main thread */
    LOG_DEBUG("Starting main event loop...");
    g_main_loop_run(app_ctx->main_loop);
    LOG_DEBUG("Main event loop exited");

    LOG_INFO("==================================");
    LOG_INFO("Initiating shutdown sequence...");
    LOG_INFO("==================================");

    /* Cleanup components in reverse initialization order
     * Note: The cleanup_handlers registered with atexit() will ensure
     * this cleanup happens even if the application exits abnormally
     */
    cleanup_components(app_ctx);

    /* Cleanup main loop */
    if (app_ctx->main_loop) {
        g_main_loop_unref(app_ctx->main_loop);
        app_ctx->main_loop = NULL;
    }

    /* Cleanup application context */
    app_context_cleanup(app_ctx);

    /* Deinitialize GStreamer */
    LOG_DEBUG("Deinitializing GStreamer...");
    gst_deinit();

    /* Cleanup utilities */
    mem_cleanup();
    logging_cleanup();

    fprintf(stdout, "---\n");
    fprintf(stdout, "Video Looper terminated normally\n");

    return 0;
}

/**
 * Main entry point - delegates to gst_macos_main()
 *
 * On macOS, osxvideosink requires NSApplication/NSRunLoop to be running
 * on the main thread. gst_macos_main() handles this by:
 * - Starting NSApplication on the main thread
 * - Running our application logic (app_main) on a secondary thread
 * - Ensuring proper Cocoa event loop integration
 *
 * @param argc Command line argument count
 * @param argv Command line arguments
 * @return Exit code from app_main
 */
int main(int argc, char *argv[])
{
    return gst_macos_main(app_main, argc, argv, NULL);
}
