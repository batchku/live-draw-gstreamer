/**
 * @file playback_bin.c
 * @brief GStreamer playback bin implementation for emitting palindrome-looped video.
 *
 * Implements playback bins that emit video frames from a recorded buffer
 * in palindrome sequence (forward then reverse then repeat).
 * Uses GStreamer's appsrc element with need-data callbacks to emit frames
 * on-demand for the videomixer.
 *
 * Frame emission flow:
 * 1. Videomixer needs a frame from this playback bin
 * 2. Appsrc triggers need-data callback
 * 3. Callback calls playback_get_next_frame() to get current frame
 * 4. Frame is pushed to appsrc via gst_app_src_push_buffer()
 * 5. Callback calls playback_advance_frame() to step the palindrome
 * 6. Next frame request continues the sequence
 */

#include "playback_bin.h"
#include "../gstreamer/gst_elements.h"
#include "../gstreamer/performance_config.h"
#include "../utils/logging.h"
#include <glib.h>
#include <gst/app/gstappsrc.h>
#include <stdio.h>
#include <string.h>

/**
 * Callback function for appsrc need-data signal.
 *
 * This is triggered when the appsrc needs more buffers to deliver downstream.
 * It pulls the next frame from the PlaybackLoop and pushes it to the appsrc.
 * The playback loop is then advanced for the next frame.
 *
 * @param source    The GstAppSrc element
 * @param length    Number of bytes needed (unused)
 * @param user_data Pointer to the PlaybackBin structure
 */
static void playback_bin_need_data_callback(GstAppSrc *source G_GNUC_UNUSED,
                                            guint length G_GNUC_UNUSED, gpointer user_data)
{
    PlaybackBin *pbin = (PlaybackBin *) user_data;

    if (!pbin || !pbin->playback_loop || !pbin->is_active) {
        return;
    }

    /* Get the next frame from the palindrome playback loop */
    GstBuffer *frame = playback_get_next_frame(pbin->playback_loop);
    if (!frame) {
        LOG_ERROR("Playback bin [cell %d]: Failed to get next frame", pbin->cell_number);
        return;
    }

    /* Copy and retimestamp the frame for monotonic playback */
    GstBuffer *out = gst_buffer_copy(frame);
    if (!out) {
        LOG_WARNING("Playback bin [cell %d]: Failed to copy buffer", pbin->cell_number);
        return;
    }

    GST_BUFFER_PTS(out) = pbin->next_pts;
    GST_BUFFER_DTS(out) = pbin->next_pts;
    GST_BUFFER_DURATION(out) = pbin->frame_duration;
    pbin->next_pts += pbin->frame_duration;

    /* Push the frame to the appsrc */
    GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(source), out);
    if (ret != GST_FLOW_OK) {
        LOG_WARNING("Playback bin [cell %d]: Failed to push buffer (flow return %d)",
                    pbin->cell_number, ret);
        return;
    }

    /* Update frame count */
    pbin->frame_count++;

    /* Advance to the next frame in the palindrome sequence */
    playback_advance_frame(pbin->playback_loop);
}

/**
 * Callback function for appsrc enough-data signal.
 *
 * This is triggered when the appsrc has buffered enough data.
 * We don't need to do anything special here; the source will ask for more
 * data via need-data when it's ready.
 *
 * @param source    The GstAppSrc element
 * @param user_data Pointer to the PlaybackBin structure
 */
static void playback_bin_enough_data_callback(GstAppSrc *source G_GNUC_UNUSED,
                                                  gpointer user_data G_GNUC_UNUSED)
{
    /* appsrc is buffered; no action needed */
}

PlaybackBin *playback_bin_create(int cell_number, RingBuffer *buffer, GstCaps *output_caps)
{
    if (cell_number < 1 || cell_number > TOTAL_LAYERS) {
        LOG_ERROR("Invalid cell_number: %d (must be 1-50)", cell_number);
        return NULL;
    }

    if (!buffer) {
        LOG_ERROR("playback_bin_create: buffer is NULL");
        return NULL;
    }

    /* Allocate PlaybackBin structure */
    PlaybackBin *pbin = (PlaybackBin *) g_malloc0(sizeof(PlaybackBin));
    if (!pbin) {
        LOG_ERROR("Failed to allocate PlaybackBin structure");
        return NULL;
    }

    /* Create bin container */
    gchar bin_name[64];
    snprintf(bin_name, sizeof(bin_name), "playback-bin-%d", cell_number);
    pbin->bin = gst_bin_new(bin_name);
    if (!pbin->bin) {
        LOG_ERROR("Failed to create GStreamer bin for cell %d", cell_number);
        g_free(pbin);
        return NULL;
    }

    /* Create appsrc element for emitting frames */
    snprintf(bin_name, sizeof(bin_name), "playback-src-%d", cell_number);
    pbin->appsrc = gst_element_factory_make("appsrc", bin_name);
    if (!pbin->appsrc) {
        LOG_ERROR("Failed to create appsrc element for cell %d", cell_number);
        gst_object_unref(pbin->bin);
        g_free(pbin);
        return NULL;
    }

    /* Configure appsrc for pulling frames */
    /* Prefer recorded buffer caps when available; fallback to common camera defaults */
    GstCaps *input_caps = NULL;
    if (buffer->caps) {
        gchar *caps_str = gst_caps_to_string(buffer->caps);
        input_caps = gst_caps_ref(buffer->caps);
        LOG_INFO("Playback bin [cell %d]: Using recorded caps %s",
                 cell_number, caps_str ? caps_str : "(null)");
        g_free(caps_str);
    } else {
        input_caps = gst_caps_new_simple("video/x-raw",
            "format", G_TYPE_STRING, "UYVY",
            "width", G_TYPE_INT, 1920,
            "height", G_TYPE_INT, 1080,
            "framerate", GST_TYPE_FRACTION, 30, 1,
            NULL);
        LOG_WARNING("Playback bin [cell %d]: Recorded caps not available; using UYVY 1920x1080",
                    cell_number);
    }

    g_object_set(G_OBJECT(pbin->appsrc),
                 "caps", input_caps,        /* Input format matches recorded frames */
                 "is-live", TRUE,           /* Live source; frames come at presentation time */
                 "do-timestamp", TRUE,      /* GStreamer applies timestamps */
                 "block", FALSE,            /* Don't block on pull, continue pushing */
                 "format", GST_FORMAT_TIME, /* Use time format for synchronization */
                 NULL);

    gst_caps_unref(input_caps);

    /* Create queue element for buffering frames */
    snprintf(bin_name, sizeof(bin_name), "playback-queue-%d", cell_number);
    pbin->queue = gst_elements_create_queue(bin_name);
    if (!pbin->queue) {
        LOG_ERROR("Failed to create queue for cell %d", cell_number);
        gst_object_unref(pbin->appsrc);
        gst_object_unref(pbin->bin);
        g_free(pbin);
        return NULL;
    }

    /* Apply performance optimization for playback queue (T-8.2: 120 fps stability) */
    /* Configure queue sizing for interpolated playback at 120 fps */
    PerformanceQueueConfig playback_queue_perf = performance_config_playback_queue();
    if (!performance_apply_queue_config(pbin->queue, &playback_queue_perf, "playback")) {
        LOG_WARNING("Failed to apply performance config to playback queue cell %d; using defaults",
                    cell_number);
        /* Continue anyway; basic queue functionality still works */
    }

    /* Create videoconvert element to convert from recorded format (UYVY) to output format */
    snprintf(bin_name, sizeof(bin_name), "playback-convert-%d", cell_number);
    GstElement *convert = gst_element_factory_make("videoconvert", bin_name);
    if (!convert) {
        LOG_ERROR("Failed to create videoconvert for cell %d", cell_number);
        gst_object_unref(pbin->queue);
        gst_object_unref(pbin->appsrc);
        gst_object_unref(pbin->bin);
        g_free(pbin);
        return NULL;
    }

    /* Create videoscale element to resize from recorded resolution to cell size */
    snprintf(bin_name, sizeof(bin_name), "playback-scale-%d", cell_number);
    GstElement *scale = gst_element_factory_make("videoscale", bin_name);
    if (!scale) {
        LOG_ERROR("Failed to create videoscale for cell %d", cell_number);
        gst_object_unref(convert);
        gst_object_unref(pbin->queue);
        gst_object_unref(pbin->appsrc);
        gst_object_unref(pbin->bin);
        g_free(pbin);
        return NULL;
    }

    /* Create capsfilter to enforce output format (I420 320x180) */
    snprintf(bin_name, sizeof(bin_name), "playback-caps-%d", cell_number);
    GstElement *capsfilter = gst_element_factory_make("capsfilter", bin_name);
    if (!capsfilter) {
        LOG_ERROR("Failed to create capsfilter for cell %d", cell_number);
        gst_object_unref(scale);
        gst_object_unref(convert);
        gst_object_unref(pbin->queue);
        gst_object_unref(pbin->appsrc);
        gst_object_unref(pbin->bin);
        g_free(pbin);
        return NULL;
    }

    /* Set output caps on the capsfilter */
    if (output_caps) {
        g_object_set(capsfilter, "caps", output_caps, NULL);
    }

    /* Add all elements to bin */
    gst_bin_add_many(GST_BIN(pbin->bin), pbin->appsrc, pbin->queue, convert, scale, capsfilter, NULL);

    /* Link elements: appsrc → queue → videoconvert → videoscale → capsfilter */
    if (!gst_element_link_many(pbin->appsrc, pbin->queue, convert, scale, capsfilter, NULL)) {
        LOG_ERROR("Failed to link playback bin elements for cell %d", cell_number);
        /* Elements are owned by bin, will be cleaned up when bin is unreferenced */
        gst_object_unref(pbin->bin);
        g_free(pbin);
        return NULL;
    }

    /* Expose source pad of the bin for external linkage to videomixer */
    /* Source pad: the bin's output is the capsfilter's source */
    GstPad *capsfilter_src = gst_element_get_static_pad(capsfilter, "src");
    GstPad *bin_src = gst_ghost_pad_new("src", capsfilter_src);
    if (!bin_src || !gst_element_add_pad(pbin->bin, bin_src)) {
        LOG_ERROR("Failed to expose source pad for playback bin cell %d", cell_number);
        if (bin_src)
            gst_object_unref(bin_src);
        gst_object_unref(capsfilter_src);
        gst_object_unref(pbin->bin);
        g_free(pbin);
        return NULL;
    }
    gst_object_unref(capsfilter_src); /* Ghost pad holds its own reference */

    /* Create playback loop from the recorded buffer */
    pbin->playback_loop = playback_loop_create(buffer);
    if (!pbin->playback_loop) {
        LOG_ERROR("Failed to create playback loop for cell %d", cell_number);
        gst_object_unref(pbin->queue);
        gst_object_unref(pbin->appsrc);
        gst_object_unref(pbin->bin);
        g_free(pbin);
        return NULL;
    }

    /* Connect appsrc callbacks for frame emission */
    /* need-data: called when appsrc needs buffers */
    g_signal_connect(pbin->appsrc, "need-data", (GCallback) playback_bin_need_data_callback, pbin);

    /* enough-data: called when appsrc has buffered enough */
    g_signal_connect(pbin->appsrc, "enough-data",
                     (GCallback) playback_bin_enough_data_callback, pbin);

    /* Initialize playback state */
    pbin->cell_number = cell_number;
    pbin->frame_count = 0;
    pbin->is_active = playback_is_playing(pbin->playback_loop);
    pbin->next_pts = 0;

    /* Derive frame duration from caps framerate if available */
    pbin->frame_duration = GST_SECOND / 30;
    if (input_caps) {
        GstStructure *s = gst_caps_get_structure(input_caps, 0);
        if (s) {
            gint num = 0, den = 1;
            if (gst_structure_get_fraction(s, "framerate", &num, &den) && num > 0 && den > 0) {
                pbin->frame_duration = gst_util_uint64_scale_int(GST_SECOND, den, num);
            }
        }
    }

    LOG_INFO("Created playback bin for cell %d (%u frames, %s)", cell_number,
             buffer_get_frame_count(buffer), pbin->is_active ? "active" : "inactive");

    return pbin;
}

gboolean playback_bin_is_active(PlaybackBin *pbin)
{
    if (!pbin) {
        return FALSE;
    }
    return pbin->is_active;
}

guint playback_bin_get_frame_count(PlaybackBin *pbin)
{
    if (!pbin) {
        return 0;
    }
    return pbin->frame_count;
}

void playback_bin_cleanup(PlaybackBin *pbin)
{
    if (!pbin) {
        return;
    }

    /* Disconnect signals before cleanup */
    if (pbin->appsrc) {
        g_signal_handlers_disconnect_by_func(
            pbin->appsrc, (gpointer) G_CALLBACK(playback_bin_need_data_callback), pbin);
        g_signal_handlers_disconnect_by_func(
            pbin->appsrc, (gpointer) G_CALLBACK(playback_bin_enough_data_callback), pbin);
    }

    /* Clean up playback loop */
    if (pbin->playback_loop) {
        playback_loop_cleanup(pbin->playback_loop);
        pbin->playback_loop = NULL;
    }

    /* Unreference bin and its elements */
    /* (Elements are unreferenced when bin is destroyed) */
    if (pbin->bin) {
        gst_object_unref(pbin->bin);
        pbin->bin = NULL;
    }

    /* Clear element references */
    pbin->appsrc = NULL;
    pbin->queue = NULL;

    /* Free the structure */
    g_free(pbin);

    LOG_DEBUG("Playback bin cleaned up");
}
