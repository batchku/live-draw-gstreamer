# Recording Buffer Edge Case Handling (T-9.3)

**Task ID**: T-9.3
**Phase**: 9 - Comprehensive Error Handling & Edge Cases
**SDD References**: §3.5 (Recording Buffer Manager), §7 (Error Handling Strategy)
**PRD References**: §4.5 (Video Loop Recording), §5.2 (Reliability)

---

## Overview

This document describes the comprehensive edge case handling implemented in the Video Looper recording system, addressing three critical scenarios:

1. **Buffer Overflow Handling** - When GPU ring buffer capacity is exceeded
2. **Short Key Presses (<33ms)** - Recording durations shorter than 1 frame at 30fps
3. **Multiple Recordings Beyond 9 Cells** - Circular wraparound when cell capacity exceeded

---

## 1. Buffer Overflow Handling

### Scenario

When a user records continuously, the recording buffer (ring buffer) can reach capacity and must discard the oldest frames to make room for new ones. This is expected behavior, not an error, but must be properly handled and monitored.

**Buffer Capacity**: ~60 frames per recording at 30fps input ≈ 2 seconds of video

### Implementation

#### 1.1 Overflow Detection & Tracking

**File**: `src/recording/buffer_manager.c` and `.h`

The `RingBuffer` structure now includes:

```c
typedef struct {
  GstBuffer **frames;           /* Circular array of GStreamer buffers */
  guint capacity;               /* Maximum frames in buffer */
  guint write_pos;              /* Current write position (0 to capacity-1) */
  guint read_pos;               /* Current read position (unused) */
  guint frame_count;            /* Current frame count (min 0, max capacity) */
  guint64 duration_us;          /* Sum of frame durations in microseconds */
  GstCaps *caps;                /* Frame format capabilities */
  guint overflow_count;         /* NEW: Number of times capacity was exceeded */
  guint total_frames_written;   /* NEW: Total frames written (includes discarded) */
} RingBuffer;
```

#### 1.2 Overflow Handling in `buffer_write_frame()`

When buffer reaches capacity and a new frame arrives:

```c
void buffer_write_frame(RingBuffer *buf, GstBuffer *frame) {
  /* ... NULL checks ... */

  buf->total_frames_written++;  /* Track all frames written */

  if (buf->frame_count >= buf->capacity) {
    /* Buffer full: unreference oldest frame to release GPU memory */
    if (buf->frames[buf->write_pos]) {
      gst_buffer_unref(buf->frames[buf->write_pos]);
      buf->frames[buf->write_pos] = NULL;
    }
    buf->overflow_count++;  /* Increment overflow counter */
    LOG_WARNING("Buffer overflow: discarded oldest frame at position %u "
                "(total overflows: %u)", buf->write_pos, buf->overflow_count);
  } else {
    buf->frame_count++;  /* Buffer not yet full */
  }

  /* Store new frame */
  gst_buffer_ref(frame);
  buf->frames[buf->write_pos] = frame;

  /* Update duration and advance write position */
  /* ... */
}
```

#### 1.3 Overflow Monitoring APIs

Two new APIs for diagnostics:

```c
/**
 * buffer_get_overflow_count() - Get number of overflow events
 *
 * Returns how many times the buffer reached capacity and had to discard frames.
 * Useful for monitoring long recording sessions and identifying memory constraints.
 */
guint buffer_get_overflow_count(RingBuffer *buf);

/**
 * buffer_get_total_frames_written() - Get total frames written
 *
 * Returns the total number of frames written to the buffer since creation,
 * including frames that were later overwritten due to overflow.
 *
 * Metric: total_written > frame_count indicates overflow has occurred
 */
guint buffer_get_total_frames_written(RingBuffer *buf);
```

### 1.4 Behavior

**Example Scenario**:
- Buffer capacity: 20 frames (2/3 second at 30fps)
- User records 50 frames continuously

**Behavior**:
1. Frames 1-20: Written to buffer (frame_count increases 0→20)
2. Frame 21: Overflow! Frame 1 discarded, Frame 21 stored (overflow_count = 1)
3. Frame 22: Frame 2 discarded (overflow_count = 2)
4. ... continues ...
5. Frame 50: Frame 30 discarded (overflow_count = 30)

**Final State**:
- `frame_count = 20` (buffer capacity)
- `overflow_count = 30` (30 frames discarded)
- `total_frames_written = 50` (all frames written)
- Buffer contains frames 31-50 (newest 20 frames)

### 1.5 Performance Impact

- **Memory**: Oldest frames automatically released to GPU (no memory leak)
- **CPU**: O(1) for each overflow event (just unref + counter increment)
- **Logging**: WARNING level logged for each overflow for diagnostics

### 1.6 User Perception

- **No visible impact**: Playback continues smoothly with the newest recorded frames
- **Implicit behavior**: User doesn't need to manage buffer capacity
- **Diagnostic visibility**: Applications can check `buffer_get_overflow_count()` to warn if buffer exhaustion is excessive

---

## 2. Short Key Press Edge Cases (<33ms)

### Scenario

When a user presses a key very briefly (less than 33 milliseconds), they might release it before a full frame is captured. The system must ensure that even very short key presses result in at least 1 frame being recorded.

**Minimum Frame Duration**: 33.333 milliseconds (1/30th second, matching 30fps input)

### Implementation

#### 2.1 Minimum Duration Enforcement

**File**: `src/recording/recording_state.c` and `.h`

```c
/**
 * recording_get_min_frame_duration_us() - Get minimum frame duration
 *
 * Returns 33333 microseconds (≈33.333 milliseconds), which represents
 * 1/30th of a second, matching the 30fps input frame rate.
 *
 * This is the minimum duration enforced for any recording.
 */
guint64 recording_get_min_frame_duration_us(void) {
  return 33333;  /* 1/30 second in microseconds */
}
```

#### 2.2 Duration Enforcement in `recording_on_key_release()`

When a user releases a key, the actual duration is checked:

```c
void recording_on_key_release(RecordingState *state, int key_number) {
  /* ... validation ... */

  /* Calculate actual duration */
  guint64 current_time = timing_get_time_us();
  guint64 duration = current_time - state->record_start_time[key_index];

  /* Enforce minimum duration */
  guint64 min_duration = recording_get_min_frame_duration_us();
  if (duration < min_duration) {
    LOG_INFO("Short key press detected: %llu us < %llu us, enforcing minimum",
             duration, min_duration);
    duration = min_duration;  /* Enforce minimum */
  }

  state->record_duration_us[key_index] = duration;

  /* ... signal GStreamer to stop capturing with enforced duration ... */
}
```

### 2.3 Behavior

**Example Scenarios**:

| Key Press Duration | Behavior | Result |
|-------------------|----------|--------|
| 5ms | < 33ms minimum | Enforced to 33ms (1 frame) |
| 15ms | < 33ms minimum | Enforced to 33ms (1 frame) |
| 32ms | < 33ms minimum | Enforced to 33ms (1 frame) |
| 33ms | = minimum | Accepted as-is (1 frame) |
| 50ms | > 33ms | Accepted as-is (1-2 frames) |
| 100ms | > 33ms | Accepted as-is (3+ frames) |

### 2.4 Multiple Simultaneous Short Presses

Each key's recording duration is enforced independently:

```c
/* User presses keys 1, 2, 3 simultaneously for 10ms, then releases */
recording_on_key_press(state, 1);
recording_on_key_press(state, 2);
recording_on_key_press(state, 3);
/* ... 10ms passes ... */
recording_on_key_release(state, 1);  /* Enforced to 33ms */
recording_on_key_release(state, 2);  /* Enforced to 33ms */
recording_on_key_release(state, 3);  /* Enforced to 33ms */
/* All 3 recordings get 1 frame each */
```

### 2.5 Logging & Diagnostics

Short key presses are logged at INFO level for user awareness:

```
[2026-01-27 14:23:45.123] [INFO] recording_on_key_release:
  Short key press detected: key 1, duration 15 us < 33333 us,
  enforcing minimum for short key press (<33ms edge case)
```

---

## 3. Multiple Recordings Beyond 9 Cells

### Scenario

The grid has exactly 9 cells available for video loops (cells 2-10; cell 1 is live feed). When a user records more than 9 loops, the system implements a circular queue where the 10th recording replaces the 1st, the 11th replaces the 2nd, etc.

**Cell Assignment**:
- Cell 0 (grid cell 2) - 1st recording
- Cell 1 (grid cell 3) - 2nd recording
- ...
- Cell 8 (grid cell 10) - 9th recording
- Cell 0 (grid cell 2) - 10th recording (overwrites 1st)
- ...

### Implementation

#### 3.1 Cell Assignment State

**File**: `src/recording/recording_state.c` and `.h`

The `RecordingState` tracks the next cell to assign:

```c
typedef struct {
  gboolean is_recording[9];          /* One per key (1-9) */
  guint64 record_start_time[9];      /* Timestamp when recording started */
  guint64 record_duration_us[9];     /* Duration in microseconds */
  gint current_cell_index;           /* Next cell to fill (0-8) */
} RecordingState;
```

#### 3.2 Circular Cell Assignment

```c
gint recording_assign_next_cell(RecordingState *state) {
  if (!state) {
    LOG_ERROR("recording_assign_next_cell: state is NULL");
    return -1;
  }

  gint assigned_cell = state->current_cell_index;

  /* Advance to next cell (circular wraparound at 9) */
  state->current_cell_index = (state->current_cell_index + 1) % 9;

  LOG_DEBUG("recording_assign_next_cell: assigned cell index %d, next is %d",
           assigned_cell, state->current_cell_index);

  return assigned_cell;
}
```

### 3.3 Behavior

**Example Scenario**: User records 15 times

| Recording # | Cell Assigned | Grid Cell | Status |
|------------|---------------|-----------|--------|
| 1 | 0 | 2 | New recording |
| 2 | 1 | 3 | New recording |
| 3 | 2 | 4 | New recording |
| 4 | 3 | 5 | New recording |
| 5 | 4 | 6 | New recording |
| 6 | 5 | 7 | New recording |
| 7 | 6 | 8 | New recording |
| 8 | 7 | 9 | New recording |
| 9 | 8 | 10 | New recording |
| 10 | 0 | 2 | **Replaces recording #1** |
| 11 | 1 | 3 | **Replaces recording #2** |
| 12 | 2 | 4 | **Replaces recording #3** |
| 13 | 3 | 5 | **Replaces recording #4** |
| 14 | 4 | 6 | **Replaces recording #5** |
| 15 | 5 | 7 | **Replaces recording #6** |

**Final Grid State**: Contains recordings 7, 8, 9, 10, 11, 12, 13, 14, 15

### 3.4 Multiple Recordings Workflow

When a recording ends, it's assigned a cell:

```c
/* User presses key 1, records for 2 seconds, releases */
recording_on_key_press(state, 1);
/* ... 2 seconds later ... */
recording_on_key_release(state, 1);  /* Recording #1 ends */

/* Get next cell to display this recording */
gint cell_index = recording_assign_next_cell(state);
/* cell_index = 0 → Grid cell 2 */

/* Create playback bin for grid cell 2 with recorded frames */
playback_loop_create(recorded_buffer);
/* ... pipeline_add_playback_bin() with cell 2 ... */
```

### 3.5 Storage During Wraparound

**Important**: Old recordings are **NOT automatically preserved**

When recording #10 arrives and overwrites recording #1:
1. Recording #1's ring buffer is cleaned up (GPU memory released)
2. New recording #10's buffer takes its place
3. Playback bin for cell 2 is updated to display recording #10
4. Recording #1 is lost forever

**This is expected behavior per PRD §4.5**: "If the user records a new video after cell 10 is filled, the oldest loop shall be replaced by the new recording."

---

## 4. Edge Case Combinations

### 4.1 Overflow During Long Recording

**Scenario**: User holds down key for 5 seconds while buffer capacity is 2 seconds

**Behavior**:
1. First 2 seconds: Buffer fills (60 frames)
2. Remaining 3 seconds: Each new frame triggers overflow
3. Final result: Buffer contains only the last 2 seconds recorded
4. Playback shows the final 2-second clip of the 5-second recording

**Advantage**: Graceful degradation - always keeps the most recent data

### 4.2 Short Press During Wraparound

**Scenario**: User presses key 5 for 10ms (short press), and it's the 10th recording

**Behavior**:
1. Key press detected, timing starts
2. User releases after 10ms
3. Duration enforced to 33ms (1 frame minimum)
4. Cell assigned: 4 (wrapping, so 10th recording)
5. Playback shows 1-frame clip in grid cell 6 (replacing old recording)

### 4.3 Continuous Short Presses

**Scenario**: User rapidly taps key 1 nine times (each for ~10ms)

**Behavior**:
1. Tap 1: 33ms → Cell 0, Cell 2 displays
2. Tap 2: 33ms → Cell 1, Cell 3 displays (overwrites tap 1 in memory)
3. ... continues ...
4. Tap 9: 33ms → Cell 8, Cell 10 displays (overwrites tap 2 in memory)
5. Tap 10: Would be short 10ms → 33ms enforced → Cell 0, Cell 2 displays (overwrites tap 1)

---

## 5. Testing Coverage

### 5.1 Unit Tests

**File**: `test/unit/test_edge_cases_buffer_recording.c`

Comprehensive test suite covering:

#### Buffer Overflow Tests
- `test_buffer_overflow_counter()` - Overflow counter incremented correctly
- `test_buffer_total_frames_written()` - Total frames tracked including discarded
- `test_buffer_continuous_overflow()` - Long recording with continuous overflow
- `test_buffer_overflow_with_wraparound_access()` - Ring wraparound correctness

#### Short Key Press Tests
- `test_short_key_press_duration_enforcement()` - Minimum duration enforced
- `test_multiple_simultaneous_short_recordings()` - Multiple keys pressed simultaneously

#### Cell Wraparound Tests
- `test_cell_assignment_wraparound()` - Circular cell assignment 0-8-0
- `test_recording_beyond_nine_cells()` - Wraparound at 9-cell boundary

#### Combined Tests
- `test_combined_overflow_and_wraparound()` - Overflow + wraparound stress test
- `test_null_safety_edge_cases()` - NULL input handling

### 5.2 Running the Tests

```bash
# Compile and run edge case tests
meson test -C build test_edge_cases_buffer_recording

# Run with verbose logging
./build/test_edge_cases_buffer_recording
```

---

## 6. Logging & Monitoring

### 6.1 Log Messages by Event

#### Buffer Overflow

```
[TIME] [WARNING] buffer_write_frame:
  Buffer overflow: discarded oldest frame at position 5 (total overflows: 1)
```

#### Short Key Press

```
[TIME] [INFO] recording_on_key_release:
  Short key press detected: key 1, duration 15 us < 33333 us,
  enforcing minimum for short key press (<33ms edge case)
```

#### Cell Assignment

```
[TIME] [DEBUG] recording_assign_next_cell:
  assigned cell index 8, next is 0
```

### 6.2 Monitoring Queries

Applications can check edge case status:

```c
/* Check if buffer is experiencing overflow */
guint overflow_count = buffer_get_overflow_count(buf);
if (overflow_count > 0) {
  /* Warn user or adjust buffer settings */
  printf("Buffer has discarded %u frames due to overflow\n", overflow_count);
}

/* Check total vs actual frames */
guint total = buffer_get_total_frames_written(buf);
guint actual = buffer_get_frame_count(buf);
printf("Buffer efficiency: %u%% (%u/%u frames retained)\n",
       (actual * 100) / total, actual, total);
```

---

## 7. Design Decisions

### 7.1 Why Minimum Frame Duration?

**Decision**: Enforce minimum 33ms duration for short key presses

**Rationale**:
- 30fps input → 33.333ms per frame minimum
- Shorter presses mean < 1 frame of video
- Must still produce valid output (≥ 1 frame)
- Alternative would be to discard recording (poor UX)

**Trade-off**: User might press for 10ms expecting no recording, but gets 1 frame instead
- **Accepted**: Explicit LOG_INFO message informs user of behavior

### 7.2 Why Overflow Silently?

**Decision**: Discard oldest frames without error when buffer full

**Rationale**:
- Ring buffer is by design a fixed-size circular structure
- Overflow is expected and normal behavior
- Keeping newest data is more useful than oldest
- Alternative would be to fail recording (bad UX)

**Trade-off**: User loses oldest recorded data after 2 seconds
- **Accepted**: Documented behavior, logging available, monitoring APIs provided

### 7.3 Why Circular Cell Assignment?

**Decision**: Wrap around from cell 8 back to cell 0 (9-cell rotation)

**Rationale**:
- Fixed grid: exactly 10 cells (1 live + 9 loops)
- Can't expand dynamically
- Circular queue natural fit for fixed-size slot allocation
- Alternative would be to reject 10th recording (poor UX)

**Trade-off**: Old recordings are replaced without warning
- **Accepted**: Explicit cell assignment logged, user can see grid update

---

## 8. Future Enhancements

### 8.1 Configurable Buffer Size

Currently hardcoded at ~60 frames. Could be:
- Configurable at compile time (build flag)
- Adjustable at runtime (before first record)
- Adaptive based on available GPU memory

### 8.2 User Warnings for Edge Cases

Could add:
- Visual indicator when buffer overflow occurs
- Beep/alert when short key press enforced to minimum
- Confirmation dialog when replacing old recordings

### 8.3 Extended Buffer Options

Could support:
- Larger buffer for extended recordings (at memory cost)
- Dual-buffer system (primary + secondary)
- Recording to disk beyond GPU capacity (out of scope for v1)

---

## 9. Validation Checklist

- [x] Buffer overflow counter implemented and tested
- [x] Total frames written counter implemented and tested
- [x] Minimum frame duration enforcement implemented and tested
- [x] Circular cell assignment implemented and tested
- [x] Edge case unit tests created and passing
- [x] Logging at appropriate levels (WARNING, INFO, DEBUG)
- [x] NULL safety verified for all new functions
- [x] Performance impact negligible (O(1) operations)
- [x] Documentation complete

---

**Document Version**: 1.0
**Last Updated**: January 27, 2026
**Status**: Implementation Complete
**SDD Alignment**: §3.5 (Buffer Manager), §7 (Error Handling)
**PRD Alignment**: §4.5 (Recording), §5.2 (Reliability)
