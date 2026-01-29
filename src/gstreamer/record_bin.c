/**
 * @file record_bin.c
 * @brief GStreamer recording bin implementation for capturing video to GPU buffers.
 *
 * Implements recording bins that capture video frames from the live stream
 * and store them in GPU memory ring buffers. Uses GStreamer pad probes to
 * intercept frames without modifying the element chain.
 *
 * Frame capture flow:
 * 1. Live tee → queue (sink pad has probe attached)
 * 2. When frame arrives at queue, probe intercepts it
 * 3. If recording is active, frame is written to ring buffer
 * 4. Frame passes through queue, capsfilter, and fakesink
 * 5. Fakesink discards the buffer (we keep it in ring buffer)
 */

#include "record_bin.h"
#include "../utils/logging.h"
#include "gst_elements.h"
#include "performance_config.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>

/**
 * Pad probe callback for intercepting video frames during recording.
 *
 * This callback is attached to the queue's sink pad and fires whenever
 * a buffer enters the queue. If recording is active, the buffer is
 * written to the ring buffer for storage. The buffer then continues
 * through the pipeline normally.
 *
 * @param pad       The GStreamer pad where the probe is attached
 * @param info      Probe info containing the buffer
 * @param user_data Pointer to the RecordBin structure
 * @return          GST_PAD_PROBE_OK to allow the buffer to continue
 */
static GstPadProbeReturn record_bin_queue_probe(GstPad *pad G_GNUC_UNUSED, GstPadProbeInfo *info,
                                                gpointer user_data)
{
    RecordBin *rbin = (RecordBin *) user_data;

    if (!rbin || !info) {
        return GST_PAD_PROBE_OK; // Let buffer through even if something is wrong
    }

    // Check if this is a buffer event
    if (GST_PAD_PROBE_INFO_TYPE(info) & GST_PAD_PROBE_TYPE_BUFFER) {
        GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);

        if (buffer && rbin->is_recording) {
            /* Capture caps once when the first buffer arrives (for playback appsrc) */
            if (rbin->ring_buffer && !rbin->ring_buffer->caps) {
                GstCaps *current_caps = gst_pad_get_current_caps(pad);
                if (current_caps) {
                    gchar *caps_str = gst_caps_to_string(current_caps);
                    rbin->ring_buffer->caps = gst_caps_ref(current_caps);
                    LOG_INFO("Record bin [key %d]: Captured caps %s",
                             rbin->key_number, caps_str ? caps_str : "(null)");
                    g_free(caps_str);
                    gst_caps_unref(current_caps);
                } else {
                    LOG_DEBUG("Record bin [key %d]: No caps available yet", rbin->key_number);
                }
            }

            // Only capture if actively recording
            LOG_DEBUG("Record bin [key %d]: Capturing frame %u", rbin->key_number,
                      rbin->ring_buffer ? buffer_get_frame_count(rbin->ring_buffer) : 0);

            // Write frame to ring buffer
            if (rbin->ring_buffer) {
                buffer_write_frame(rbin->ring_buffer, buffer);
            }
        }
    }

    // Allow buffer to continue through the pipeline
    return GST_PAD_PROBE_OK;
}

RecordBin *record_bin_create(int key_number, guint max_frames, GstCaps *caps)
{
    if (key_number < 1 || key_number > TOTAL_LAYERS) {
        LOG_ERROR("Invalid key_number: %d (must be 1-20)", key_number);
        return NULL;
    }

    if (max_frames == 0) {
        LOG_ERROR("max_frames must be > 0");
        return NULL;
    }

    // Allocate RecordBin structure
    RecordBin *rbin = (RecordBin *) g_malloc0(sizeof(RecordBin));
    if (!rbin) {
        LOG_ERROR("Failed to allocate RecordBin structure");
        return NULL;
    }

    // Create bin container
    gchar bin_name[64];
    snprintf(bin_name, sizeof(bin_name), "record-bin-%d", key_number);
    rbin->bin = gst_bin_new(bin_name);
    if (!rbin->bin) {
        LOG_ERROR("Failed to create GStreamer bin for key %d", key_number);
        g_free(rbin);
        return NULL;
    }

    // Create queue element for buffering frames
    snprintf(bin_name, sizeof(bin_name), "record-queue-%d", key_number);
    rbin->queue = gst_elements_create_queue(bin_name);
    if (!rbin->queue) {
        LOG_ERROR("Failed to create queue for key %d", key_number);
        gst_object_unref(rbin->bin);
        g_free(rbin);
        return NULL;
    }

    // Apply performance optimization for recording queue (T-8.2: 120 fps stability)
    // Configure queue sizing for high-frame-rate capture with minimal CPU overhead
    PerformanceQueueConfig record_queue_perf = performance_config_recording_queue();
    if (!performance_apply_queue_config(rbin->queue, &record_queue_perf, "recording")) {
        LOG_WARNING("Failed to apply performance config to recording queue key %d; using defaults",
                    key_number);
        // Continue anyway; basic queue functionality still works
    }

    // Create capsfilter for format negotiation
    snprintf(bin_name, sizeof(bin_name), "record-caps-%d", key_number);
    rbin->capsfilter = gst_elements_create_capsfilter(bin_name);
    if (!rbin->capsfilter) {
        LOG_ERROR("Failed to create capsfilter for key %d", key_number);
        gst_object_unref(rbin->queue);
        gst_object_unref(rbin->bin);
        g_free(rbin);
        return NULL;
    }

    // Create fakesink for consuming buffers
    snprintf(bin_name, sizeof(bin_name), "record-sink-%d", key_number);
    rbin->fakesink = gst_elements_create_fakesink(bin_name);
    if (!rbin->fakesink) {
        LOG_ERROR("Failed to create fakesink for key %d", key_number);
        gst_object_unref(rbin->capsfilter);
        gst_object_unref(rbin->queue);
        gst_object_unref(rbin->bin);
        g_free(rbin);
        return NULL;
    }

    // Add elements to bin
    gst_bin_add_many(GST_BIN(rbin->bin), rbin->queue, rbin->capsfilter, rbin->fakesink, NULL);

    // Link elements: queue → capsfilter → fakesink
    if (!gst_element_link_many(rbin->queue, rbin->capsfilter, rbin->fakesink, NULL)) {
        LOG_ERROR("Failed to link elements in record bin for key %d", key_number);
        gst_object_unref(rbin->fakesink);
        gst_object_unref(rbin->capsfilter);
        gst_object_unref(rbin->queue);
        gst_object_unref(rbin->bin);
        g_free(rbin);
        return NULL;
    }

    // Expose sink and source pads of the bin for external linkage
    // Sink pad: the bin's sink should be the queue's sink
    GstPad *queue_sink = gst_element_get_static_pad(rbin->queue, "sink");
    GstPad *bin_sink = gst_ghost_pad_new("sink", queue_sink);
    if (!bin_sink || !gst_element_add_pad(rbin->bin, bin_sink)) {
        LOG_ERROR("Failed to expose sink pad for record bin key %d", key_number);
        if (bin_sink)
            gst_object_unref(bin_sink);
        gst_object_unref(queue_sink);
        gst_object_unref(rbin->fakesink);
        gst_object_unref(rbin->capsfilter);
        gst_object_unref(rbin->queue);
        gst_object_unref(rbin->bin);
        g_free(rbin);
        return NULL;
    }
    gst_object_unref(queue_sink); // Ghost pad holds its own reference

    // Note: Record bin is a sink bin - it receives frames, stores them in
    // ring buffer via pad probe, and fakesink discards them. No source
    // ghost pad is needed since nothing downstream receives from this bin.

    // Create ring buffer for storing captured frames
    rbin->ring_buffer = buffer_create(max_frames, caps);
    if (!rbin->ring_buffer) {
        LOG_ERROR("Failed to create ring buffer for key %d", key_number);
        gst_object_unref(rbin->fakesink);
        gst_object_unref(rbin->capsfilter);
        gst_object_unref(rbin->queue);
        gst_object_unref(rbin->bin);
        g_free(rbin);
        return NULL;
    }

    // Attach pad probe to queue's sink pad for frame capture
    rbin->probe_pad = gst_element_get_static_pad(rbin->queue, "sink");
    if (!rbin->probe_pad) {
        LOG_ERROR("Failed to get queue sink pad for key %d", key_number);
        buffer_cleanup(rbin->ring_buffer);
        gst_object_unref(rbin->fakesink);
        gst_object_unref(rbin->capsfilter);
        gst_object_unref(rbin->queue);
        gst_object_unref(rbin->bin);
        g_free(rbin);
        return NULL;
    }

    // Add the buffer probe to the sink pad
    rbin->probe_id =
        gst_pad_add_probe(rbin->probe_pad, GST_PAD_PROBE_TYPE_BUFFER, record_bin_queue_probe, rbin,
                          NULL); // No destroy notify

    if (rbin->probe_id == 0) {
        LOG_ERROR("Failed to attach pad probe to queue sink pad for key %d", key_number);
        gst_object_unref(rbin->probe_pad);
        buffer_cleanup(rbin->ring_buffer);
        gst_object_unref(rbin->fakesink);
        gst_object_unref(rbin->capsfilter);
        gst_object_unref(rbin->queue);
        gst_object_unref(rbin->bin);
        g_free(rbin);
        return NULL;
    }

    // Initialize recording state
    rbin->key_number = key_number;
    rbin->is_recording = FALSE;

    LOG_INFO("Created record bin for key %d (max %u frames)", key_number, max_frames);
    return rbin;
}

gboolean record_bin_start_recording(RecordBin *rbin)
{
    if (!rbin) {
        LOG_ERROR("record_bin_start_recording: rbin is NULL");
        return FALSE;
    }

    if (rbin->is_recording) {
        LOG_WARNING("Record bin for key %d is already recording", rbin->key_number);
        return TRUE; // Already recording, not an error
    }

    rbin->is_recording = TRUE;
    LOG_DEBUG("Started recording for key %d", rbin->key_number);
    return TRUE;
}

gboolean record_bin_stop_recording(RecordBin *rbin)
{
    if (!rbin) {
        LOG_ERROR("record_bin_stop_recording: rbin is NULL");
        return FALSE;
    }

    if (!rbin->is_recording) {
        LOG_DEBUG("Record bin for key %d is not recording", rbin->key_number);
        return TRUE; // Not recording, not an error
    }

    rbin->is_recording = FALSE;
    LOG_DEBUG("Stopped recording for key %d (captured %u frames)", rbin->key_number,
              rbin->ring_buffer ? buffer_get_frame_count(rbin->ring_buffer) : 0);
    return TRUE;
}

gboolean record_bin_is_recording(RecordBin *rbin)
{
    if (!rbin) {
        return FALSE;
    }
    return rbin->is_recording;
}

RingBuffer *record_bin_get_buffer(RecordBin *rbin)
{
    if (!rbin) {
        return NULL;
    }
    return rbin->ring_buffer;
}

gboolean record_bin_reset(RecordBin *rbin)
{
    if (!rbin || !rbin->ring_buffer) {
        LOG_ERROR("record_bin_reset: invalid record bin");
        return FALSE;
    }

    // Clean up the current ring buffer
    buffer_cleanup(rbin->ring_buffer);

    // Create a new ring buffer with the same capacity and caps
    GstCaps *old_caps = gst_caps_copy(rbin->ring_buffer->caps);
    guint capacity = rbin->ring_buffer->capacity;

    rbin->ring_buffer = buffer_create(capacity, old_caps);

    if (old_caps) {
        gst_caps_unref(old_caps);
    }

    if (!rbin->ring_buffer) {
        LOG_ERROR("Failed to reset ring buffer for key %d", rbin->key_number);
        return FALSE;
    }

    LOG_DEBUG("Reset record bin for key %d", rbin->key_number);
    return TRUE;
}

void record_bin_cleanup(RecordBin *rbin)
{
    if (!rbin) {
        return;
    }

    // Unreference tee pad if set
    // Note: The tee pad should have been released via live_tee_release_pad
    // during pipeline_remove_record_bin, but we clean it up here for safety
    if (rbin->tee_pad) {
        gst_object_unref(rbin->tee_pad);
        rbin->tee_pad = NULL;
    }

    // Remove pad probe from queue sink pad
    if (rbin->probe_pad && rbin->probe_id) {
        gst_pad_remove_probe(rbin->probe_pad, rbin->probe_id);
        gst_object_unref(rbin->probe_pad);
        rbin->probe_pad = NULL;
        rbin->probe_id = 0;
    }

    // Clean up ring buffer
    if (rbin->ring_buffer) {
        buffer_cleanup(rbin->ring_buffer);
        rbin->ring_buffer = NULL;
    }

    // Unreference bin and its elements
    // (Elements are unreferenced when bin is destroyed)
    if (rbin->bin) {
        gst_object_unref(rbin->bin);
        rbin->bin = NULL;
    }

    // Clear element references
    rbin->queue = NULL;
    rbin->capsfilter = NULL;
    rbin->fakesink = NULL;

    // Free the structure
    g_free(rbin);

    LOG_DEBUG("Record bin cleaned up");
}
