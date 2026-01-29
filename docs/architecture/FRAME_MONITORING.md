# Frame Rate Monitoring and Drop Detection

## Overview

The frame monitoring system provides real-time measurement and validation of video frame rates for the Video Looper application. It detects frame drops, measures frame rate metrics, and validates that the application maintains the target 120 fps performance requirement.

**Document**: Frame monitoring system architecture and implementation
**Component**: `src/utils/frame_monitor.h`, `src/utils/frame_monitor.c`
**SDD Reference**: §3.4 (Pipeline Builder), §7.4 (Logging Approach)
**PRD Reference**: §5.1 (Performance Requirements)

---

## Purpose and Motivation

### Problem Statement

The Video Looper application has a strict performance requirement: **maintain 120 fps playback across all 10 grid cells simultaneously**. Without runtime monitoring, performance regressions can go undetected until end-to-end testing or user deployment.

The frame monitoring system solves this by:

1. **Real-time frame tracking**: Records every rendered frame timestamp
2. **Drop detection**: Identifies gaps in frame delivery indicating missed frames
3. **Rate validation**: Compares measured frame rate against target and tolerance
4. **Performance reporting**: Generates logs and reports for debugging

### Why It Matters

- **Performance regression detection**: Early warning of bottlenecks
- **Hardware validation**: Verifies target system can sustain 120 fps
- **Debugging aid**: Timestamp data helps identify when/where drops occur
- **Test automation**: Enables automated performance validation in CI/CD

---

## Architecture

### Component Overview

```
Frame Monitor
├── Frame Timestamp Tracking
│   ├── GQueue for rolling window of frame times
│   ├── Circular buffer (max 300 frames)
│   └── Thread-safe access with GMutex
├── Drop Detection Engine
│   ├── Analyzes gaps between frame timestamps
│   ├── Adaptive threshold (1.5x expected frame duration)
│   └── Counts missing frames per gap
├── Statistics Calculator
│   ├── Average, min, max frame rates
│   ├── Standard deviation for stability
│   └── Overall drop rate computation
└── Validation & Reporting
    ├── Compare against target (120 fps ±2 tolerance)
    ├── Report generation for logging
    └── Performance event logging
```

### Data Flow

```
Rendered Frame
    ↓
frame_monitor_on_frame()
    ↓
Timestamp recorded in window queue
    ↓
Drop detection (adaptive threshold)
    ├─→ If gap > 1.5x expected interval
    │       ↓
    │   Count dropped frames
    │       ↓
    │   Log warning
    │
    └─→ If gap normal
            ↓
        Continue tracking

Statistics Calculation (on demand)
    ├─→ Per-frame intervals from window
    ├─→ Calculate average, min, max, stddev
    └─→ Compute overall drop rate

Validation Check (on demand)
    ├─→ Compare average fps vs target (120 ±2)
    ├─→ Check variance (stddev < 10% of target)
    └─→ Return validation result

Report Generation (on demand)
    ├─→ Format statistics
    ├─→ Add drop information
    ├─→ Include validation status
    └─→ Return formatted report
```

---

## API Reference

### Core Types

#### `FrameMonitor`

Opaque context for frame monitoring. Create with `frame_monitor_create()`, use with monitoring functions, cleanup with `frame_monitor_cleanup()`.

```c
FrameMonitor *monitor = frame_monitor_create();
// ... use monitor ...
frame_monitor_cleanup(monitor);
```

#### `FrameMonitorStats`

Statistics from the monitoring window:

```c
typedef struct {
    guint64 total_frames;           // All frames since creation
    guint64 dropped_frames;         // Detected dropped frames
    gdouble current_fps;            // Last measured fps
    gdouble average_fps;            // Window average
    gdouble fps_min;                // Minimum observed
    gdouble fps_max;                // Maximum observed
    gdouble fps_std_dev;            // Standard deviation
    guint64 first_frame_timestamp;  // First frame time (ns)
    guint64 last_frame_timestamp;   // Last frame time (ns)
    guint64 session_duration_ns;    // Total window duration
} FrameMonitorStats;
```

#### `FrameDropInfo`

Frame drop analysis:

```c
typedef struct {
    gboolean has_drops;             // True if drops detected
    guint64 drop_count;             // Number of drop events
    gdouble drop_rate;              // Percentage (0-100)
    guint64 largest_drop_gap_ns;    // Largest gap observed
    guint64 drop_start_timestamp;   // When first drop occurred
} FrameDropInfo;
```

#### `FrameRateValidationResult`

Validation outcome:

```c
typedef enum {
    FRAME_RATE_VALID,               // Within target ±tolerance
    FRAME_RATE_LOW,                 // Below minimum
    FRAME_RATE_HIGH,                // Above maximum
    FRAME_RATE_UNSTABLE,            // High variance
    FRAME_RATE_INSUFFICIENT_DATA,   // <30 frames in window
} FrameRateValidationResult;
```

### Primary Functions

#### Lifecycle

```c
// Create monitor
FrameMonitor* frame_monitor_create(void);

// Clean up
void frame_monitor_cleanup(FrameMonitor *monitor);

// Reset statistics
void frame_monitor_reset(FrameMonitor *monitor);
```

#### Frame Recording

```c
// Record rendered frame
gboolean frame_monitor_on_frame(FrameMonitor *monitor, GstClockTime timestamp);
```

Must be called once per rendered frame with the frame's GStreamer clock timestamp.

#### Statistics Retrieval

```c
// Get current statistics
FrameMonitorStats* frame_monitor_get_stats(FrameMonitor *monitor);

// Detect dropped frames
FrameDropInfo* frame_monitor_detect_drops(FrameMonitor *monitor);

// Validate frame rate
FrameRateValidationResult frame_monitor_validate_framerate(
    FrameMonitor *monitor,
    guint target_fps,
    guint tolerance_fps);
```

Remember to free returned `Stats` and `DropInfo` structures with `g_free()`.

#### Logging and Reporting

```c
// Log statistics
void frame_monitor_log_stats(FrameMonitor *monitor, gboolean include_detailed);

// Log drop information
void frame_monitor_log_drops(FrameMonitor *monitor);

// Generate formatted report
gchar* frame_monitor_generate_report(FrameMonitor *monitor);  // Must g_free()

// Get validation description
const gchar* frame_monitor_validation_string(FrameRateValidationResult result);
```

#### State Queries

```c
// Get window size
guint64 frame_monitor_get_window_size(FrameMonitor *monitor);

// Check for sufficient data
gboolean frame_monitor_has_sufficient_data(FrameMonitor *monitor);
```

---

## Usage Examples

### Basic Integration

Integrate frame monitoring into the rendering pipeline:

```c
#include "utils/frame_monitor.h"

// Global or context-stored monitor
static FrameMonitor *frame_monitor = NULL;

// In application initialization
void app_init(void) {
    frame_monitor = frame_monitor_create();
    if (!frame_monitor) {
        LOG_ERROR("Failed to create frame monitor");
        return;
    }
}

// In rendering callback (called once per frame)
void on_frame_rendered(GstClockTime frame_timestamp) {
    // Record frame
    if (!frame_monitor_on_frame(frame_monitor, frame_timestamp)) {
        LOG_ERROR("Failed to record frame in monitor");
    }
}

// In cleanup
void app_cleanup(void) {
    if (frame_monitor) {
        frame_monitor_cleanup(frame_monitor);
    }
}
```

### Performance Validation

Validate frame rate meets requirements:

```c
// After collecting baseline (~2.5 seconds)
if (frame_monitor_has_sufficient_data(frame_monitor)) {
    FrameRateValidationResult result =
        frame_monitor_validate_framerate(frame_monitor, 120, 2);

    switch (result) {
    case FRAME_RATE_VALID:
        LOG_INFO("✓ Frame rate valid: 120 fps ±2 fps");
        break;
    case FRAME_RATE_LOW:
        LOG_WARNING("✗ Frame rate LOW: below 118 fps");
        break;
    case FRAME_RATE_HIGH:
        LOG_WARNING("✗ Frame rate HIGH: above 122 fps");
        break;
    case FRAME_RATE_UNSTABLE:
        LOG_WARNING("✗ Frame rate UNSTABLE: high variance");
        break;
    default:
        LOG_ERROR("✗ Validation error");
    }
}
```

### Periodic Reporting

Log statistics periodically (e.g., every 30 seconds):

```c
// In timer callback
void on_monitoring_timer(void) {
    if (frame_monitor_has_sufficient_data(frame_monitor)) {
        // Log summary
        frame_monitor_log_stats(frame_monitor, FALSE);  // Brief mode

        // Log drops if any
        frame_monitor_log_drops(frame_monitor);

        // Reset window for next interval
        // frame_monitor_reset(frame_monitor);  // Optional
    }
}
```

### Detailed Analysis

Generate full performance report:

```c
// On demand (e.g., at shutdown or in tests)
gchar *report = frame_monitor_generate_report(frame_monitor);
if (report) {
    LOG_INFO("Performance Report:\n%s", report);
    g_free(report);
}
```

---

## Implementation Details

### Frame Time Tracking

The monitor maintains a rolling circular buffer of frame timestamps:

- **Window size**: 300 frames (≈2.5 seconds at 120 fps)
- **Storage**: GQueue (GLib priority queue) with fixed max size
- **Thread-safe**: Protected by GMutex for concurrent access
- **Memory**: ~8 bytes per timestamp × 300 = ~2.4 KB overhead

### Drop Detection Algorithm

Drops are detected using adaptive threshold analysis:

```
Expected frame duration @ 120 fps = 8.333 ms = 8,333,333 ns
Maximum acceptable gap = 1.5 × expected = 12,500,000 ns

For each frame:
  gap = current_timestamp - previous_timestamp
  if gap > max_acceptable_gap:
    missing_frames = (gap / expected) - 1
    dropped_frames += missing_frames
    log warning
```

The 1.5× threshold accounts for:
- Jitter in frame delivery (typically <20%)
- System scheduler variance
- GPU pipeline variability
- Minor thermal throttling

### Statistics Calculation

Per-frame intervals are calculated from the timestamp window:

```
intervals[] = [T1-T0, T2-T1, T3-T2, ..., Tn-Tn-1]

average_fps = (n-1) / (Tn - T0)
current_fps = 1.0 / (last_interval)

min/max = extrema(intervals)
stddev = sqrt(Σ(fps_i - mean)² / n)
```

### Validation Logic

Frame rate is valid when:

1. **Sufficient data**: Window has ≥30 frames (~0.25 sec)
2. **Rate in range**: `120 - 2 ≤ average_fps ≤ 120 + 2`
3. **Stable delivery**: `stddev < 10% of target` (≤12 fps variance)

Fails with specific code for debugging:
- `FRAME_RATE_LOW`: Bottleneck detected
- `FRAME_RATE_HIGH`: Clock skew or measurement error
- `FRAME_RATE_UNSTABLE`: Variable latency or stutter
- `FRAME_RATE_INSUFFICIENT_DATA`: Not enough samples yet

---

## Integration Points

### With GStreamer Pipeline

Connect to the osxvideosink element's buffer probe:

```c
// In pipeline_builder.c
GstPad *sink_pad = gst_element_get_static_pad(osxvideosink, "sink");
gst_pad_add_probe(sink_pad, GST_PAD_PROBE_TYPE_BUFFER,
                  on_buffer_probe, NULL, NULL);

static GstPadProbeReturn on_buffer_probe(GstPad *pad, GstPadProbeInfo *info,
                                         gpointer user_data) {
    GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    frame_monitor_on_frame(global_monitor, GST_BUFFER_PTS(buffer));
    return GST_PAD_PROBE_OK;
}
```

### With Logging System

Frame monitor uses the application's logging system:

```c
LOG_DEBUG("Frame received: count=%lld", total_frames);
LOG_INFO("Frame Rate Statistics: avg=%.1f fps", average_fps);
LOG_WARNING("Frame drop detected: gap=%lld ns", gap);
LOG_ERROR("Frame monitor error");
```

### With Performance Profiling

Export metrics for external profiling tools:

```c
// Could implement JSON export for perf dashboards
void frame_monitor_export_json(FrameMonitor *monitor, FILE *f) {
    FrameMonitorStats *stats = frame_monitor_get_stats(monitor);
    fprintf(f, "{\"avg_fps\": %.1f, \"dropped\": %lld, ...}\n",
            stats->average_fps, stats->dropped_frames);
    g_free(stats);
}
```

---

## Constants

```c
#define FRAME_MONITOR_WINDOW_SIZE 300       // ~2.5 sec at 120 fps
#define FRAME_MONITOR_TARGET_FPS 120        // Target frame rate
#define FRAME_MONITOR_TOLERANCE_FPS 2       // ±2 fps acceptable range
```

---

## Thread Safety

- **Thread-safe**: Yes, uses GMutex for all state access
- **Lock scope**: Minimized to data structure updates only
- **Contention**: Expected minimal (one frame per frame time)
- **No allocations under lock**: Safe for real-time threads

---

## Performance Characteristics

- **Per-frame overhead**: ~100-200 ns (queue push, timestamp comparison)
- **Memory usage**: ~2.4 KB for 300-frame window
- **CPU usage**: <1% during normal operation
- **GPU usage**: None (pure software monitoring)

---

## Testing

Unit tests in `test/unit/test_frame_monitor.c`:

- `test_frame_monitor_create_cleanup` - Lifecycle
- `test_frame_monitor_perfect_120fps` - Normal operation
- `test_frame_monitor_detect_single_drop` - Drop detection
- `test_frame_monitor_validate_*` - Validation logic
- `test_frame_monitor_generate_report` - Reporting
- `test_frame_monitor_window_limit` - Buffer limits
- `test_frame_monitor_unstable_fps` - Variance detection

Run tests:
```bash
meson test frame_monitor
```

---

## Debugging Tips

### Frame rate is reported as LOW

**Symptom**: `frame_monitor_validate_framerate()` returns `FRAME_RATE_LOW`

**Investigation**:
1. Check system load: `Activity Monitor` → CPU and GPU usage
2. Verify no other applications using GPU intensively
3. Check GStreamer pipeline for bottlenecks: `GST_DEBUG=*:3`
4. Look for memory pressure or swapping
5. Verify hardware supports 120 fps (check GPU capabilities)

### Frame drops detected

**Symptom**: `frame_monitor_detect_drops()` reports `has_drops=TRUE`

**Investigation**:
1. Check drop count and timing: Was it a single event or continuous?
2. Look at `largest_drop_gap_ns` to understand severity
3. Correlate with system events (background tasks, thermal throttling)
4. Check GPU memory allocation patterns
5. Verify camera input is not dropping frames at source

### Unstable frame rate

**Symptom**: `FRAME_RATE_UNSTABLE` with high std deviation

**Investigation**:
1. Monitor CPU and GPU clock speeds (may indicate thermal scaling)
2. Check for competing threads or tasks
3. Look for priority inversion in GStreamer pipeline
4. Verify queue sizes not causing backpressure
5. Check if window is resizing or other UI events occurring

---

## Future Enhancements

Potential improvements (post-MVP):

1. **Histogram tracking**: Per-frame latency distribution
2. **Adaptive thresholds**: Learn normal baseline, alert on deviations
3. **Event correlation**: Link drops to GStreamer bus errors
4. **Remote reporting**: Send metrics to performance monitoring service
5. **Power state tracking**: Correlate with CPU/GPU frequency scaling
6. **Memory profiling**: Track GPU memory allocation during frames

---

## References

- **SDD §3.4**: GStreamer Pipeline Builder - bus message handling
- **SDD §7.4**: Logging Approach - logging infrastructure
- **PRD §5.1**: Performance requirements - 120 fps ±2 tolerance
- **GStreamer**: Clock and timing documentation
- **GLib**: Data structures (GQueue, GMutex) documentation

