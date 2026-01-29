/**
 * @file window.c
 * @brief OS X window management (C API wrapper)
 *
 * Pure C implementation that wraps Objective-C window functionality.
 * Actual Cocoa integration is in window.m.
 */

#include "window.h"
#include "../utils/logging.h"
#include <gst/video/video.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Forward declarations for Objective-C helper functions (implemented in window.m)
 * ========================================================================= */

extern void window_calculate_dimensions(guint grid_cols, guint grid_rows, CGFloat aspect_ratio,
                                        CGFloat *out_width, CGFloat *out_height);

extern GstElement *window_create_osxvideosink(void);

extern gboolean window_create_nswindow(OSXWindow *win, CGFloat width, CGFloat height);

extern void window_update_nswindow_frame(OSXWindow *win, CGFloat width, CGFloat height);

extern gboolean window_check_nswindow_visible(OSXWindow *win);

extern void window_request_nsview_redraw(OSXWindow *win);

extern void window_release_nswindow(OSXWindow *win);

/* ============================================================================
 * Public API Implementation
 * ========================================================================= */

OSXWindow *window_create(guint grid_cols, guint grid_rows)
{
    LOG_INFO("Creating OS X window for %ux%u grid...", grid_cols, grid_rows);

    // Allocate window context
    OSXWindow *win = malloc(sizeof(OSXWindow));
    if (!win) {
        LOG_ERROR("Failed to allocate OSXWindow structure");
        return NULL;
    }
    memset(win, 0, sizeof(OSXWindow));

    // Store grid configuration
    win->grid_cols = grid_cols;
    win->grid_rows = grid_rows;
    win->cell_width = 320.0;
    win->cell_height = 180.0;  // 16:9 aspect to match 1080p camera (320/180 = 1.778)
    win->aspect_ratio = 16.0 / 9.0; // 16:9 aspect ratio to match camera
    win->resizable = TRUE;

    // Calculate window dimensions with 16:9 aspect ratio
    CGFloat window_width = 0;
    CGFloat window_height = 0;
    window_calculate_dimensions(grid_cols, grid_rows, win->aspect_ratio, &window_width,
                                &window_height);
    LOG_DEBUG("Window dimensions: %.0f x %.0f pixels", window_width, window_height);

    // Create NSWindow and VideoLooperView on main thread (Objective-C)
    if (!window_create_nswindow(win, window_width, window_height)) {
        LOG_ERROR("Failed to create NSWindow");
        free(win);
        return NULL;
    }

    // Create osxvideosink element for GStreamer integration
    GstElement *videosink = window_create_osxvideosink();
    if (!videosink) {
        LOG_ERROR("Failed to create osxvideosink element");
        window_cleanup(win);
        return NULL;
    }

    win->videosink = videosink;

    LOG_INFO("OS X window successfully created (%.0fx%.0f @ 16:9 aspect ratio)", window_width,
             window_height);
    return win;
}

GstElement *window_get_videosink(OSXWindow *win)
{
    if (!win) {
        LOG_WARNING("window_get_videosink: NULL window context");
        return NULL;
    }
    return win->videosink;
}

void window_set_aspect_ratio(OSXWindow *win, gdouble aspect_ratio)
{
    if (!win || aspect_ratio <= 0) {
        return;
    }

    LOG_DEBUG("Setting window aspect ratio to %.3f", aspect_ratio);

    win->aspect_ratio = (CGFloat) aspect_ratio;

    // Recalculate cell height based on new aspect ratio
    win->cell_height = win->cell_width / win->aspect_ratio;

    // Update window size
    CGFloat new_width = win->cell_width * win->grid_cols;
    CGFloat new_height = win->cell_height * win->grid_rows;
    window_update_nswindow_frame(win, new_width, new_height);
}

void window_on_resize(OSXWindow *win, CGFloat width, CGFloat height)
{
    if (!win) {
        return;
    }

    LOG_DEBUG("Window resize event: %.0f x %.0f", width, height);

    // Recalculate grid cell dimensions based on new window size
    if (win->grid_cols > 0) {
        win->cell_width = width / win->grid_cols;
    }
    win->cell_height = height / win->grid_rows;

    // Update videosink to reflect new window dimensions
    // (osxvideosink should auto-scale video to fit the view)
}

void window_request_render(OSXWindow *win)
{
    if (!win || !win->video_view) {
        return;
    }

    window_request_nsview_redraw(win);
}

void window_swap_buffers(OSXWindow *win)
{
    if (!win) {
        return;
    }

    // For osxvideosink, this is typically a no-op as the sink handles presentation
    // This method is provided for future OpenGL/Metal integration
    LOG_DEBUG("window_swap_buffers called (no-op for osxvideosink)");
}

gboolean window_is_visible(OSXWindow *win)
{
    if (!win || !win->nswindow) {
        return FALSE;
    }

    return window_check_nswindow_visible(win);
}

void window_cleanup(OSXWindow *win)
{
    if (!win) {
        return;
    }

    LOG_INFO("Cleaning up OS X window");

    // Release GStreamer videosink
    if (win->videosink) {
        gst_object_unref(win->videosink);
        win->videosink = NULL;
        LOG_DEBUG("osxvideosink element released");
    }

    // Release Cocoa objects (Objective-C)
    window_release_nswindow(win);

    // Free structure
    free(win);

    LOG_DEBUG("Window cleanup complete");
}
