# GStreamer Pipeline Profiling Guide

## Task T-8.1: Pipeline Profiling for 120 FPS Target

This document describes the profiling infrastructure implemented for identifying bottlenecks in the video-looper GStreamer pipeline, specifically focusing on queue buffering and synchronization issues.

## Overview

The profiling system provides tools for measuring and analyzing pipeline performance to achieve the 120 fps target. It identifies bottleneck elements by analyzing:

- **Queue buffering depth** - How full queues are, indicating producer/consumer imbalance
- **Element latency** - Processing time spent in each element
- **Synchronization metrics** - Buffer drop rates and timing jitter
- **Frame rate consistency** - FPS variance and stability

## Architecture

### Core Components

#### 1. Profiling Context (`profiling.h/c`)

The `ProfilingContext` structure manages all profiling state:

```c
typedef struct {
    GstElement *pipeline;                   // Pipeline being profiled
    GQueue *frame_timings;                  // Per-frame timing data
    GQueue *performance_samples;            // Periodic performance snapshots
    GHashTable *element_metrics;            // Per-element statistics
    guint64 profile_start_time_us;          // When profiling started
    guint sample_interval_ms;               // Sampling frequency
    gboolean is_active;                     // Profiling running flag
    guint max_samples;                      // Circular buffer size (0=unlimited)
} ProfilingContext;
```

#### 2. Data Structures

**FrameTiming**: Captures timing information for individual frames:
- Frame number
- GStreamer timestamp (nanoseconds)
- Arrival time (system time, microseconds)
- End-to-end latency
- Drop flag

**PerformanceSample**: Periodic snapshot of pipeline performance:
- Timestamp
- Frame number
- Current FPS
- CPU usage
- Queue depth
- Dropped frame count

**ProfileMetrics**: Aggregated statistics for pipeline elements:
- Processing statistics (buffers, bytes, time)
- Latency measurements (min, max, average)
- Buffer drop ratio
- Queue maximum levels
- Bottleneck identification flag

### Key Functions

#### Profiling Lifecycle

```c
// Create context for pipeline
ProfilingContext* profiling_context_create(GstElement *pipeline,
                                           guint sample_interval_ms,
                                           guint max_samples);

// Start collecting metrics
gboolean profiling_start(ProfilingContext *ctx);

// Stop profiling
void profiling_stop(ProfilingContext *ctx);

// Periodic sampling (call every sample_interval_ms)
gboolean profiling_collect_sample(ProfilingContext *ctx);

// Clean up
void profiling_context_free(ProfilingContext *ctx);
```

#### Data Collection

```c
// Record frame timing (called from pad probes)
void profiling_record_frame(ProfilingContext *ctx,
                            guint frame_num,
                            guint64 timestamp_ns,
                            guint64 arrival_time_us,
                            gboolean dropped);
```

#### Analysis Functions

```c
// Get frame rate statistics
gboolean profiling_get_fps_stats(ProfilingContext *ctx,
                                 gdouble *current_fps,
                                 gdouble *average_fps,
                                 gdouble *min_fps,
                                 gdouble *max_fps,
                                 gdouble *std_dev_fps);

// Get queue buffering statistics
gboolean profiling_get_queue_stats(ProfilingContext *ctx,
                                   guint64 *total_queue_bytes,
                                   guint *max_queue_depth,
                                   guint *num_queues);

// Get synchronization metrics
gboolean profiling_get_sync_metrics(ProfilingContext *ctx,
                                    gdouble *buffer_drop_ratio,
                                    guint64 *jitter_us,
                                    guint64 *max_latency_us);

// Identify bottleneck elements
ProfileMetrics* profiling_identify_bottlenecks(ProfilingContext *ctx);

// Generate text report
gchar* profiling_generate_report(ProfilingContext *ctx);

// Export metrics to JSON
gboolean profiling_export_json(ProfilingContext *ctx, const char *filename);
```

## Usage

### Command-Line Profiling

Run the application with GStreamer tracepoints enabled:

```bash
# Enable GStreamer profiling and run for 30 seconds
GST_TRACERS="latency(flags=pipeline+element)" \
  GST_DEBUG="GST_TRACER:7" \
  GST_DEBUG_FILE="profiling.log" \
  ./scripts/profile.sh --gst --duration 30
```

### Profiling Script

The `scripts/profile.sh` script automates profiling with multiple backends:

```bash
# Profile GStreamer pipeline (default)
./scripts/profile.sh --gst --duration 30

# Profile CPU usage
./scripts/profile.sh --cpu --duration 30

# Profile memory usage
./scripts/profile.sh --memory --duration 30

# All outputs saved to profiling_results/
```

### Analysis

Analyze profiling results with the analysis script:

```bash
./scripts/analyze_profiling.sh profiling_results/
```

This script:
1. Parses GStreamer debug logs
2. Extracts queue statistics
3. Identifies synchronization issues
4. Generates optimization recommendations

## Bottleneck Detection

### Criteria for Identifying Bottlenecks

An element is considered a bottleneck if:

1. **Queue Overflow**: Queue max depth > 80% of configured buffer size
   - Indicates producer faster than consumer
   - Solution: Increase `max-size-buffers` in queue element

2. **High Latency**: Element latency > 50ms
   - At 30 fps input, frame time is ~33ms
   - > 50ms indicates processing delay
   - Solution: Reduce complexity or parallelize

3. **Frame Drops**: Drop ratio > 1%
   - Consumer can't keep up
   - Solution: Optimize or reduce pipeline complexity

4. **Jitter**: Timing variance > 5ms
   - Synchronization issues
   - Solution: Adjust clock sync settings

### Common Bottleneck Scenarios

#### Scenario 1: Recording Queue Overflow

**Symptom**: Queue max-size-buffers hit during recording

```
Queue Statistics:
  Max Queue Depth: 10 buffers (at limit)
  Frame drop ratio: 2.5%
```

**Root Cause**: Recording rate > storage rate

**Solution**:
```c
g_object_set(queue, "max-size-buffers", 15, NULL);  // Increase buffer
```

#### Scenario 2: Videomixer Latency

**Symptom**: 120 fps target not achieved, average 110 fps

```
Element Latency Analysis:
  videomixer: 25ms average
  osxvideosink: 15ms average
```

**Root Cause**: Videomixer composition too slow

**Solution**:
```c
g_object_set(videomixer, "latency", 0, NULL);  // Minimize latency
g_object_set(videomixer, "background", 0, NULL);  // Black background faster
```

#### Scenario 3: Synchronization Jitter

**Symptom**: FPS variance > 5 fps (110-125 fps)

```
Synchronization Metrics:
  Jitter: 12ms
  Max latency: 67ms
```

**Root Cause**: Clock sync issues in osxvideosink

**Solution**:
```c
g_object_set(osxvideosink, "sync", TRUE, NULL);  // Enable frame sync
g_object_set(osxvideosink, "throttle-time", 33333, NULL);  // 30ms for 30fps input
```

## Performance Tuning Checklist

### Before Profiling

- [ ] Build in Release mode (not Debug)
- [ ] Disable logging if enabled
- [ ] Close other applications
- [ ] Allow GPU to warm up (run for 10s before measuring)

### During Profiling

- [ ] Run for at least 30 seconds (to see steady-state behavior)
- [ ] Trigger multiple recordings (exercise all code paths)
- [ ] Monitor system temperature
- [ ] Capture baseline metrics

### After Profiling

- [ ] Analyze FPS statistics (target: 120 ± 2 fps)
- [ ] Check queue statistics (no overflow)
- [ ] Review element latency (< 50ms per element)
- [ ] Verify buffer drop ratio (< 1%)
- [ ] Compare against baseline

### Optimization Priority

1. **Critical** (blocking 120 fps):
   - Queue overflow → increase max-size-buffers
   - High drop ratio → reduce pipeline complexity
   - Element latency > 100ms → investigate element

2. **Important** (affecting stability):
   - FPS variance > 5 fps → adjust sync settings
   - Jitter > 5ms → tune clock synchronization
   - Memory growth > 10%/hour → check for leaks

3. **Nice-to-have** (optimization):
   - CPU load < 2% → fine-tune element properties
   - Latency < 20ms → pipeline optimization

## Queue Configuration Guidelines

### Live Feed Queue (Cell 1)

```c
g_object_set(live_queue,
    "max-size-buffers", 1,      // Latest frame only (no buffering)
    "max-size-bytes", 0,        // Unlimited (GPU memory)
    "max-size-time", 0,         // Unlimited
    NULL);
```

### Recording Queues (Cells 2-10)

```c
g_object_set(record_queue,
    "max-size-buffers", 10,     // 1 second at 10 fps (or adjust for actual rate)
    "max-size-bytes", 0,        // Unlimited (GPU memory)
    "max-size-time", 0,         // Unlimited
    NULL);
```

**Calculation for buffer size**:
- Input FPS: 30 fps
- Frame time: 33.3ms
- Target latency: 300ms (3 frames)
- Min buffers: 3-5
- Recommended: 10 (provides 1 second buffer)
- Maximum useful: 20 (diminishing returns beyond this)

## Profiling Output Interpretation

### Report Sections

#### Frame Rate Statistics

```
Current FPS:  119.8
Average FPS:  120.1
Min FPS:      118.2
Max FPS:      121.9
Std Dev:      1.2
```

**Good**: Std Dev < 2 fps (tight control around 120 fps)
**Warning**: Std Dev > 5 fps (inconsistent frame delivery)
**Bad**: Std Dev > 10 fps (severe synchronization issues)

#### Queue Buffering Statistics

```
Number of Queues:  10
Total Queue Depth: 2.4 MB
Max Queue Level:   8 buffers
```

**Good**: Max level < 80% of configured capacity
**Warning**: Max level > 80% (approaching overflow)
**Bad**: Overflow detected (buffers being dropped)

#### Synchronization Metrics

```
Buffer Drop Ratio: 0.12%
Jitter:            2.3 µs
Max Latency:       45 ms
```

**Good**: Drop < 0.1%, Jitter < 1ms, Latency < 50ms
**Warning**: Drop 0.1-1%, Jitter 1-5ms, Latency 50-100ms
**Bad**: Drop > 1%, Jitter > 5ms, Latency > 100ms

### JSON Export Format

Exported profiling data in JSON format for further analysis:

```json
{
  "profiling_session": {
    "duration_us": 30000000,
    "total_frames_recorded": 3600,
    "total_samples": 30
  },
  "fps_statistics": {
    "current_fps": 120.1,
    "average_fps": 120.05
  },
  "samples": [
    {"timestamp_us": 1000000, "fps": 120.0},
    {"timestamp_us": 1050000, "fps": 120.1},
    ...
  ]
}
```

## Integration with Main Application

To integrate profiling into the main application:

```c
// In main.c
#include "src/utils/profiling.h"

ProfilingContext *profiling_ctx = NULL;

// In initialization
profiling_ctx = profiling_context_create(pipeline, 100, 0);
profiling_start(profiling_ctx);

// In main loop (every 100ms)
profiling_collect_sample(profiling_ctx);

// On shutdown
gchar *report = profiling_generate_report(profiling_ctx);
g_print("Profiling Report:\n%s\n", report);
profiling_export_json(profiling_ctx, "profiling_results.json");
profiling_context_free(profiling_ctx);
g_free(report);
```

## Testing

Unit tests verify profiling infrastructure:

```bash
# Compile and run profiling tests
gcc -o test_profiling_utils \
  src/utils/profiling.c \
  src/utils/logging.c \
  test/unit/test_profiling_utils.c \
  $(pkg-config --cflags --libs glib-2.0 gstreamer-1.0)

./test_profiling_utils
```

Tests cover:
- Context creation and initialization
- Frame timing recording
- Sample collection
- FPS statistics calculation
- Queue statistics querying
- Synchronization metrics
- Report generation
- JSON export

## References

### GStreamer Resources

- [GStreamer Debugging Guide](https://gstreamer.freedesktop.org/documentation/application-development/advanced/debugging.html)
- [GStreamer Pipeline Manual](https://gstreamer.freedesktop.org/documentation/gstreamer/gi-index.html)
- [GST_TRACER Documentation](https://gstreamer.freedesktop.org/documentation/gstreamer/gst-tracer.html)

### Related Tasks

- **T-8.2**: Optimize queue sizes and osxvideosink sync settings
- **T-8.3**: Implement frame drop detection and logging
- **T-8.4**: Benchmark GPU memory and CPU utilization
- **T-8.5**: Validate 120 fps sustained playback

## Troubleshooting

### Q: No debug output appears

**A**: Ensure GST_DEBUG_FILE is writable and directory exists:
```bash
mkdir -p profiling_results
export GST_DEBUG_FILE="profiling_results/debug.log"
```

### Q: Profiling overhead affects measurements

**A**: Disable logging in Release builds:
```bash
./scripts/build.sh --release
```

### Q: Queue statistics not available

**A**: Pipeline must be in PLAYING state to query queue properties.

### Q: FPS significantly lower in profiling mode

**A**: GStreamer tracing has overhead. Use `--gst` backend for initial analysis, then run without profiling for production validation.

## Summary

The profiling infrastructure provides comprehensive tools for identifying and resolving performance bottlenecks in the video-looper pipeline. By following this guide's analysis methodology and optimization recommendations, you can achieve and maintain the 120 fps target across all 10 grid cells.
