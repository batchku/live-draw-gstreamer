/**
 * @file live_queue.c
 * @brief Live queue element creation and caps negotiation for cell 1 live feed.
 *
 * Implements queue element creation and caps negotiation for the live camera
 * feed that displays in cell 1 of the video grid. Handles GPU memory buffering,
 * format negotiation with camera source, and proper capsfilter configuration
 * for downstream videomixer compatibility.
 */

#include "live_queue.h"
#include "../utils/logging.h"
#include <stdio.h>
#include <string.h>

/**
 * Create a live queue element with GPU memory configuration.
 *
 * The live queue buffers frames in GPU memory between the camera source and
 * videomixer. Configured with downstream leaky behavior to ensure continuous
 * live feed even during high-load recording sessions.
 */
GstElement *live_queue_create(const char *name)
{
    g_return_val_if_fail(name != NULL, NULL);

    GstElement *queue = gst_element_factory_make("queue", name);
    if (!queue) {
        LOG_ERROR("Failed to create live queue element '%s': queue factory not found", name);
        return NULL;
    }

    /**
     * Configure queue properties:
     * - max-size-buffers: 30 frames (≈ 1 second at 30fps camera input)
     *   This allows minimal buffering while decoupling source and sink rates.
     *   Camera outputs 30fps, videomixer may consume at variable rate (interpolating to 120fps)
     *
     * - max-size-bytes: 0 (unlimited)
     *   Rely on buffer count, not byte size. Each frame is approximately:
     *   1920×1080 @ 3 bytes/pixel (BGRx) ≈ 6.2MB
     *   30 buffers ≈ 186 MB in GPU memory (acceptable)
     *
     * - max-size-time: 0 (unlimited)
     *   Don't constrain by duration. Frame timing is handled by GStreamer's clock.
     *
     * - leaky: 2 (GST_QUEUE_LEAK_DOWNSTREAM)
     *   Drop oldest frames if buffer fills, but never drop the input stream.
     *   Critical for live feed: ensures frames keep flowing even during high videomixer load.
     *   When cell recording causes videomixer to slow down, we drop old live frames
     *   rather than blocking the camera source.
     */
    g_object_set(G_OBJECT(queue), "max-size-buffers", 30, /* 1 second at 30fps */
                 "max-size-bytes", 0, /* Unlimited bytes; rely on buffer count */
                 "max-size-time", 0,  /* Unlimited time; rely on buffer count */
                 "leaky", 2,          /* GST_QUEUE_LEAK_DOWNSTREAM */
                 NULL);

    LOG_DEBUG("Created live queue element '%s' with downstream leaky behavior", name);
    return queue;
}

/**
 * Configure caps negotiation for live feed.
 *
 * Sets the capsfilter element properties to negotiate video format between
 * camera source and videomixer. The capsfilter acts as a constraint that
 * GStreamer uses during caps negotiation to select compatible formats.
 */
gboolean live_queue_configure_caps(GstElement *live_caps_elem, const LiveQueueCaps *caps_config)
{
    if (!live_caps_elem) {
        LOG_ERROR("live_caps_elem is NULL");
        return FALSE;
    }

    if (!caps_config) {
        LOG_ERROR("caps_config is NULL");
        return FALSE;
    }

    /**
     * Build caps string with video format specification.
     *
     * Format: "video/x-raw,format=<fmt>,width=<w>,height=<h>,framerate=<num>/<den>"
     *
     * Components:
     * - video/x-raw: GStreamer video type (uncompressed raw video)
     * - format: Pixel format (BGRx for GPU, YUY2 alternative)
     * - width/height: Resolution (1920x1080 or fallback)
     * - framerate: Frames per second (30/1 for camera input)
     *
     * The capsfilter negotiates these constraints with upstream camera source
     * and downstream videomixer to ensure format compatibility.
     */
    char caps_string[512];
    int written = snprintf(caps_string, sizeof(caps_string),
                           "video/x-raw,format=%s,width=%d,height=%d,framerate=%d/%d",
                           caps_config->format, caps_config->width, caps_config->height,
                           caps_config->framerate_num, caps_config->framerate_den);

    if (written < 0 || written >= (int) sizeof(caps_string)) {
        LOG_ERROR("Failed to build caps string (buffer too small)");
        return FALSE;
    }

    LOG_DEBUG("Configuring live caps: %s", caps_string);

    /**
     * Create GStreamer caps object from string.
     *
     * GStreamer caps are specifications that elements use during negotiation.
     * If the caps string is invalid, gst_caps_from_string returns a valid
     * but empty caps object. We need to check if the string parses correctly.
     */
    GstCaps *gst_caps = gst_caps_from_string(caps_string);
    if (!gst_caps || gst_caps_is_empty(gst_caps)) {
        LOG_ERROR("Failed to parse caps string: %s", caps_string);
        if (gst_caps) {
            gst_caps_unref(gst_caps);
        }
        return FALSE;
    }

    /**
     * Apply caps to the capsfilter element.
     *
     * Setting the "caps" property tells the capsfilter what formats to accept/produce.
     * GStreamer will negotiate to find a format that satisfies this constraint
     * and matches upstream/downstream capabilities.
     */
    g_object_set(G_OBJECT(live_caps_elem), "caps", gst_caps, NULL);

    /**
     * Unreference the caps object.
     *
     * We passed a reference to the capsfilter via g_object_set.
     * The capsfilter now holds its own reference, so we can unref our copy.
     * Always unref gst_caps to avoid memory leaks.
     */
    gst_caps_unref(gst_caps);

    LOG_INFO("Live caps configured: %s", caps_string);
    return TRUE;
}

/**
 * Get recommended caps configuration from camera source element.
 *
 * Queries the camera source to determine what video formats it can provide,
 * then selects the best compatible format for the live feed.
 *
 * The selection strategy prioritizes:
 * 1. BGRx at 1920×1080 @ 30fps (ideal: GPU-compatible, HD resolution)
 * 2. BGRx at 1280×720 @ 30fps (fallback: lower bandwidth)
 * 3. YUY2 at 1920×1080 @ 30fps (alternative: different color format)
 * 4. First available format (last resort: compatibility mode)
 */
gboolean live_queue_negotiate_caps(GstElement *camera_source, LiveQueueCaps *out_caps_config)
{
    if (!camera_source) {
        LOG_ERROR("camera_source is NULL");
        return FALSE;
    }

    if (!out_caps_config) {
        LOG_ERROR("out_caps_config is NULL");
        return FALSE;
    }

    /**
     * Initialize output with default values.
     *
     * These are conservative defaults that should work with most cameras.
     * If caps negotiation fails, we'll still have valid values.
     */
    out_caps_config->width = 1920;
    out_caps_config->height = 1080;
    out_caps_config->framerate_num = 30;
    out_caps_config->framerate_den = 1;
    out_caps_config->format = "UYVY";

    /**
     * Query camera source for allowed caps.
     *
     * GStreamer elements expose their capabilities via pad templates.
     * We'll check what the camera can produce and select a compatible format.
     *
     * In practice, macOS avfvideosrc typically supports:
     * - BGRx at 1920×1080 @ 30fps (primary)
     * - BGRx at 1280×720 @ 30fps
     * - YUY2 at various resolutions
     */
    GstPad *camera_src_pad = gst_element_get_static_pad(camera_source, "src");
    if (!camera_src_pad) {
        LOG_WARNING("Failed to get source pad from camera element; using default caps");
        return TRUE; // Non-fatal: use defaults
    }

    /**
     * Get pad template caps to see what the camera can produce.
     *
     * Pad template caps describe the possible formats a pad can produce/consume.
     * We'll use these to confirm our format choices are compatible.
     */
    GstPadTemplate *pad_template = gst_pad_get_pad_template(camera_src_pad);
    gst_object_unref(camera_src_pad);

    if (!pad_template) {
        LOG_WARNING("Failed to get pad template from camera; using default caps");
        return TRUE; // Non-fatal: use defaults
    }

    /**
     * Extract caps from pad template.
     *
     * The caps in the template describe what formats are possible.
     * We'll iterate through them to find a good match.
     */
    GstCaps *template_caps = gst_pad_template_get_caps(pad_template);
    if (!template_caps) {
        LOG_WARNING("Failed to get template caps from camera; using default caps");
        return TRUE; // Non-fatal: use defaults
    }

    LOG_DEBUG("Camera pad template has %d caps structures", gst_caps_get_size(template_caps));

    /**
     * Iterate through available caps to find best match.
     *
     * Priority order:
     * 1. BGRx 1920×1080 @ 30fps
     * 2. BGRx 1280×720 @ 30fps
     * 3. YUY2 1920×1080 @ 30fps
     * 4. First structure (fallback)
     */
    gboolean found_ideal = FALSE;
    gboolean found_fallback = FALSE;

    for (guint i = 0; i < gst_caps_get_size(template_caps); i++) {
        GstStructure *structure = gst_caps_get_structure(template_caps, i);
        if (!structure) {
            continue;
        }

        const gchar *media_type = gst_structure_get_name(structure);
        if (!media_type || strcmp(media_type, "video/x-raw") != 0) {
            continue; // Skip non-video types
        }

        /**
         * Extract format, width, height from structure.
         *
         * GStreamer structures may have:
         * - Fixed values (e.g., format=BGRx)
         * - Ranges (e.g., width=[640, 1920])
         * - Lists (e.g., format={BGRx, YUY2})
         */
        const gchar *format = gst_structure_get_string(structure, "format");
        gint width = 1920, height = 1080; // Defaults
        gint framerate_num = 30, framerate_den = 1;

        // Extract width (might be a range, take max)
        const GValue *width_val = gst_structure_get_value(structure, "width");
        if (width_val) {
            if (G_VALUE_TYPE(width_val) == G_TYPE_INT) {
                width = g_value_get_int(width_val);
            } else if (GST_VALUE_HOLDS_INT_RANGE(width_val)) {
                width = gst_value_get_int_range_max(width_val);
            }
        }

        // Extract height (might be a range, take max)
        const GValue *height_val = gst_structure_get_value(structure, "height");
        if (height_val) {
            if (G_VALUE_TYPE(height_val) == G_TYPE_INT) {
                height = g_value_get_int(height_val);
            } else if (GST_VALUE_HOLDS_INT_RANGE(height_val)) {
                height = gst_value_get_int_range_max(height_val);
            }
        }

        // Extract framerate (might be a fraction or range)
        const GValue *fps_val = gst_structure_get_value(structure, "framerate");
        if (fps_val && GST_VALUE_HOLDS_FRACTION(fps_val)) {
            framerate_num = gst_value_get_fraction_numerator(fps_val);
            framerate_den = gst_value_get_fraction_denominator(fps_val);
        }

        LOG_DEBUG("  Cap %d: %s %d×%d @%d/%d fps", i, format ? format : "unknown", width, height,
                  framerate_num, framerate_den);

        /**
         * Check for ideal format: BGRx 1920×1080 @ 30fps.
         *
         * This is the best case: GPU-compatible format, HD resolution, native framerate.
         */
        if (!found_ideal && format && strcmp(format, "BGRx") == 0 && width >= 1920 &&
            height >= 1080) {
            out_caps_config->format = "BGRx";
            out_caps_config->width = 1920;
            out_caps_config->height = 1080;
            out_caps_config->framerate_num = 30;
            out_caps_config->framerate_den = 1;
            found_ideal = TRUE;
            LOG_DEBUG("Selected ideal caps: BGRx 1920×1080 @ 30fps");
            break; // Perfect match, stop searching
        }

        /**
         * Check for fallback: BGRx 1280×720 @ 30fps.
         *
         * Still GPU-compatible, lower bandwidth.
         */
        if (!found_fallback && format && strcmp(format, "BGRx") == 0 && width >= 1280 &&
            height >= 720) {
            out_caps_config->format = "BGRx";
            out_caps_config->width = 1280;
            out_caps_config->height = 720;
            out_caps_config->framerate_num = 30;
            out_caps_config->framerate_den = 1;
            found_fallback = TRUE;
            LOG_DEBUG("Found fallback caps: BGRx 1280×720 @ 30fps");
            // Continue searching in case we find ideal
        }

        /**
         * Check for secondary: YUY2 1920×1080 @ 30fps.
         *
         * Different color format but still HD resolution.
         * GStreamer can convert YUY2 to BGRx in videomixer if needed.
         */
        if (!found_fallback && format && strcmp(format, "YUY2") == 0 && width >= 1920 &&
            height >= 1080) {
            out_caps_config->format = "YUY2";
            out_caps_config->width = 1920;
            out_caps_config->height = 1080;
            out_caps_config->framerate_num = 30;
            out_caps_config->framerate_den = 1;
            found_fallback = TRUE;
            LOG_DEBUG("Found secondary caps: YUY2 1920×1080 @ 30fps");
            // Continue searching in case we find ideal
        }
    }

    gst_caps_unref(template_caps);

    /**
     * Log negotiation result.
     *
     * Report what caps we selected. The priority-based selection ensures
     * we always have a fallback, even if ideal formats aren't available.
     */
    if (found_ideal) {
        LOG_INFO("Negotiated ideal live caps: %s %d×%d @ %d/%d fps", out_caps_config->format,
                 out_caps_config->width, out_caps_config->height, out_caps_config->framerate_num,
                 out_caps_config->framerate_den);
    } else if (found_fallback) {
        LOG_INFO("Negotiated fallback live caps: %s %d×%d @ %d/%d fps", out_caps_config->format,
                 out_caps_config->width, out_caps_config->height, out_caps_config->framerate_num,
                 out_caps_config->framerate_den);
    } else {
        LOG_INFO("Using default live caps: %s %d×%d @ %d/%d fps", out_caps_config->format,
                 out_caps_config->width, out_caps_config->height, out_caps_config->framerate_num,
                 out_caps_config->framerate_den);
    }

    return TRUE;
}
