/*
 * recording_state.h - Recording State Manager Header
 *
 * Public interface for recording state tracking and key press/release handling.
 */

#ifndef RECORDING_STATE_H
#define RECORDING_STATE_H

#include <stdint.h>
#include "../app/app_config.h"

/* GLib types - defined here for compilation with standard C types */
#ifndef __GLIB_H__
typedef int gboolean;
typedef unsigned long long guint64;
typedef int gint;
#endif

/* Forward declaration to avoid gst/gst.h dependency */
typedef struct _GstElement GstElement;

/**
 * RecordingState - Track recording state for all layers
 *
 * @is_recording: Array indicating if each key (1-50) is currently recording
 * @record_start_time: Start timestamp for each recording (microseconds)
 * @record_duration_us: Duration of each recording (microseconds)
 * @current_cell_index: Next cell to assign (0-49, maps to layers 1-50)
 */
typedef struct {
    gboolean is_recording[TOTAL_LAYERS];      /* One per key (1-50) */
    guint64 record_start_time[TOTAL_LAYERS];  /* Timestamp when recording started */
    guint64 record_duration_us[TOTAL_LAYERS]; /* Duration in microseconds */
    gint current_cell_index;                  /* Next cell to fill (0-49) */
} RecordingState;

/**
 * recording_state_init - Initialize recording state tracker
 *
 * Returns: Allocated RecordingState, or NULL on allocation failure
 */
RecordingState *recording_state_init(void);

/**
 * recording_on_key_press - Handle keyboard key press event
 *
 * @state: Recording state tracker
 * @key_number: Key number (1-50)
 *
 * Marks the key as recording and captures the start timestamp.
 */
void recording_on_key_press(RecordingState *state, int key_number);

/**
 * recording_on_key_release - Handle keyboard key release event
 *
 * @state: Recording state tracker
 * @key_number: Key number (1-50)
 *
 * Stops recording for the key and calculates total duration.
 */
void recording_on_key_release(RecordingState *state, int key_number);

/**
 * recording_is_recording - Query if a key is currently recording
 *
 * @state: Recording state tracker
 * @key_number: Key number (1-50)
 *
 * Returns: TRUE if the key is currently being held and recording
 */
gboolean recording_is_recording(RecordingState *state, int key_number);

/**
 * recording_get_duration - Get duration of a recorded segment
 *
 * @state: Recording state tracker
 * @key_number: Key number (1-50)
 *
 * Returns: Duration in microseconds, or 0 if not recorded yet
 */
guint64 recording_get_duration(RecordingState *state, int key_number);

/**
 * recording_assign_next_cell - Get next cell and advance circular index
 *
 * @state: Recording state tracker
 *
 * Implements circular cell assignment:
 * Returns cell 0 (layer 1), advances index to 1, ..., returns cell 49 (layer 50),
 * then wraps to 0 again.
 *
 * Returns: Cell index (0-8) for the next recording
 */
gint recording_assign_next_cell(RecordingState *state);

/**
 * recording_start_capture - Signal GStreamer record bin to start capturing
 *
 * @record_bin: GStreamer record bin element
 * @start_time: Timestamp when recording started (microseconds)
 *
 * Signals the record bin to begin capturing video frames to its buffer.
 */
void recording_start_capture(GstElement *record_bin, guint64 start_time);

/**
 * recording_stop_capture - Signal GStreamer record bin to stop capturing
 *
 * @record_bin: GStreamer record bin element
 * @duration_us: Duration of the recording in microseconds
 *
 * Signals the record bin to stop capturing video frames.
 */
void recording_stop_capture(GstElement *record_bin, guint64 duration_us);

/**
 * recording_state_cleanup - Clean up and deallocate recording state
 *
 * @state: Recording state tracker to clean up
 *
 * Frees all allocated memory associated with the recording state.
 */
void recording_state_cleanup(RecordingState *state);

/**
 * recording_get_min_frame_duration_us - Get minimum frame duration constant
 *
 * Returns the enforced minimum duration for any recording: 33333 microseconds
 * (approximately 1/30th of a second, matching 30fps input rate).
 *
 * Edge case handling:
 * - Key presses shorter than this are rounded up to this minimum
 * - Ensures at least 1 frame is captured even for sub-frame key presses
 *
 * Returns: Minimum frame duration in microseconds (33333)
 */
guint64 recording_get_min_frame_duration_us(void);

#endif /* RECORDING_STATE_H */
