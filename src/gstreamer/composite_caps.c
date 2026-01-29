#include "composite_caps.h"
#include <gst/gst.h>
#include <stdio.h>
#include <string.h>

gboolean composite_caps_configure(GstElement *caps_element, gint grid_width, gint grid_height,
                                  gint framerate_num, gint framerate_den)
{

    if (!caps_element) {
        fprintf(stderr, "[ERROR] composite_caps_configure: caps_element is NULL\n");
        return FALSE;
    }

    if (grid_width <= 0 || grid_height <= 0) {
        fprintf(stderr, "[ERROR] composite_caps_configure: Invalid grid dimensions (%d x %d)\n",
                grid_width, grid_height);
        return FALSE;
    }

    if (framerate_num <= 0 || framerate_den <= 0) {
        fprintf(stderr, "[ERROR] composite_caps_configure: Invalid frame rate (%d/%d)\n",
                framerate_num, framerate_den);
        return FALSE;
    }

    /**
     * Create caps string for the composite capsfilter.
     *
     * Format selection strategy:
     * - UYVY format (required by osxvideosink on macOS)
     * - videoconvert element handles conversion from videomixer's BGRx to UYVY
     *
     * The videomixer outputs frames in BGRx format from the camera input.
     * osxvideosink only accepts UYVY format for rendering.
     * videoconvert performs the format conversion efficiently.
     *
     * Framerate specification (120/1):
     * - 120 fps playback is the PRD requirement
     * - This ensures smooth rendering across all 10 cells
     * - The pipeline clock will synchronize actual playback rate
     *
     * Resolution specification (3200×height):
     * - Videomixer outputs composite at full grid size
     * - 3200 pixels = 10 cells × 320 pixels per cell
     * - Height is determined by camera aspect ratio
     * - osxvideosink will scale to window size
     */

    // Create a caps structure - let format negotiate automatically
    // Only constrain dimensions and framerate
    GstCaps *caps =
        gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT,
                            grid_width, "height", G_TYPE_INT, grid_height, "framerate",
                            GST_TYPE_FRACTION, framerate_num, framerate_den, NULL);

    if (!caps) {
        fprintf(stderr, "[ERROR] composite_caps_configure: Failed to create GStreamer caps\n");
        return FALSE;
    }

    // Set the caps on the capsfilter element
    g_object_set(G_OBJECT(caps_element), "caps", caps, NULL);

    gst_caps_unref(caps);

    fprintf(stdout,
            "[INFO] composite_caps_configure: Configured composite caps for %dx%d @ %d/%d fps\n",
            grid_width, grid_height, framerate_num, framerate_den);

    return TRUE;
}
