#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

/**
 * @file app_context.h
 * @brief Application context and state management
 *
 * Defines the main application context structure that holds references
 * to all major components and their state.
 */

#include <gst/gst.h>
#include <time.h>

/* Include headers to avoid typedef conflicts and provide full definitions */
#include "../camera/camera_source.h"
#include "../gstreamer/pipeline_builder.h"
#include "../osx/window.h"
#include "../recording/recording_state.h"

/* Forward declarations for types not included */
typedef struct PlaybackManager PlaybackManager;

/* Pipeline is defined in pipeline_builder.h (included above) */
/* RecordingState is defined in recording_state.h (included above) */
/* CameraSource is defined in camera_source.h (included above) */
/* OSXWindow is defined in window.h (included above) */

/**
 * @struct AppContext
 * @brief Main application context holding all component references
 */
typedef struct {
    /* GStreamer context */
    GstElement *pipeline;
    GstBus *bus;
    GMainLoop *main_loop;

    /* Component references */
    CameraSource *camera;
    Pipeline *gst_pipeline;
    RecordingState *recording_state;
    OSXWindow *window;
    PlaybackManager *playback_mgr;

    /* Configuration */
    guint target_fps;     /* 120 fps target */
    guint grid_cells;     /* 10 cells */
    guint cell_width_px;  /* 320 pixels */
    guint camera_width;   /* Negotiated width */
    guint camera_height;  /* Negotiated height */
    gdouble aspect_ratio; /* width / height */

    /* Timing */
    guint64 startup_time_us;    /* When app started */
    guint64 last_frame_time_us; /* Last rendered frame timestamp */
} AppContext;

/**
 * Initialize application context
 *
 * @return Newly allocated AppContext, or NULL on failure
 */
AppContext *app_context_create(void);

/**
 * Cleanup application context and free all resources
 *
 * @param ctx Application context to cleanup
 */
void app_context_cleanup(AppContext *ctx);

/**
 * Get singleton application context instance
 *
 * @return Current application context, or NULL if not initialized
 */
AppContext *app_context_get(void);

/**
 * Set singleton application context instance
 *
 * @param ctx Application context to set
 */
void app_context_set(AppContext *ctx);

#endif /* APP_CONTEXT_H */
