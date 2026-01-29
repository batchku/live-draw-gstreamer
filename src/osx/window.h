/**
 * @file window.h
 * @brief OS X window and rendering component
 *
 * Manages Cocoa NSWindow creation, sizing, Metal/OpenGL rendering context,
 * and integration with GStreamer osxvideosink for video display.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <CoreGraphics/CoreGraphics.h>
#include <gst/gst.h>

/**
 * @struct OSXWindow
 * @brief OS X window context for video rendering
 *
 * Encapsulates NSWindow, rendering view, and GStreamer sink element.
 */
typedef struct {
    void *nswindow;        /**< NSWindow* (opaque pointer) */
    void *video_view;      /**< NSView* for video rendering (opaque pointer) */
    void *cocoa_delegate;  /**< Window delegate for events (opaque pointer) */
    GstElement *videosink; /**< osxvideosink element for frame output */
    CGFloat cell_width;    /**< Width of each grid cell (320px) */
    CGFloat cell_height;   /**< Height of each cell (calculated from aspect ratio) */
    guint grid_cols;       /**< Number of grid columns (10) */
    guint grid_rows;       /**< Number of grid rows (1) */
    CGFloat aspect_ratio;  /**< Video aspect ratio (width / height) */
    gboolean resizable;    /**< Whether window is resizable */
} OSXWindow;

/**
 * Create OS X window with video rendering view
 *
 * Creates a native Cocoa NSWindow with the specified grid dimensions.
 * Window size is calculated as:
 * - Width: cell_width * grid_cols (320px * 10 = 3200px)
 * - Height: cell_width / aspect_ratio * grid_rows (320 / (16/9) * 1 = 180px)
 *
 * The window title is set to "Video Looper".
 * The window is positioned using macOS default positioning (cascade).
 *
 * @param num_cells Number of grid cells (typically 10 for 10x1 layout)
 * @return Newly allocated OSXWindow, or NULL on failure
 */
OSXWindow *window_create(guint num_cells);

/**
 * Get GStreamer osxvideosink element from window
 *
 * Returns the videosink element that is integrated with the window's
 * video view for frame rendering.
 *
 * @param win OS X window context
 * @return GStreamer videosink element, or NULL if window is invalid
 */
GstElement *window_get_videosink(OSXWindow *win);

/**
 * Set window aspect ratio from camera input
 *
 * Updates the window's expected video aspect ratio, which affects
 * cell height calculation. Should be called after camera resolution
 * is negotiated (e.g., 16:9 for 1920x1080).
 *
 * @param win OS X window context
 * @param aspect_ratio Video aspect ratio (width / height), e.g., 1.777 for 16:9
 */
void window_set_aspect_ratio(OSXWindow *win, gdouble aspect_ratio);

/**
 * Handle window resize events
 *
 * Called when window is resized by user or system.
 * Recalculates grid cell dimensions and updates videosink.
 *
 * @param win OS X window context
 * @param width New window width in pixels
 * @param height New window height in pixels
 */
void window_on_resize(OSXWindow *win, CGFloat width, CGFloat height);

/**
 * Request rendering on next frame
 *
 * Marks the window view as needing redraw.
 * Called when new frames are available from GStreamer.
 *
 * @param win OS X window context
 */
void window_request_render(OSXWindow *win);

/**
 * Swap rendering buffers (if applicable to Metal/OpenGL)
 *
 * For Metal/OpenGL rendering contexts, triggers buffer swap.
 * For osxvideosink, this may be a no-op.
 *
 * @param win OS X window context
 */
void window_swap_buffers(OSXWindow *win);

/**
 * Check if window is still open
 *
 * @param win OS X window context
 * @return TRUE if window is visible and active, FALSE otherwise
 */
gboolean window_is_visible(OSXWindow *win);

/**
 * Cleanup and release window resources
 *
 * Releases the NSWindow, video view, and associated resources.
 * After cleanup, the pointer should not be used.
 *
 * @param win OS X window context to cleanup
 */
void window_cleanup(OSXWindow *win);

#endif /* WINDOW_H */
