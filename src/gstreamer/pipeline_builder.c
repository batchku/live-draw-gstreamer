#include "pipeline_builder.h"
#include "../app/app_error.h"
#include "../utils/logging.h"
#include "composite_caps.h"
#include "gst_elements.h"
#include "gstreamer_error_handler.h"
#include "live_queue.h"
#include "live_tee.h"
#include "performance_config.h"
#include "pipeline_error_recovery.h"
#include "record_bin.h"
#include <glib.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <stdio.h>
#include <string.h>

static gboolean pipeline_layer_position(int layer_number, gint *xpos, gint *ypos)
{
    if (layer_number < 1 || layer_number > TOTAL_LAYERS || !xpos || !ypos) {
        return FALSE;
    }

    int layer_index = layer_number - 1;
    int col = (layer_index % LAYER_COLUMNS) + 1; // columns 1-10, col 0 reserved for live
    int row = layer_index / LAYER_COLUMNS;       // rows 0-1

    *xpos = col * CELL_WIDTH_PX;
    *ypos = row * CELL_HEIGHT_PX;
    return TRUE;
}
/**
 * Static bus watch handler to process GStreamer bus messages.
 *
 * Handles ERROR, WARNING, INFO, STATE_CHANGED, and EOS messages from the GStreamer pipeline.
 * Logs messages using the centralized logging system and dispatches to registered callback.
 *
 * Called whenever messages arrive on the pipeline bus.
 *
 * @param bus       The GStreamer bus (unused)
 * @param msg       The GStreamer message to process
 * @param user_data Pointer to the Pipeline structure
 * @return          TRUE to keep watching the bus, FALSE to stop
 */
static gboolean pipeline_bus_watch_handler(GstBus *bus G_GNUC_UNUSED, GstMessage *msg,
                                           gpointer user_data)
{
    Pipeline *pipeline = (Pipeline *) user_data;

    if (!pipeline) {
        return TRUE; // Keep watching even if pipeline is invalid
    }

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err = NULL;
        gchar *debug_info = NULL;
        gst_message_parse_error(msg, &err, &debug_info);

        if (err) {
            // Log main error message at ERROR level
            LOG_ERROR("GStreamer pipeline error: %s", err->message);

            // Log debug info at DEBUG level if available
            if (debug_info) {
                LOG_DEBUG("Error debug info: %s", debug_info);
                g_free(debug_info);
            }

            // Log to application error handler for possible user-facing dialog
            app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED, "GStreamer pipeline error: %s",
                          err->message);

            // Dispatch to registered callback if present
            if (pipeline->msg_callback) {
                pipeline->msg_callback("error", err->message);
            }

            g_error_free(err);
        }
        break;
    }

    case GST_MESSAGE_WARNING: {
        GError *err = NULL;
        gchar *debug_info = NULL;
        gst_message_parse_warning(msg, &err, &debug_info);

        if (err) {
            // Log main warning message at WARNING level
            LOG_WARNING("GStreamer pipeline warning: %s", err->message);

            // Log debug info at DEBUG level if available
            if (debug_info) {
                LOG_DEBUG("Warning debug info: %s", debug_info);
                g_free(debug_info);
            }

            // Dispatch to registered callback if present
            if (pipeline->msg_callback) {
                pipeline->msg_callback("warning", err->message);
            }

            g_error_free(err);
        }
        break;
    }

    case GST_MESSAGE_INFO: {
        GError *err = NULL;
        gchar *debug_info = NULL;
        gst_message_parse_info(msg, &err, &debug_info);

        if (err) {
            // Log info message at INFO level
            LOG_INFO("GStreamer pipeline: %s", err->message);

            // Log debug info at DEBUG level if available
            if (debug_info) {
                LOG_DEBUG("Info debug info: %s", debug_info);
                g_free(debug_info);
            }

            // Dispatch to registered callback if present
            if (pipeline->msg_callback) {
                pipeline->msg_callback("info", err->message);
            }

            g_error_free(err);
        }
        break;
    }

    case GST_MESSAGE_STATE_CHANGED: {
        GstState old_state, new_state, pending_state;
        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);

        // Log state transition at INFO level
        const gchar *old_name = gst_element_state_get_name(old_state);
        const gchar *new_name = gst_element_state_get_name(new_state);
        const gchar *pending_name = gst_element_state_get_name(pending_state);

        LOG_INFO("Pipeline state changed: %s → %s (pending: %s)", old_name, new_name, pending_name);

        // Dispatch to registered callback if present
        if (pipeline->msg_callback) {
            gchar msg_str[256];
            snprintf(msg_str, sizeof(msg_str), "State changed: %s → %s", old_name, new_name);
            pipeline->msg_callback("state_changed", msg_str);
        }
        break;
    }

    case GST_MESSAGE_EOS: {
        // Log end-of-stream at INFO level (expected during normal operation)
        LOG_INFO("Pipeline reached end-of-stream");

        // Dispatch to registered callback if present
        if (pipeline->msg_callback) {
            pipeline->msg_callback("eos", "End of stream reached");
        }
        break;
    }

    case GST_MESSAGE_ELEMENT: {
        // Handle element-specific messages
        const GstStructure *s = gst_message_get_structure(msg);
        if (s) {
            const gchar *msg_name = gst_structure_get_name(s);
            LOG_DEBUG("Element message: %s", msg_name);

            // Handle prepare-window-handle message from osxvideosink
            if (g_strcmp0(msg_name, "prepare-window-handle") == 0) {
                if (pipeline->window && GST_IS_VIDEO_OVERLAY(GST_MESSAGE_SRC(msg))) {
                    // Get NSView from OSXWindow
                    // OSXWindow struct has video_view field containing NSView*
                    typedef struct {
                        void *nswindow;
                        void *video_view;
                        void *cocoa_delegate;
                        GstElement *videosink;
                        // ... rest of fields not needed here
                    } OSXWindow_Internal;

                    OSXWindow_Internal *win = (OSXWindow_Internal *)pipeline->window;

                    if (win->video_view) {
                        LOG_DEBUG("Setting window handle for osxvideosink");
                        gst_video_overlay_set_window_handle(
                            GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(msg)),
                            (guintptr)win->video_view
                        );
                        LOG_INFO("osxvideosink window handle configured");
                    } else {
                        LOG_WARNING("Window has no video_view for overlay");
                    }
                } else {
                    LOG_WARNING("Cannot set window handle - window not available");
                }
            }
        }
        break;
    }

    case GST_MESSAGE_DURATION_CHANGED: {
        // Log duration changes at DEBUG level (informational but verbose)
        LOG_DEBUG("Pipeline duration changed");
        break;
    }

    case GST_MESSAGE_QOS:
    case GST_MESSAGE_LATENCY:
    case GST_MESSAGE_ASYNC_DONE:
    case GST_MESSAGE_NEW_CLOCK:
    case GST_MESSAGE_STREAM_STATUS:
    case GST_MESSAGE_STREAM_START:
        // Silently ignore common informational messages
        break;

    default: {
        // Log other unhandled message types at DEBUG level
        LOG_DEBUG("Unhandled GStreamer message type: %s", GST_MESSAGE_TYPE_NAME(msg));
        break;
    }
    }

    return TRUE; // Keep watching the bus for more messages
}

Pipeline *pipeline_create(GstElement *camera_source_element)
{
    if (!camera_source_element) {
        LOG_ERROR("camera_source_element is NULL");
        return NULL;
    }

    // Allocate Pipeline structure
    Pipeline *p = (Pipeline *) g_malloc0(sizeof(Pipeline));
    if (!p) {
        LOG_ERROR("Failed to allocate Pipeline structure");
        return NULL;
    }

    // Create pipeline
    p->pipeline = gst_pipeline_new("video-looper-pipeline");
    if (!p->pipeline) {
        LOG_ERROR("Failed to create pipeline");
        g_free(p);
        return NULL;
    }

    // Set camera source (assume it's already created)
    p->camera_source = camera_source_element;
    gst_object_ref(p->camera_source); // Increment ref count since we hold a reference

    // Create tee element to split camera stream
    p->live_tee = gst_element_factory_make("tee", "live-tee");
    if (!p->live_tee) {
        LOG_ERROR("Failed to create tee element");
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Configure tee element for deadlock-free stream splitting
    // This enables allow-not-linked to prevent deadlocks when recording bins are added/removed
    if (!live_tee_configure(p->live_tee)) {
        LOG_WARNING("Failed to fully configure tee element; proceeding with default settings");
        // Don't fail here; basic tee functionality should still work
    }

    // Create live queue element for cell 1 (GPU memory buffer)
    // Uses live_queue_create() for specialized configuration and logging
    p->live_queue = live_queue_create("live-queue");
    if (!p->live_queue) {
        LOG_ERROR("Failed to create live queue element");
        gst_object_unref(p->live_tee);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Apply performance optimization for live feed queue (T-8.2: 120 fps stability)
    // Configure queue sizing and latency for low-latency continuous display
    PerformanceQueueConfig live_queue_perf = performance_config_live_queue();
    if (!performance_apply_queue_config(p->live_queue, &live_queue_perf, "live_feed")) {
        LOG_WARNING("Failed to apply performance config to live queue; using defaults");
        // Continue anyway; basic queue functionality still works
    }

    // Create videoscale element to scale camera feed to cell size (320x180)
    GstElement *live_scale = gst_element_factory_make("videoscale", "live-scale");
    if (!live_scale) {
        LOG_ERROR("Failed to create videoscale element");
        gst_object_unref(p->live_queue);
        gst_object_unref(p->live_tee);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }
    // Configure videoscale to fill the target size without adding borders
    g_object_set(G_OBJECT(live_scale), "add-borders", FALSE, NULL);
    LOG_DEBUG("videoscale configured: add-borders=false");

    // Create videoconvert before caps to ensure proper format conversion
    GstElement *live_convert = gst_element_factory_make("videoconvert", "live-convert");
    if (!live_convert) {
        LOG_ERROR("Failed to create live videoconvert element");
        gst_object_unref(live_scale);
        gst_object_unref(p->live_queue);
        gst_object_unref(p->live_tee);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Create capsfilter for live feed to scale it to 320x180 (cell size)
    p->live_caps = gst_elements_create_capsfilter("live-caps");
    if (!p->live_caps) {
        LOG_ERROR("Failed to create live capsfilter element");
        gst_object_unref(live_convert);
        gst_object_unref(live_scale);
        gst_object_unref(p->live_queue);
        gst_object_unref(p->live_tee);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Configure live caps to scale down to cell size: 320x180 (16:9 aspect)
    // Use I420 format which videomixer handles better than UYVY
    // Height must match window cell height (320 / 1.778 = 180)
    GstCaps *cell_caps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, "I420",
        "width", G_TYPE_INT, CELL_WIDTH_PX,
        "height", G_TYPE_INT, CELL_HEIGHT_PX,
        NULL);
    g_object_set(G_OBJECT(p->live_caps), "caps", cell_caps, NULL);
    gst_caps_unref(cell_caps);
    LOG_INFO("Live feed will be scaled to %dx%d I420 (cell size)", CELL_WIDTH_PX, CELL_HEIGHT_PX);

    // Create compositor for compositing 10 cells (modern replacement for videomixer)
    p->videomixer = gst_element_factory_make("compositor", "compositor");
    if (!p->videomixer) {
        LOG_ERROR("Failed to create compositor element");
        gst_object_unref(p->live_caps);
        gst_object_unref(p->live_queue);
        gst_object_unref(p->live_tee);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Configure videomixer for 120 fps composition (T-8.2: Performance optimization)
    // Optimizes latency and composition for sustained 120 fps grid rendering
    guint64 mixer_latency = performance_config_videomixer_latency();
    GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(p->videomixer), "latency");
    if (pspec) {
        g_object_set(G_OBJECT(p->videomixer), "background", 2, // White background (2=white, 1=black)
                     "latency", mixer_latency,                 // Optimized for 120fps composition
                     NULL);
        LOG_DEBUG("Applied videomixer latency=%llu ns for 120fps composition",
                  (unsigned long long) mixer_latency);
    } else {
        g_object_set(G_OBJECT(p->videomixer), "background", 2, // White background (2=white, 1=black)
                     NULL);
        LOG_WARNING("Videomixer 'latency' property not available; composition may not be optimal");
    }

    // Create videoconvert for format conversion between videomixer and osxvideosink
    p->videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    if (!p->videoconvert) {
        LOG_ERROR("Failed to create videoconvert element");
        gst_object_unref(p->videomixer);
        gst_object_unref(p->live_caps);
        gst_object_unref(p->live_queue);
        gst_object_unref(p->live_tee);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Create composite capsfilter for format conversion
    p->composite_caps = gst_element_factory_make("capsfilter", "composite-caps");
    if (!p->composite_caps) {
        LOG_ERROR("Failed to create composite capsfilter element");
        gst_object_unref(p->videoconvert);
        gst_object_unref(p->videomixer);
        gst_object_unref(p->live_caps);
        gst_object_unref(p->live_queue);
        gst_object_unref(p->live_tee);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Configure composite caps for format conversion between videomixer and osxvideosink
    // Grid dimensions: 3520×360 (11 columns × 320 pixels, 2 rows)
    // Each cell is 320x180 (16:9 aspect ratio) - matches camera and window aspect ratio
    // Frame rate: 120/1 (120 fps for target rendering rate)
    const gint COMPOSITE_GRID_WIDTH = CELL_WIDTH_PX * GRID_COLS;
    const gint COMPOSITE_GRID_HEIGHT = CELL_HEIGHT_PX * GRID_ROWS;
    const gint TARGET_FRAMERATE_NUM = 120;
    const gint TARGET_FRAMERATE_DEN = 1;

    if (!composite_caps_configure(p->composite_caps, COMPOSITE_GRID_WIDTH, COMPOSITE_GRID_HEIGHT,
                                  TARGET_FRAMERATE_NUM, TARGET_FRAMERATE_DEN)) {
        LOG_ERROR("Failed to configure composite caps");
        gst_object_unref(p->composite_caps);
        gst_object_unref(p->videomixer);
        gst_object_unref(p->live_caps);
        gst_object_unref(p->live_queue);
        gst_object_unref(p->live_tee);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Create osxvideosink for rendering to Cocoa window
    // Note: This will generate a warning about NSRunLoop, but should still render video
    p->osxvideosink = gst_element_factory_make("osxvideosink", "osxvideosink");
    if (!p->osxvideosink) {
        LOG_ERROR("Failed to create osxvideosink element");
        gst_object_unref(p->composite_caps);
        gst_object_unref(p->videoconvert);
        gst_object_unref(p->videomixer);
        gst_object_unref(p->live_caps);
        gst_object_unref(p->live_queue);
        gst_object_unref(p->live_tee);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Configure osxvideosink for video playback
    g_object_set(G_OBJECT(p->osxvideosink),
                 "sync", TRUE,                   // Synchronize to clock for proper framerate
                 "force-aspect-ratio", FALSE,    // Use exact dimensions (window handles aspect ratio)
                 NULL);
    LOG_DEBUG("osxvideosink configured: sync=true, force-aspect-ratio=false");

    // Store live_scale and live_convert in temporary variables since we don't have fields for them
    // They will be owned by the pipeline bin

    // Add elements to pipeline (camera source is preconfigured with caps)
    gst_bin_add_many(GST_BIN(p->pipeline), p->camera_source, p->live_tee, p->live_queue,
                     live_scale, live_convert, p->live_caps, p->videomixer, p->videoconvert, p->composite_caps, p->osxvideosink, NULL);

    // Link elements: camera_source → live_tee
    if (!gst_element_link(p->camera_source, p->live_tee)) {
        LOG_ERROR("Failed to link camera_source to live_tee");
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }
    LOG_DEBUG("Linked: camera_source → live_tee");

    // Link live stream path: live_tee → live_queue → live_scale → live_convert → live_caps → videomixer
    // Tee has request pads - must manually request a src pad and link it
    GstPad *tee_src_pad = gst_element_request_pad_simple(p->live_tee, "src_%u");
    GstPad *queue_sink_pad = gst_element_get_static_pad(p->live_queue, "sink");

    if (!tee_src_pad || !queue_sink_pad) {
        LOG_ERROR("Failed to get pads for live_tee → live_queue linkage");
        if (tee_src_pad) gst_object_unref(tee_src_pad);
        if (queue_sink_pad) gst_object_unref(queue_sink_pad);
        gst_bin_remove_many(GST_BIN(p->pipeline), p->osxvideosink, p->composite_caps, p->videoconvert, p->videomixer,
                            p->live_caps, live_convert, live_scale, p->live_queue, p->live_tee, p->camera_source, NULL);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    if (gst_pad_link(tee_src_pad, queue_sink_pad) != GST_PAD_LINK_OK) {
        LOG_ERROR("Failed to link live_tee to live_queue");
        gst_object_unref(tee_src_pad);
        gst_object_unref(queue_sink_pad);
        gst_bin_remove_many(GST_BIN(p->pipeline), p->osxvideosink, p->composite_caps, p->videoconvert, p->videomixer,
                            p->live_caps, live_convert, live_scale, p->live_queue, p->live_tee, p->camera_source, NULL);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    gst_object_unref(tee_src_pad);
    gst_object_unref(queue_sink_pad);
    LOG_DEBUG("live_tee → live_queue linked via request pad");

    // Link order: queue → convert → scale → caps
    // videoconvert first to handle UYVY from camera, then videoscale can work with standard formats
    if (!gst_element_link(p->live_queue, live_convert)) {
        LOG_ERROR("Failed to link live_queue to live_convert");
        gst_bin_remove_many(GST_BIN(p->pipeline), p->osxvideosink, p->composite_caps, p->videoconvert, p->videomixer,
                            p->live_caps, live_convert, live_scale, p->live_queue, p->live_tee, p->camera_source, NULL);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    if (!gst_element_link(live_convert, live_scale)) {
        LOG_ERROR("Failed to link live_convert to live_scale");
        gst_bin_remove_many(GST_BIN(p->pipeline), p->osxvideosink, p->composite_caps, p->videoconvert, p->videomixer,
                            p->live_caps, live_convert, live_scale, p->live_queue, p->live_tee, p->camera_source, NULL);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    if (!gst_element_link(live_scale, p->live_caps)) {
        LOG_ERROR("Failed to link live_scale to live_caps");
        gst_bin_remove_many(GST_BIN(p->pipeline), p->osxvideosink, p->composite_caps, p->videoconvert, p->videomixer,
                            p->live_caps, live_convert, live_scale, p->live_queue, p->live_tee, p->camera_source, NULL);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Link live_caps to videomixer pad 0 (cell 1)
    // Request sink pad for live feed (cell 1, leftmost)
    GstPad *live_caps_src = gst_element_get_static_pad(p->live_caps, "src");
    GstPad *mixer_sink_pad_0 = gst_element_request_pad_simple(p->videomixer, "sink_%u");
    if (!live_caps_src || !mixer_sink_pad_0) {
        LOG_ERROR("Failed to get pads for live_caps → videomixer linkage");
        if (live_caps_src)
            gst_object_unref(live_caps_src);
        if (mixer_sink_pad_0)
            gst_object_unref(mixer_sink_pad_0);
        gst_bin_remove_many(GST_BIN(p->pipeline), p->osxvideosink, p->composite_caps, p->videoconvert, p->videomixer,
                            p->live_caps, live_convert, live_scale, p->live_queue, p->live_tee, p->camera_source, NULL);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Configure pad properties for live feed (row 0, col 0)
    // xpos: 0 pixels (leftmost)
    // ypos: 0 pixels (top)
    // width/height: 320x180 (16:9 cell size)
    // zorder: 0 (back layer)
    // alpha: 1.0 (fully opaque)
    g_object_set(mixer_sink_pad_0,
                 "xpos", 0,
                 "ypos", 0,
                 "width", CELL_WIDTH_PX,
                 "height", CELL_HEIGHT_PX,
                 "zorder", 0,
                 "alpha", 1.0,
                 NULL);
    LOG_DEBUG("Compositor sink pad 0 configured: %dx%d at (0,0)", CELL_WIDTH_PX,
              CELL_HEIGHT_PX);

    if (gst_pad_link(live_caps_src, mixer_sink_pad_0) != GST_PAD_LINK_OK) {
        LOG_ERROR("Failed to link live_caps to videomixer pad 0");
        gst_object_unref(live_caps_src);
        gst_object_unref(mixer_sink_pad_0);
        gst_bin_remove_many(GST_BIN(p->pipeline), p->osxvideosink, p->composite_caps, p->videoconvert, p->videomixer,
                            p->live_caps, live_convert, live_scale, p->live_queue, p->live_tee, p->camera_source, NULL);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    gst_object_unref(live_caps_src);
    gst_object_unref(mixer_sink_pad_0);

    // Pre-allocate sink pads for playback layers (1-20) for future use
    // This ensures the videomixer is prepared for dynamic playback bin addition
    // Pads are stored in cell_sink_pads array for later use by playback bins
    for (int layer = 1; layer <= TOTAL_LAYERS; layer++) {
        GstPad *mixer_sink_pad = gst_element_request_pad_simple(p->videomixer, "sink_%u");
        if (!mixer_sink_pad) {
            fprintf(stderr, "[WARNING] pipeline_create: Failed to request sink pad for layer %d\n",
                    layer);
            p->cell_sink_pads[layer - 1] = NULL;
            continue;
        }

        gint xpos = 0;
        gint ypos = 0;
        if (!pipeline_layer_position(layer, &xpos, &ypos)) {
            fprintf(stderr,
                    "[WARNING] pipeline_create: Failed to compute position for layer %d\n",
                    layer);
            gst_object_unref(mixer_sink_pad);
            p->cell_sink_pads[layer - 1] = NULL;
            continue;
        }

        // Configure pad properties for this playback layer
        // xpos: columns 1-10 (col 0 reserved for live)
        // ypos: row 0 or 1
        // zorder: layer number (higher zorder = front layer)
        // alpha: 1.0 (fully opaque)
        // Note: width/height are determined by input caps, not pad properties
        g_object_set(mixer_sink_pad, "xpos", xpos, "ypos", ypos, "zorder", layer,
                     "alpha", 1.0, NULL);

        // Store reference to pad for later use when playback bins are added
        // We keep our reference - the videomixer also holds one internally
        p->cell_sink_pads[layer - 1] = mixer_sink_pad;

        LOG_DEBUG("Pre-configured sink pad for layer %d (xpos=%d, ypos=%d, zorder=%d)", layer,
                  xpos, ypos, layer);
    }

    // Link videomixer → videoconvert → composite_caps → osxvideosink
    if (!gst_element_link_many(p->videomixer, p->videoconvert, p->composite_caps, p->osxvideosink, NULL)) {
        LOG_ERROR("Failed to link videomixer → videoconvert → composite_caps → osxvideosink");
        gst_bin_remove_many(GST_BIN(p->pipeline), p->osxvideosink, p->composite_caps, p->videoconvert, p->videomixer,
                            p->live_caps, live_convert, live_scale, p->live_queue, p->live_tee, p->camera_source, NULL);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Get bus and set up message handling
    p->bus = gst_element_get_bus(p->pipeline);
    if (!p->bus) {
        LOG_ERROR("Failed to get pipeline bus");
        gst_bin_remove_many(GST_BIN(p->pipeline), p->osxvideosink, p->composite_caps, p->videoconvert, p->videomixer,
                            p->live_caps, live_convert, live_scale, p->live_queue, p->live_tee, p->camera_source, NULL);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    // Add bus watch for message handling
    gst_bus_add_watch(p->bus, pipeline_bus_watch_handler, p);

    // Initialize record_bins, playback_bins, and preview_bins to NULL
    for (int i = 0; i < TOTAL_LAYERS; i++) {
        p->record_bins[i] = NULL;
        p->playback_bins[i] = NULL;
        p->playback_queues[i] = NULL;
        p->preview_bins[i] = NULL;
        p->preview_tee_pads[i] = NULL;
    }

    // Initialize message callback to NULL
    p->msg_callback = NULL;

    // Initialize window pointer to NULL (will be set later)
    p->window = NULL;

    // Set pipeline to READY state
    GstStateChangeReturn ret = gst_element_set_state(p->pipeline, GST_STATE_READY);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        LOG_ERROR("Failed to set pipeline to READY state");
        gst_bus_remove_watch(p->bus);
        gst_object_unref(p->bus);
        gst_bin_remove_many(GST_BIN(p->pipeline), p->osxvideosink, p->composite_caps, p->videoconvert, p->videomixer,
                            p->live_caps, live_convert, live_scale, p->live_queue, p->live_tee, p->camera_source, NULL);
        gst_object_unref(p->pipeline);
        g_free(p);
        return NULL;
    }

    LOG_INFO("GStreamer pipeline created successfully");
    return p;
}

gboolean pipeline_add_record_bin(Pipeline *p, int key_num)
{
    if (!p || !p->pipeline || key_num < 1 || key_num > TOTAL_LAYERS) {
        LOG_ERROR("Invalid pipeline or key_num=%d", key_num);
        return FALSE;
    }

    int bin_index = key_num - 1;

    // Check if bin already exists
    if (p->record_bins[bin_index]) {
        LOG_WARNING("Record bin for key %d already exists", key_num);
        return FALSE;
    }

    // Create record bin with max 60 frames (~2 seconds at 30fps)
    RecordBin *rbin = record_bin_create(key_num, 60, NULL);
    if (!rbin) {
        LOG_ERROR("Failed to create record bin for key %d", key_num);
        return FALSE;
    }

    // Get the bin's GStreamer element container
    GstElement *bin_element = rbin->bin;
    if (!bin_element) {
        LOG_ERROR("Record bin has NULL bin element for key %d", key_num);
        record_bin_cleanup(rbin);
        return FALSE;
    }

    // Add bin to the pipeline
    gst_bin_add(GST_BIN(p->pipeline), bin_element);

    // Link tee to the record bin
    // Request a new pad from the tee element using live_tee_request_pad
    // which handles deadlock prevention configuration
    GstPad *tee_src_pad = live_tee_request_pad(p->live_tee, key_num);
    if (!tee_src_pad) {
        LOG_ERROR("Failed to request source pad from tee for key %d", key_num);
        gst_bin_remove(GST_BIN(p->pipeline), bin_element);
        record_bin_cleanup(rbin);
        return FALSE;
    }

    // Get the sink pad of the record bin
    GstPad *bin_sink_pad = gst_element_get_static_pad(bin_element, "sink");
    if (!bin_sink_pad) {
        LOG_ERROR("Failed to get sink pad from record bin for key %d", key_num);
        gst_object_unref(tee_src_pad);
        gst_bin_remove(GST_BIN(p->pipeline), bin_element);
        record_bin_cleanup(rbin);
        return FALSE;
    }

    // Link tee output to record bin input
    GstPadLinkReturn link_ret = gst_pad_link(tee_src_pad, bin_sink_pad);
    gst_object_unref(bin_sink_pad);

    if (link_ret != GST_PAD_LINK_OK) {
        LOG_ERROR("Failed to link tee to record bin for key %d (link return: %d)", key_num,
                  link_ret);
        gst_object_unref(tee_src_pad);
        gst_bin_remove(GST_BIN(p->pipeline), bin_element);
        record_bin_cleanup(rbin);
        return FALSE;
    }

    // Store the tee pad reference in the record bin for later cleanup
    // This allows proper deallocation of the tee pad when the record bin is removed
    rbin->tee_pad = tee_src_pad;
    // Note: We keep the reference to tee_src_pad; it will be unreferenced during cleanup

    // Set the record bin to the same state as the pipeline if it's playing
    GstState current_state;
    gst_element_get_state(p->pipeline, &current_state, NULL, 0);
    if (current_state >= GST_STATE_READY) {
        gst_element_set_state(bin_element, GST_STATE_READY);
        if (current_state == GST_STATE_PLAYING) {
            gst_element_set_state(bin_element, GST_STATE_PLAYING);
        }
    }

    // Store the record bin in the pipeline
    p->record_bins[bin_index] = rbin;

    LOG_INFO("Added record bin for key %d to pipeline", key_num);
    return TRUE;
}

gboolean pipeline_remove_record_bin(Pipeline *p, int key_num)
{
    if (!p || !p->pipeline || key_num < 1 || key_num > TOTAL_LAYERS) {
        LOG_ERROR("Invalid pipeline or key_num=%d", key_num);
        return FALSE;
    }

    int bin_index = key_num - 1;

    // Check if bin exists
    if (!p->record_bins[bin_index]) {
        LOG_DEBUG("No record bin for key %d to remove", key_num);
        return TRUE; // Not an error if it doesn't exist
    }

    // Get the record bin
    RecordBin *rbin = p->record_bins[bin_index];

    // Get the GStreamer element from the record bin
    GstElement *bin = rbin->bin;
    if (!bin) {
        LOG_ERROR("Record bin has NULL bin element for key %d", key_num);
        p->record_bins[bin_index] = NULL;
        return FALSE;
    }

    // Stop recording if active
    record_bin_stop_recording(rbin);

    // Set bin to NULL state to stop processing
    gst_element_set_state(bin, GST_STATE_NULL);

    // Release the tee pad if it was stored
    // This properly releases the pad from the tee element using live_tee_release_pad
    if (rbin->tee_pad) {
        live_tee_release_pad(p->live_tee, rbin->tee_pad);
        rbin->tee_pad = NULL;
    } else {
        // Fallback: manually unlink in case tee_pad wasn't set
        LOG_DEBUG("Tee pad not stored; attempting manual unlink for key %d", key_num);
        GstPad *bin_sink_pad = gst_element_get_static_pad(bin, "sink");
        if (bin_sink_pad) {
            GstPad *peer_pad = gst_pad_get_peer(bin_sink_pad);
            if (peer_pad) {
                gst_pad_unlink(peer_pad, bin_sink_pad);
                gst_object_unref(peer_pad);
            }
            gst_object_unref(bin_sink_pad);
        }
    }

    // Remove bin from pipeline
    gst_bin_remove(GST_BIN(p->pipeline), bin);

    // Clean up the record bin
    record_bin_cleanup(rbin);
    p->record_bins[bin_index] = NULL;

    LOG_INFO("Removed record bin for key %d", key_num);
    return TRUE;
}

gboolean pipeline_add_playback_bin(Pipeline *p, int cell_num, guint64 duration_us)
{
    if (!p || !p->pipeline || !p->videomixer) {
        LOG_ERROR("pipeline_add_playback_bin: Invalid pipeline structure");
        return FALSE;
    }

    if (cell_num < 1 || cell_num > TOTAL_LAYERS) {
        LOG_ERROR("pipeline_add_playback_bin: Invalid cell_num=%d (must be 1-20)", cell_num);
        return FALSE;
    }

    int bin_index = cell_num - 1; // Layer 1 → index 0, layer 20 → index 19

    // Check if bin already exists in this cell
    if (p->playback_bins[bin_index]) {
        LOG_WARNING("pipeline_add_playback_bin: Playback bin for cell %d already exists", cell_num);
        return FALSE;
    }

    // Note on API Design:
    // This function receives duration_us but not a RingBuffer directly.
    // The proper implementation requires a buffer to be passed from the caller
    // (typically from the playback manager or recording system).
    //
    // Since the function signature doesn't include a buffer parameter,
    // this implementation serves as an infrastructure point that:
    // 1. Validates the cell number and pipeline state
    // 2. Allocates space in the playback_bins array
    // 3. Is meant to be called in coordination with playback_create_bin()
    //
    // In a complete system, the playback bin would be created separately
    // using playback_create_bin(buffer, cell_num), and then added here.
    //
    // For this implementation, we create a placeholder that marks the cell
    // as allocated. The actual playback bin would be created and linked
    // by the caller in a subsequent step.

    // Allocate a placeholder to mark this cell as reserved
    // This will be replaced by the actual playback bin element
    p->playback_bins[bin_index] = (GstElement *) g_malloc0(sizeof(gpointer));
    if (!p->playback_bins[bin_index]) {
        LOG_ERROR("pipeline_add_playback_bin: Failed to allocate placeholder for cell %d",
                  cell_num);
        return FALSE;
    }

    // Log at INFO level to indicate playback infrastructure is ready
    LOG_INFO("pipeline_add_playback_bin: Allocated playback bin slot for layer %d "
             "(duration=%llu us); playback element will be added dynamically",
             cell_num, (unsigned long long) duration_us);

    return TRUE;
}

gboolean pipeline_remove_playback_bin(Pipeline *p, int cell_num)
{
    if (!p || !p->pipeline || !p->videomixer) {
        LOG_ERROR("pipeline_remove_playback_bin: Invalid pipeline structure");
        return FALSE;
    }

    if (cell_num < 1 || cell_num > TOTAL_LAYERS) {
        LOG_ERROR("pipeline_remove_playback_bin: Invalid cell_num=%d (must be 1-20)", cell_num);
        return FALSE;
    }

    int bin_index = cell_num - 1; // Layer 1 → index 0, layer 20 → index 19

    // Check if bin exists
    if (!p->playback_bins[bin_index]) {
        LOG_DEBUG("pipeline_remove_playback_bin: No playback bin for layer %d to remove", cell_num);
        return TRUE; // Not an error if it doesn't exist
    }

    GstElement *bin = p->playback_bins[bin_index];

    // Check if this is a real GStreamer element or a placeholder
    // Real playback bins will be valid GStreamer elements (bin containers with appsrc, queue)
    // Placeholders are malloc'd gpointers
    if (GST_IS_ELEMENT(bin)) {
        // This is a real GStreamer element; safely remove it

        // Set the element to NULL state to stop processing
        GstState current_state;
        GstStateChangeReturn ret = gst_element_get_state(bin, &current_state, NULL, 0);
        if (ret != GST_STATE_CHANGE_FAILURE && current_state != GST_STATE_NULL) {
            gst_element_set_state(bin, GST_STATE_NULL);
        }

        // Unlink from the videomixer
        // The playback bin source pad should be linked to a videomixer sink pad
        GstPad *bin_src_pad = gst_element_get_static_pad(bin, "src");
        if (bin_src_pad) {
            GstPad *peer_pad = gst_pad_get_peer(bin_src_pad);
            if (peer_pad) {
                // Unlink the playback bin from the videomixer
                GstPadLinkReturn link_ret = gst_pad_unlink(bin_src_pad, peer_pad);
                if (link_ret != GST_PAD_LINK_OK) {
            LOG_WARNING("pipeline_remove_playback_bin: Failed to unlink playback bin "
                                "for layer %d from videomixer (link_ret=%d)",
                                cell_num, link_ret);
                }
                gst_object_unref(peer_pad);
            }
            gst_object_unref(bin_src_pad);
        }

        // Remove the bin from the pipeline
        gint remove_ret = gst_bin_remove(GST_BIN(p->pipeline), bin);
        if (!remove_ret) {
            LOG_WARNING("pipeline_remove_playback_bin: Failed to remove playback bin "
                        "for layer %d from pipeline",
                        cell_num);
        }

        // Unreference the element
        gst_object_unref(bin);

        LOG_INFO("pipeline_remove_playback_bin: Removed playback bin from pipeline for layer %d",
                 cell_num);
    } else {
        // This is a placeholder (malloc'd gpointer), just free it
        g_free(bin);
        LOG_DEBUG("pipeline_remove_playback_bin: Freed placeholder for layer %d", cell_num);
    }

    // Clean up associated queue if it exists
    if (p->playback_queues[bin_index]) {
        // The queue is part of the playback bin, so it's already been cleaned up
        // Just clear the reference
        p->playback_queues[bin_index] = NULL;
    }

    // Clear the bin reference
    p->playback_bins[bin_index] = NULL;

    LOG_INFO("pipeline_remove_playback_bin: Playback bin infrastructure removed for layer %d",
             cell_num);
    return TRUE;
}

gboolean pipeline_connect_live_preview(Pipeline *p, int cell_num)
{
    if (!p || !p->pipeline || !p->videomixer || !p->live_tee) {
        LOG_ERROR("pipeline_connect_live_preview: Invalid pipeline structure");
        return FALSE;
    }

    if (cell_num < 1 || cell_num > TOTAL_LAYERS) {
        LOG_ERROR("pipeline_connect_live_preview: Invalid cell_num=%d (must be 1-20)", cell_num);
        return FALSE;
    }

    int bin_index = cell_num - 1; // Layer 1 → index 0, layer 20 → index 19

    // Check if preview is already connected for this cell
    if (p->preview_bins[bin_index]) {
        LOG_WARNING("pipeline_connect_live_preview: Preview already connected for cell %d", cell_num);
        return TRUE; // Already connected, not an error
    }

    // Create a bin for the preview path: queue → videoconvert → videoscale → capsfilter
    gchar bin_name[64];
    snprintf(bin_name, sizeof(bin_name), "preview-bin-%d", cell_num);
    GstElement *preview_bin = gst_bin_new(bin_name);
    if (!preview_bin) {
        LOG_ERROR("pipeline_connect_live_preview: Failed to create preview bin for cell %d", cell_num);
        return FALSE;
    }

    // Create elements for the preview path
    snprintf(bin_name, sizeof(bin_name), "preview-queue-%d", cell_num);
    GstElement *queue = gst_element_factory_make("queue", bin_name);

    snprintf(bin_name, sizeof(bin_name), "preview-convert-%d", cell_num);
    GstElement *convert = gst_element_factory_make("videoconvert", bin_name);

    snprintf(bin_name, sizeof(bin_name), "preview-scale-%d", cell_num);
    GstElement *scale = gst_element_factory_make("videoscale", bin_name);

    snprintf(bin_name, sizeof(bin_name), "preview-caps-%d", cell_num);
    GstElement *caps = gst_element_factory_make("capsfilter", bin_name);

    if (!queue || !convert || !scale || !caps) {
        LOG_ERROR("pipeline_connect_live_preview: Failed to create elements for cell %d", cell_num);
        if (queue) gst_object_unref(queue);
        if (convert) gst_object_unref(convert);
        if (scale) gst_object_unref(scale);
        if (caps) gst_object_unref(caps);
        gst_object_unref(preview_bin);
        return FALSE;
    }

    // Configure queue for low latency
    g_object_set(queue, "max-size-buffers", 2, "leaky", 2, NULL); // downstream leaky

    // Configure capsfilter for cell size (320x180 I420)
    GstCaps *cell_caps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, "I420",
        "width", G_TYPE_INT, CELL_WIDTH_PX,
        "height", G_TYPE_INT, CELL_HEIGHT_PX,
        NULL);
    g_object_set(caps, "caps", cell_caps, NULL);
    gst_caps_unref(cell_caps);

    // Add elements to bin
    gst_bin_add_many(GST_BIN(preview_bin), queue, convert, scale, caps, NULL);

    // Link elements: queue → convert → scale → caps
    if (!gst_element_link_many(queue, convert, scale, caps, NULL)) {
        LOG_ERROR("pipeline_connect_live_preview: Failed to link elements for cell %d", cell_num);
        gst_object_unref(preview_bin);
        return FALSE;
    }

    // Create ghost pads for the bin
    GstPad *queue_sink = gst_element_get_static_pad(queue, "sink");
    GstPad *bin_sink = gst_ghost_pad_new("sink", queue_sink);
    gst_element_add_pad(preview_bin, bin_sink);
    gst_object_unref(queue_sink);

    GstPad *caps_src = gst_element_get_static_pad(caps, "src");
    GstPad *bin_src = gst_ghost_pad_new("src", caps_src);
    gst_element_add_pad(preview_bin, bin_src);
    gst_object_unref(caps_src);

    // Add bin to pipeline
    gst_bin_add(GST_BIN(p->pipeline), preview_bin);

    // Request a pad from the tee
    GstPad *tee_pad = gst_element_request_pad_simple(p->live_tee, "src_%u");
    if (!tee_pad) {
        LOG_ERROR("pipeline_connect_live_preview: Failed to request tee pad for cell %d", cell_num);
        gst_bin_remove(GST_BIN(p->pipeline), preview_bin);
        return FALSE;
    }

    // Link tee to preview bin
    GstPad *preview_sink = gst_element_get_static_pad(preview_bin, "sink");
    if (gst_pad_link(tee_pad, preview_sink) != GST_PAD_LINK_OK) {
        LOG_ERROR("pipeline_connect_live_preview: Failed to link tee to preview bin for cell %d", cell_num);
        gst_object_unref(preview_sink);
        gst_element_release_request_pad(p->live_tee, tee_pad);
        gst_object_unref(tee_pad);
        gst_bin_remove(GST_BIN(p->pipeline), preview_bin);
        return FALSE;
    }
    gst_object_unref(preview_sink);

    // Request sink pad from compositor for this cell
    GstPad *mixer_sink = gst_element_request_pad_simple(p->videomixer, "sink_%u");
    if (!mixer_sink) {
        LOG_ERROR("pipeline_connect_live_preview: Failed to request mixer sink pad for cell %d", cell_num);
        gst_element_release_request_pad(p->live_tee, tee_pad);
        gst_object_unref(tee_pad);
        gst_bin_remove(GST_BIN(p->pipeline), preview_bin);
        return FALSE;
    }

    // Configure mixer sink pad position for this layer
    int xpos = 0;
    int ypos = 0;
    if (!pipeline_layer_position(cell_num, &xpos, &ypos)) {
        LOG_ERROR("pipeline_connect_live_preview: Failed to compute position for layer %d",
                  cell_num);
        gst_object_unref(mixer_sink);
        gst_element_release_request_pad(p->live_tee, tee_pad);
        gst_object_unref(tee_pad);
        gst_bin_remove(GST_BIN(p->pipeline), preview_bin);
        return FALSE;
    }
    g_object_set(mixer_sink,
                 "xpos", xpos,
                 "ypos", ypos,
                 "width", CELL_WIDTH_PX,
                 "height", CELL_HEIGHT_PX,
                 "zorder", cell_num, // Higher zorder for preview
                 "alpha", 1.0,
                 NULL);

    // Link preview bin to compositor
    GstPad *preview_src = gst_element_get_static_pad(preview_bin, "src");
    if (gst_pad_link(preview_src, mixer_sink) != GST_PAD_LINK_OK) {
        LOG_ERROR("pipeline_connect_live_preview: Failed to link preview to mixer for cell %d", cell_num);
        gst_object_unref(preview_src);
        gst_object_unref(mixer_sink);
        gst_element_release_request_pad(p->live_tee, tee_pad);
        gst_object_unref(tee_pad);
        gst_bin_remove(GST_BIN(p->pipeline), preview_bin);
        return FALSE;
    }
    gst_object_unref(preview_src);
    gst_object_unref(mixer_sink);

    // Set the preview bin to PLAYING state
    gst_element_set_state(preview_bin, GST_STATE_PLAYING);

    // Store references
    p->preview_bins[bin_index] = preview_bin;
    p->preview_tee_pads[bin_index] = tee_pad;

    LOG_INFO("pipeline_connect_live_preview: Connected live preview to layer %d (xpos=%d, ypos=%d)",
             cell_num, xpos, ypos);
    return TRUE;
}

gboolean pipeline_disconnect_live_preview(Pipeline *p, int cell_num)
{
    if (!p || !p->pipeline) {
        LOG_ERROR("pipeline_disconnect_live_preview: Invalid pipeline structure");
        return FALSE;
    }

    if (cell_num < 1 || cell_num > TOTAL_LAYERS) {
        LOG_ERROR("pipeline_disconnect_live_preview: Invalid cell_num=%d (must be 1-20)", cell_num);
        return FALSE;
    }

    int bin_index = cell_num - 1;

    // Check if preview exists
    if (!p->preview_bins[bin_index]) {
        LOG_DEBUG("pipeline_disconnect_live_preview: No preview for layer %d", cell_num);
        return TRUE; // Nothing to disconnect
    }

    GstElement *preview_bin = p->preview_bins[bin_index];
    GstPad *tee_pad = p->preview_tee_pads[bin_index];

    // Set bin to NULL state
    gst_element_set_state(preview_bin, GST_STATE_NULL);

    // Unlink from tee
    if (tee_pad) {
        GstPad *preview_sink = gst_element_get_static_pad(preview_bin, "sink");
        if (preview_sink) {
            gst_pad_unlink(tee_pad, preview_sink);
            gst_object_unref(preview_sink);
        }
        gst_element_release_request_pad(p->live_tee, tee_pad);
        gst_object_unref(tee_pad);
    }

    // Unlink from compositor
    GstPad *preview_src = gst_element_get_static_pad(preview_bin, "src");
    if (preview_src) {
        GstPad *mixer_sink = gst_pad_get_peer(preview_src);
        if (mixer_sink) {
            gst_pad_unlink(preview_src, mixer_sink);
            gst_element_release_request_pad(p->videomixer, mixer_sink);
            gst_object_unref(mixer_sink);
        }
        gst_object_unref(preview_src);
    }

    // Remove bin from pipeline
    gst_bin_remove(GST_BIN(p->pipeline), preview_bin);

    // Clear references
    p->preview_bins[bin_index] = NULL;
    p->preview_tee_pads[bin_index] = NULL;

    LOG_INFO("pipeline_disconnect_live_preview: Disconnected live preview from layer %d",
             cell_num);
    return TRUE;
}

gboolean pipeline_set_state(Pipeline *p, GstState state)
{
    if (!p || !p->pipeline) {
        LOG_ERROR("Invalid pipeline");
        app_log_error(APP_ERROR_PIPELINE_STATE_CHANGE_FAILED, "Pipeline is not initialized");
        return FALSE;
    }

    /* Get current state for error recovery context */
    GstState current_state;
    gst_element_get_state(p->pipeline, &current_state, NULL, 0);

    LOG_INFO("Pipeline state transition: %s → %s", gst_element_state_get_name(current_state),
             gst_element_state_get_name(state));

    /* Use state change with deadlock detection (10 second timeout) */
    gboolean change_result =
        gstreamer_error_handler_set_state_with_detection(p->pipeline, state, 10000);

    if (change_result) {
        LOG_INFO("State change to %s succeeded or is in progress",
                 gst_element_state_get_name(state));
        return TRUE;
    }

    LOG_ERROR("State change to %s failed - attempting recovery", gst_element_state_get_name(state));

    /* Attempt error recovery if state change failed */
    gboolean recovery_result =
        pipeline_error_attempt_recovery(p->pipeline, state, current_state, NULL);

    if (recovery_result) {
        LOG_INFO("Pipeline recovery successful");
        app_log_error(APP_ERROR_PIPELINE_STATE_CHANGE_FAILED,
                      "State change failed but recovered to a stable state");
        return TRUE; /* Recovery was successful, operation can continue */
    }

    LOG_ERROR("Pipeline state change failed and recovery unsuccessful");
    app_log_error(APP_ERROR_PIPELINE_STATE_CHANGE_FAILED,
                  "Failed to transition pipeline to %s state; recovery failed",
                  gst_element_state_get_name(state));

    return FALSE;
}

GstState pipeline_get_state(Pipeline *p)
{
    if (!p || !p->pipeline) {
        LOG_ERROR("Invalid pipeline");
        return GST_STATE_NULL;
    }

    GstState state;
    gst_element_get_state(p->pipeline, &state, NULL, GST_CLOCK_TIME_NONE);
    return state;
}

void pipeline_set_message_callback(Pipeline *p, PipelineMessageCallback cb)
{
    if (!p) {
        LOG_ERROR("Invalid pipeline");
        return;
    }

    p->msg_callback = cb;
    LOG_DEBUG("Message callback %s", cb ? "registered" : "unregistered");
}

void pipeline_set_window(Pipeline *p, gpointer window)
{
    if (!p) {
        LOG_ERROR("Invalid pipeline");
        return;
    }

    p->window = window;
    LOG_DEBUG("Window associated with pipeline: %p", window);
}

void pipeline_cleanup(Pipeline *p)
{
    if (!p) {
        return; // Safe to call with NULL
    }

    // Stop the pipeline
    if (p->pipeline) {
        gst_element_set_state(p->pipeline, GST_STATE_NULL);
    }

    // Remove bus watch and unref bus
    if (p->bus) {
        gst_bus_remove_watch(p->bus);
        gst_object_unref(p->bus);
    }

    // Remove and unref record bins
    for (int i = 0; i < TOTAL_LAYERS; i++) {
        if (p->record_bins[i]) {
            RecordBin *rbin = p->record_bins[i];
            GstElement *bin = rbin->bin;
            if (bin) {
                gst_bin_remove(GST_BIN(p->pipeline), bin);
            }
            record_bin_cleanup(rbin);
            p->record_bins[i] = NULL;
        }
    }

    // Remove and unref playback bins
    for (int i = 0; i < TOTAL_LAYERS; i++) {
        if (p->playback_bins[i]) {
            gst_bin_remove(GST_BIN(p->pipeline), p->playback_bins[i]);
            gst_object_unref(p->playback_bins[i]);
            p->playback_bins[i] = NULL;
        }

        if (p->playback_queues[i]) {
            gst_object_unref(p->playback_queues[i]);
            p->playback_queues[i] = NULL;
        }
    }

    // Unref pipeline and static elements
    if (p->pipeline) {
        gst_object_unref(p->pipeline);
        p->pipeline = NULL;
    }

    // Unref camera source (we incremented ref count in pipeline_create)
    if (p->camera_source) {
        gst_object_unref(p->camera_source);
        p->camera_source = NULL;
    }

    // Free the Pipeline structure
    g_free(p);

    LOG_INFO("Pipeline cleaned up successfully");
}
