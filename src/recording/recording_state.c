/*
 * recording_state.c - Recording State Manager
 *
 * Tracks keyboard input and manages which cells are currently recording.
 * Translates key press/release events to record start/stop signals sent to GStreamer bins.
 *
 * Copyright (c) 2026
 */

#include "recording_state.h"
#include "../utils/logging.h"
#include "../utils/timing.h"
#include <glib.h>
#include <string.h>

/**
 * recording_state_init - Initialize recording state tracker
 *
 * Allocates and initializes the recording state structure.
 * All keys start in non-recording state with zero duration.
 * Cell assignment starts at index 0 (cell 2 in the grid).
 *
 * Returns: Allocated RecordingState, or NULL on allocation failure
 */
RecordingState *recording_state_init(void)
{
    RecordingState *state = g_malloc0(sizeof(RecordingState));
    if (!state) {
        LOG_ERROR("Failed to allocate recording state");
        return NULL;
    }

    /* Initialize all keys to not recording */
    for (int i = 0; i < TOTAL_LAYERS; i++) {
        state->is_recording[i] = FALSE;
        state->record_start_time[i] = 0;
        state->record_duration_us[i] = 0;
    }

    /* Start layer assignment at index 0 (maps to layer 1) */
    state->current_cell_index = 0;

    LOG_INFO("Recording state initialized");
    return state;
}

/**
 * recording_on_key_press - Handle keyboard key press event
 *
 * @state: Recording state tracker
 * @key_number: Key number (1-50)
 *
 * When a key is pressed:
 * 1. Mark the key as recording
 * 2. Capture the current timestamp
 * 3. Signal the GStreamer record bin to start capturing
 * 4. Provide visual/auditory feedback
 *
 * Multiple simultaneous key presses are handled independently.
 * Key presses for keys outside 1-50 range are silently ignored.
 */
void recording_on_key_press(RecordingState *state, int key_number)
{
    if (!state) {
        LOG_ERROR("recording_on_key_press: state is NULL");
        return;
    }

    /* Validate key number is in range 1-50 */
    if (key_number < 1 || key_number > TOTAL_LAYERS) {
        LOG_DEBUG("recording_on_key_press: key %d outside 1-50 range, ignoring", key_number);
        return;
    }

    int key_index = key_number - 1; /* Convert 1-50 to 0-49 array index */

    /* Check if already recording on this key */
    if (state->is_recording[key_index]) {
        LOG_DEBUG("recording_on_key_press: key %d already recording", key_number);
        return;
    }

    /* Mark as recording and capture start time */
    state->is_recording[key_index] = TRUE;
    state->record_start_time[key_index] = timing_get_time_us();
    state->record_duration_us[key_index] = 0;

    LOG_DEBUG("recording_on_key_press: key %d started recording at %llu us", key_number,
              state->record_start_time[key_index]);

    /* Note: Visual/auditory feedback and GStreamer signaling are handled by
     * the e2e_coordinator module which calls recording_on_key_press
     */
}

/**
 * recording_on_key_release - Handle keyboard key release event
 *
 * @state: Recording state tracker
 * @key_number: Key number (1-50)
 *
 * When a key is released:
 * 1. Mark the key as not recording
 * 2. Calculate the duration of the recording
 * 3. Signal the GStreamer record bin to stop capturing
 * 4. Notify playback manager of the new recording
 *
 * Key releases for keys that are not currently recording are silently ignored.
 * Very short recordings (< 1 frame at 33ms) still record as 1 frame minimum.
 */
void recording_on_key_release(RecordingState *state, int key_number)
{
    if (!state) {
        LOG_ERROR("recording_on_key_release: state is NULL");
        return;
    }

    /* Validate key number is in range 1-50 */
    if (key_number < 1 || key_number > TOTAL_LAYERS) {
        LOG_DEBUG("recording_on_key_release: key %d outside 1-50 range, ignoring", key_number);
        return;
    }

    int key_index = key_number - 1; /* Convert 1-50 to 0-49 array index */

    /* Check if this key is currently recording */
    if (!state->is_recording[key_index]) {
        LOG_DEBUG("recording_on_key_release: key %d not recording", key_number);
        return;
    }

    /* Calculate recording duration */
    guint64 current_time = timing_get_time_us();
    guint64 duration = current_time - state->record_start_time[key_index];

    /* Enforce minimum duration of 1 frame (33ms at 30fps) */
    guint64 min_frame_duration_us = recording_get_min_frame_duration_us();
    if (duration < min_frame_duration_us) {
        LOG_INFO("recording_on_key_release: key %d duration %llu us < 1 frame (%llu us), "
                 "enforcing minimum for short key press (<33ms edge case)",
                 key_number, duration, min_frame_duration_us);
        duration = min_frame_duration_us;
    }

    state->record_duration_us[key_index] = duration;
    state->is_recording[key_index] = FALSE;

    LOG_DEBUG("recording_on_key_release: key %d stopped recording, duration: %llu us (%.1f ms)",
              key_number, duration, duration / 1000.0);

    /* Note: GStreamer signaling, playback manager notification, and cell assignment
     * are handled by the e2e_coordinator module which calls recording_on_key_release
     */
}

/**
 * recording_is_recording - Query if a key is currently recording
 *
 * @state: Recording state tracker
 * @key_number: Key number (1-50)
 *
 * Returns: TRUE if the key is currently being held and recording, FALSE otherwise
 */
gboolean recording_is_recording(RecordingState *state, int key_number)
{
    if (!state) {
        LOG_ERROR("recording_is_recording: state is NULL");
        return FALSE;
    }

    if (key_number < 1 || key_number > TOTAL_LAYERS) {
        return FALSE;
    }

    return state->is_recording[key_number - 1];
}

/**
 * recording_get_duration - Get duration of a recorded segment
 *
 * @state: Recording state tracker
 * @key_number: Key number (1-50)
 *
 * Returns: Duration in microseconds, or 0 if not recorded yet
 */
guint64 recording_get_duration(RecordingState *state, int key_number)
{
    if (!state) {
        LOG_ERROR("recording_get_duration: state is NULL");
        return 0;
    }

    if (key_number < 1 || key_number > TOTAL_LAYERS) {
        return 0;
    }

    return state->record_duration_us[key_number - 1];
}

/**
 * recording_assign_next_cell - Get the next cell to fill and advance the circular index
 *
 * @state: Recording state tracker
 *
 * Implements circular cell assignment logic:
 * - Layer assignment starts at index 0 (layer 1)
 * - After each recording, advance to next index
 * - After layer 49 (layer 50), wrap around to 0 (layer 1)
 * - This creates a circular buffer of 50 layers for recordings
 *
 * Returns: Layer index (0-49) for the next recording
 */
gint recording_assign_next_cell(RecordingState *state)
{
    if (!state) {
        LOG_ERROR("recording_assign_next_cell: state is NULL");
        return -1;
    }

    gint assigned_cell = state->current_cell_index;

    /* Advance to next cell (circular) */
    state->current_cell_index = (state->current_cell_index + 1) % TOTAL_LAYERS;

    LOG_DEBUG("recording_assign_next_cell: assigned cell index %d, next is %d", assigned_cell,
              state->current_cell_index);

    return assigned_cell;
}

/**
 * recording_start_capture - Signal GStreamer record bin to start capturing
 *
 * @record_bin: GStreamer record bin element
 * @start_time: Timestamp when recording started (microseconds)
 *
 * This function signals the GStreamer record bin to begin capturing video frames
 * to its associated ring buffer. Called during key press event.
 *
 * The record bin's internal sink pad receives frames automatically when
 * the pipeline is in PLAYING state and the tee element routes data to it.
 * No explicit signaling is needed; the GStreamer pipeline handles frame delivery.
 */
void recording_start_capture(GstElement *record_bin, guint64 start_time)
{
    if (!record_bin) {
        LOG_ERROR("recording_start_capture: record_bin is NULL");
        return;
    }

    LOG_DEBUG("recording_start_capture: signaling record bin to start at %llu us", start_time);

    /* The record bin will begin receiving frames from the tee element
     * via GStreamer's automatic data flow. No explicit action needed.
     */
}

/**
 * recording_stop_capture - Signal GStreamer record bin to stop capturing
 *
 * @record_bin: GStreamer record bin element
 * @duration_us: Duration of the recording in microseconds
 *
 * This function signals the GStreamer record bin to stop capturing video frames.
 * Called during key release event.
 *
 * The record bin continues to receive frames as long as it remains linked to the tee.
 * The actual stopping happens when the pipeline removes the record bin linkage
 * (handled by e2e_coordinator through pipeline_remove_record_bin).
 */
void recording_stop_capture(GstElement *record_bin, guint64 duration_us)
{
    if (!record_bin) {
        LOG_ERROR("recording_stop_capture: record_bin is NULL");
        return;
    }

    LOG_DEBUG("recording_stop_capture: signaling record bin to stop, duration: %llu us",
              duration_us);

    /* The record bin will stop receiving frames when it is unlinked from the tee
     * and removed from the pipeline. This is handled at the pipeline level.
     * No explicit action needed here.
     */
}

/**
 * recording_state_cleanup - Clean up and deallocate recording state
 *
 * @state: Recording state tracker to clean up
 *
 * Frees all allocated memory associated with the recording state.
 * This should be called during application shutdown.
 */
void recording_state_cleanup(RecordingState *state)
{
    if (!state) {
        LOG_ERROR("recording_state_cleanup: state is NULL");
        return;
    }

    LOG_DEBUG("Cleaning up recording state");
    g_free(state);
}

/**
 * recording_get_min_frame_duration_us - Get minimum frame duration
 *
 * Returns the enforced minimum duration for recordings: 33333 microseconds.
 * This represents approximately 1/30th of a second, matching the 30fps input rate.
 *
 * Edge cases handled:
 * - Key presses < 33ms: Duration is rounded up to minimum
 * - Short key presses < 1 frame: Still recorded as 1 frame minimum
 * - Multiple recordings: Each enforces its own minimum
 *
 * Returns: Minimum frame duration in microseconds (33333)
 */
guint64 recording_get_min_frame_duration_us(void)
{
    /* 1/30th second = 33.333 milliseconds = 33333 microseconds */
    return 33333;
}
