/**
 * @file e2e_coordinator.c
 * @brief End-to-end flow coordinator implementation
 *
 * Coordinates the complete recording→buffer→playback→display flow.
 * Manages the recording buffers for each cell and transitions between
 * recording and playback states.
 */

#include "e2e_coordinator.h"
#include "../gstreamer/pipeline_builder.h"
#include "../gstreamer/record_bin.h"
#include "../playback/playback_bin.h"
#include "../playback/playback_manager.h"
#include "../recording/buffer_manager.h"
#include "../recording/recording_state.h"
#include "../utils/logging.h"
#include "../utils/timing.h"
#include "app_config.h"
#include "app_context.h"
#include <glib.h>
#include <string.h>

/**
 * E2E Coordinator state
 * Maintains the recording buffers and playback bins for all cells
 * Note: RingBuffer is typedef'd in buffer_manager.h, PlaybackBin in playback_bin.h
 */
typedef struct {
    AppContext *app_ctx;        /* Reference to app context */
    void *recording_buffers[TOTAL_LAYERS];   /* Buffers for layers 1-50 */
    PlaybackBin *playback_bins[TOTAL_LAYERS]; /* Playback bins for layers 1-50 */
    guint64 recording_start_times[TOTAL_LAYERS]; /* Start time for each recording */
    gint active_recordings[TOTAL_LAYERS];        /* Which key is recording to each layer (-1 if none) */
} E2ECoordinator;

/* Global coordinator instance */
static E2ECoordinator *g_coordinator = NULL;

static gboolean e2e_layer_position(int layer_number, int *xpos, int *ypos)
{
    if (layer_number < 1 || layer_number > TOTAL_LAYERS || !xpos || !ypos) {
        return FALSE;
    }

    int layer_index = layer_number - 1;
    int col = (layer_index % LAYER_COLUMNS) + 1;
    int row = layer_index / LAYER_COLUMNS;

    *xpos = col * CELL_WIDTH_PX;
    *ypos = row * CELL_HEIGHT_PX;
    return TRUE;
}

/**
 * Initialize the E2E coordinator
 */
gboolean e2e_coordinator_init(void *app_ctx_ptr)
{
    AppContext *app_ctx = (AppContext *) app_ctx_ptr;
    if (!app_ctx) {
        LOG_ERROR("e2e_coordinator_init: app_ctx is NULL");
        return FALSE;
    }

    LOG_DEBUG("Initializing E2E coordinator...");

    g_coordinator = (E2ECoordinator *) g_malloc0(sizeof(E2ECoordinator));
    if (!g_coordinator) {
        LOG_ERROR("e2e_coordinator_init: Failed to allocate coordinator");
        return FALSE;
    }

    g_coordinator->app_ctx = app_ctx;

    /* Initialize all cells to empty/not-recording */
    for (int i = 0; i < TOTAL_LAYERS; i++) {
        g_coordinator->recording_buffers[i] = NULL;
        g_coordinator->playback_bins[i] = NULL;
        g_coordinator->recording_start_times[i] = 0;
        g_coordinator->active_recordings[i] = -1;
    }

    LOG_INFO("E2E coordinator initialized successfully");
    return TRUE;
}

/**
 * Handle key press: Start recording
 */
static void handle_key_press(int key_number)
{
    if (key_number < 1 || key_number > TOTAL_LAYERS) {
        LOG_DEBUG("handle_key_press: Invalid key_number %d", key_number);
        return;
    }

    if (!g_coordinator || !g_coordinator->app_ctx) {
        LOG_ERROR("handle_key_press: Coordinator not initialized");
        return;
    }

    RecordingState *rec_state = g_coordinator->app_ctx->recording_state;
    if (!rec_state) {
        LOG_ERROR("handle_key_press: Recording state not available");
        return;
    }

    LOG_DEBUG("handle_key_press: Key %d pressed, starting recording", key_number);

    if (recording_is_recording(rec_state, key_number)) {
        return;
    }

    /* Mark the key as recording in the recording state manager */
    recording_on_key_press(rec_state, key_number);

    /* Capture the start time for this recording */
    guint64 now = timing_get_time_us();
    int key_index = key_number - 1;
    g_coordinator->recording_start_times[key_index] = now;

    /* Signal the record bin to start capturing */
    Pipeline *pipeline = g_coordinator->app_ctx->gst_pipeline;
    if (!pipeline) {
        LOG_ERROR("handle_key_press: Pipeline not available");
        return;
    }

    /* If this layer already has playback, remove it before recording again */
    if (g_coordinator->playback_bins[key_index]) {
        PlaybackBin *existing = g_coordinator->playback_bins[key_index];
        if (existing->bin) {
            GstPad *src_pad = gst_element_get_static_pad(existing->bin, "src");
            if (src_pad) {
                GstPad *peer_pad = gst_pad_get_peer(src_pad);
                if (peer_pad) {
                    gst_pad_unlink(src_pad, peer_pad);
                    gst_element_release_request_pad(pipeline->videomixer, peer_pad);
                    gst_object_unref(peer_pad);
                }
                gst_object_unref(src_pad);
            }

            gst_element_set_state(existing->bin, GST_STATE_NULL);
            gst_bin_remove(GST_BIN(pipeline->pipeline), existing->bin);
        }
        playback_bin_cleanup(existing);
        g_coordinator->playback_bins[key_index] = NULL;
    }

    if (g_coordinator->recording_buffers[key_index]) {
        buffer_cleanup(g_coordinator->recording_buffers[key_index]);
        g_coordinator->recording_buffers[key_index] = NULL;
    }

    /* Recreate record bin to ensure a fresh ring buffer */
    if (pipeline->record_bins[key_index]) {
        pipeline_remove_record_bin(pipeline, key_number);
    }
    if (!pipeline_add_record_bin(pipeline, key_number)) {
        LOG_ERROR("handle_key_press: Failed to create record bin for key %d", key_number);
        return;
    }

    /* Start capturing on the record bin */
    RecordBin *rbin = pipeline->record_bins[key_index];
    if (!record_bin_start_recording(rbin)) {
        LOG_ERROR("handle_key_press: Failed to start recording on record bin for key %d", key_number);
        return;
    }

    /* Connect live preview to show recording is happening */
    int layer_num = key_number; // Layer matches key number
    if (!pipeline_connect_live_preview(pipeline, layer_num)) {
        LOG_WARNING("handle_key_press: Failed to connect live preview for key %d", key_number);
        // Continue anyway - recording still works
    }

    LOG_DEBUG("handle_key_press: Recording started for key %d at %llu us", key_number, now);
}

/**
 * Handle key release: Stop recording and create playback
 */
static void handle_key_release(int key_number)
{
    if (key_number < 1 || key_number > TOTAL_LAYERS) {
        LOG_DEBUG("handle_key_release: Invalid key_number %d", key_number);
        return;
    }

    if (!g_coordinator || !g_coordinator->app_ctx) {
        LOG_ERROR("handle_key_release: Coordinator not initialized");
        return;
    }

    RecordingState *rec_state = g_coordinator->app_ctx->recording_state;
    if (!rec_state) {
        LOG_ERROR("handle_key_release: Recording state not available");
        return;
    }

    LOG_DEBUG("handle_key_release: Key %d released, stopping recording", key_number);

    /* Mark the key as not recording and get duration */
    recording_on_key_release(rec_state, key_number);
    guint64 duration_us = recording_get_duration(rec_state, key_number);

    int key_index = key_number - 1;

    /* Signal the record bin to stop capturing */
    Pipeline *pipeline = g_coordinator->app_ctx->gst_pipeline;
    if (!pipeline || !pipeline->record_bins[key_index]) {
        LOG_ERROR("handle_key_release: Record bin for key %d not available", key_number);
        return;
    }

    RecordBin *rbin = pipeline->record_bins[key_index];
    record_bin_stop_recording(rbin);

    /* Disconnect live preview now that recording is done */
    int layer_num = key_number; // Layer matches key number
    pipeline_disconnect_live_preview(pipeline, layer_num);

    /* Get the buffer containing the captured frames from the record bin */
    RingBuffer *recorded_buffer = record_bin_get_buffer(rbin);
    guint frame_count = recorded_buffer ? buffer_get_frame_count(recorded_buffer) : 0;
    LOG_DEBUG("handle_key_release: Captured %u frames for key %d", frame_count, key_number);

    if (!recorded_buffer || frame_count == 0) {
        LOG_WARNING("handle_key_release: No frames captured for key %d, skipping playback", key_number);
        return;
    }

    /* Assign this recording to the layer matching the key */
    gint cell_index = key_number - 1; /* Convert 1-20 to 0-19 */
    int cell_num = key_number;

    LOG_DEBUG("handle_key_release: Recording assigned to layer %d (index %d), duration: %llu us",
              cell_num, cell_index, duration_us);

    /* Transfer ownership of the recorded buffer from record_bin to coordinator.
     * This avoids creating a new empty buffer - we use the one with actual frames.
     * We must also clear the record_bin's reference to prevent double-free on cleanup. */
    g_coordinator->recording_buffers[cell_index] = recorded_buffer;
    rbin->ring_buffer = NULL; /* Transfer ownership - record_bin no longer owns this buffer */

    /* Create output caps for the playback bin (320x180 I420 to match cell size) */
    GstCaps *output_caps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, "I420",
        "width", G_TYPE_INT, CELL_WIDTH_PX,
        "height", G_TYPE_INT, CELL_HEIGHT_PX,
        NULL);

    /* Create playback bin with GStreamer elements (appsrc → queue → output) */
    PlaybackBin *pbin = playback_bin_create(cell_num, recorded_buffer, output_caps);
    gst_caps_unref(output_caps);

    if (!pbin) {
        LOG_ERROR("handle_key_release: Failed to create playback bin for cell %d", cell_num);
        return;
    }

    g_coordinator->playback_bins[cell_index] = pbin;
    g_coordinator->active_recordings[cell_index] = key_number;

    LOG_DEBUG("handle_key_release: Playback bin created for cell %d", cell_num);

    /* Add the playback bin to the GStreamer pipeline */
    gst_bin_add(GST_BIN(pipeline->pipeline), pbin->bin);

    /* Request a fresh sink pad from the videomixer (like pipeline_connect_live_preview does) */
    GstPad *mixer_sink = gst_element_request_pad_simple(pipeline->videomixer, "sink_%u");
    if (!mixer_sink) {
        LOG_ERROR("handle_key_release: Failed to request mixer sink pad for cell %d", cell_num);
        gst_bin_remove(GST_BIN(pipeline->pipeline), pbin->bin);
        playback_bin_cleanup(pbin);
        g_coordinator->playback_bins[cell_index] = NULL;
        return;
    }

    /* Configure the mixer sink pad position for this layer */
    int xpos = 0;
    int ypos = 0;
    if (!e2e_layer_position(cell_num, &xpos, &ypos)) {
        LOG_ERROR("handle_key_release: Failed to compute position for layer %d", cell_num);
        gst_element_release_request_pad(pipeline->videomixer, mixer_sink);
        gst_object_unref(mixer_sink);
        gst_bin_remove(GST_BIN(pipeline->pipeline), pbin->bin);
        playback_bin_cleanup(pbin);
        g_coordinator->playback_bins[cell_index] = NULL;
        return;
    }
    g_object_set(mixer_sink,
                 "xpos", xpos,
                 "ypos", ypos,
                 "width", CELL_WIDTH_PX,
                 "height", CELL_HEIGHT_PX,
                 "zorder", cell_num,
                 "alpha", 1.0,
                 NULL);
    LOG_DEBUG("handle_key_release: Configured fresh sink pad for layer %d (xpos=%d, ypos=%d)",
              cell_num, xpos, ypos);

    /* Link the playback bin's source pad to the videomixer sink pad */
    GstPad *pbin_src = gst_element_get_static_pad(pbin->bin, "src");
    if (!pbin_src || gst_pad_link(pbin_src, mixer_sink) != GST_PAD_LINK_OK) {
        LOG_ERROR("handle_key_release: Failed to link playback bin to mixer for cell %d", cell_num);
        if (pbin_src) gst_object_unref(pbin_src);
        gst_element_release_request_pad(pipeline->videomixer, mixer_sink);
        gst_object_unref(mixer_sink);
        gst_bin_remove(GST_BIN(pipeline->pipeline), pbin->bin);
        playback_bin_cleanup(pbin);
        g_coordinator->playback_bins[cell_index] = NULL;
        return;
    }
    gst_object_unref(pbin_src);
    gst_object_unref(mixer_sink);

    /* Sync the playback bin state with the pipeline (set to PLAYING) */
    gst_element_sync_state_with_parent(pbin->bin);

    LOG_INFO("handle_key_release: Key %d recording complete, playback started in layer %d (xpos=%d)",
             key_number, cell_num, xpos);
}

/**
 * Handle keyboard event
 */
void e2e_on_key_event(int key_number, gboolean is_pressed)
{
    if (key_number == -1) {
        /* Quit key */
        if (is_pressed) {
            LOG_INFO("Quit key pressed, initiating shutdown");
            AppContext *ctx = app_context_get();
            if (ctx && ctx->main_loop) {
                g_main_loop_quit(ctx->main_loop);
            }
        }
        return;
    }

    if (is_pressed) {
        handle_key_press(key_number);
    } else {
        handle_key_release(key_number);
    }
}

/**
 * Get the recording buffer for a specific cell
 */
void *e2e_get_recording_buffer(int cell_num)
{
    if (cell_num < 1 || cell_num > TOTAL_LAYERS) {
        LOG_ERROR("e2e_get_recording_buffer: Invalid layer %d", cell_num);
        return NULL;
    }

    if (!g_coordinator) {
        LOG_ERROR("e2e_get_recording_buffer: Coordinator not initialized");
        return NULL;
    }

    int cell_index = cell_num - 1; /* Convert 1-20 to 0-19 */
    return g_coordinator->recording_buffers[cell_index];
}

/**
 * Cleanup the E2E coordinator
 */
void e2e_coordinator_cleanup(void)
{
    if (!g_coordinator) {
        return;
    }

    LOG_DEBUG("Cleaning up E2E coordinator...");

    /* Clean up all playback bins */
    for (int i = 0; i < TOTAL_LAYERS; i++) {
        if (g_coordinator->playback_bins[i]) {
            playback_bin_cleanup(g_coordinator->playback_bins[i]);
            g_coordinator->playback_bins[i] = NULL;
        }
    }

    /* Clean up all recording buffers */
    for (int i = 0; i < TOTAL_LAYERS; i++) {
        if (g_coordinator->recording_buffers[i]) {
            buffer_cleanup(g_coordinator->recording_buffers[i]);
            g_coordinator->recording_buffers[i] = NULL;
        }
    }

    g_free(g_coordinator);
    g_coordinator = NULL;

    LOG_INFO("E2E coordinator cleanup complete");
}
