#ifndef COMPOSITE_CAPS_H
#define COMPOSITE_CAPS_H

#include <gst/gst.h>

/**
 * Configure composite caps element for format conversion between videomixer and osxvideosink.
 *
 * The composite_caps capsfilter element ensures proper format conversion from the
 * videomixer output (composited 10-cell grid at 3200×height) to a format compatible
 * with osxvideosink. This handles:
 * - Format negotiation (e.g., BGRx, which is native for macOS Metal/OpenGL)
 * - Frame rate specification (120 fps for smooth rendering)
 * - Resolution specification (3200 pixels width for 10 cells × 320 pixels each)
 *
 * @param caps_element     The capsfilter element to configure (already created)
 * @param grid_width       Total width in pixels (3200 for 10×320 grid)
 * @param grid_height      Total height in pixels (determined by aspect ratio)
 * @param framerate_num    Numerator of target frame rate (e.g., 120)
 * @param framerate_den    Denominator of target frame rate (e.g., 1 for 120/1)
 * @return                 TRUE if caps configured successfully, FALSE on failure
 *
 * Error handling:
 * - If caps_element is NULL, logs error and returns FALSE
 * - If invalid dimensions (width/height <= 0), logs error and returns FALSE
 * - If invalid frame rate, logs error and returns FALSE
 * - If GStreamer cap setting fails, logs error and returns FALSE
 *
 * Notes:
 * - BGRx format is used because it's the most compatible with macOS Metal/OpenGL rendering
 * - Frame rate of 120 fps (120/1) is the target for smooth playback
 * - Grid resolution is calculated as: width=3200 (10 cells), height=aspect_ratio_height
 */
gboolean composite_caps_configure(GstElement *caps_element, gint grid_width, gint grid_height,
                                  gint framerate_num, gint framerate_den);

#endif // COMPOSITE_CAPS_H
