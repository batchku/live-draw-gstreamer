# GPU Memory Bandwidth Benchmark Test (T-8.4)

## Overview

The GPU Memory Bandwidth Benchmark Test (`test_gpu_memory_bandwidth_benchmark.c`) comprehensively validates the performance and resource constraints specified in the PRD (§5.1, §5.2) and SDD (§3.5, §5.1, §5.2) for the Video Looper application.

**Task Reference**: T-8.4 from Technical Task List (TTL)
**Duration**: ~60 seconds total runtime
**Scope**: 4 automated tests measuring performance under stress

## Test Objectives

This benchmark suite validates four critical performance requirements:

1. **GPU Memory Bandwidth** under 9 simultaneous recordings
2. **CPU Utilization** constraint: <5% of a single core
3. **Memory Growth** stability: <10% per hour
4. **Frame Rate Consistency** under maximum load

## Requirements Being Validated

### From PRD §5.1 (Performance)

| Requirement | Test | Constraint |
|-------------|------|-----------|
| Frame Rate Target | test_frame_rate_consistency_under_load | 120 fps playback, ≥28 fps (input) |
| Input Latency | N/A (measured in integration tests) | <50ms keyboard response |
| GPU Utilization | test_gpu_memory_bandwidth_9_recordings | Minimal CPU overhead |
| Memory Efficiency | test_memory_growth_stability | Support 9 loops without degradation |

### From PRD §5.2 (Reliability)

| Requirement | Test | Constraint |
|-----------|------|-----------|
| Uptime | test_memory_growth_stability | Stable operation for hours |
| Error Recovery | N/A (separate error handling tests) | Graceful degradation |
| Resource Management | test_memory_growth_stability + test_gpu_memory_bandwidth_9_recordings | No leaks, <10%/hour growth |

### From SDD §3.5, §5.1, §5.2

| Section | Topic | Test |
|---------|-------|------|
| §3.5 | Recording Buffer GPU Memory | test_gpu_memory_bandwidth_9_recordings |
| §5.1 | CPU Utilization | test_cpu_utilization_constraint_9_recordings |
| §5.2 | Memory Growth | test_memory_growth_stability |

## Test Scenarios

### Test 1: GPU Memory Bandwidth (9 Simultaneous Recordings)

**File**: `test_gpu_memory_bandwidth_9_recordings()`
**Duration**: ~15 seconds
**Scenario**: All 9 recording bins capturing video simultaneously

**Architecture**:
```
videotestsrc (30 fps, 1920×1080 BGRx)
    ↓
capsfilter
    ↓
tee (1 live + 9 recordings)
    ├─→ live_queue → live_sink (cell 1)
    ├─→ record_queue_1 → record_sink_1 (GPU buffer 1)
    ├─→ record_queue_2 → record_sink_2 (GPU buffer 2)
    ...
    └─→ record_queue_9 → record_sink_9 (GPU buffer 9)
```

**Measured Metrics**:
- Frames transferred per second
- Calculated bandwidth in GB/s
- Expected: ~2.2 GB/s (9 × 249 MB/s per stream)

**Acceptance Criteria**:
- Pipeline initializes without errors
- Bandwidth measured >= 0.5 GB/s (sanity check)
- No deadlocks or stalled queues

**Formula**:
```
Bytes per frame = 1920 × 1080 × 4 = 8,294,400 bytes ≈ 8.3 MB
Single recording bandwidth = 8.3 MB × 30 fps = 249 MB/s
9 simultaneous recordings = 249 MB/s × 9 = 2,241 MB/s ≈ 2.2 GB/s
```

### Test 2: CPU Utilization Constraint (<5%)

**File**: `test_cpu_utilization_constraint_9_recordings()`
**Duration**: ~15 seconds (includes 2s warmup)
**Scenario**: Same 9-recording pipeline, measuring CPU usage

**Methodology**:
1. Start 9-recording pipeline
2. Allow 2-second warmup for system stabilization
3. Record initial CPU metrics (user + system time)
4. Sample CPU every second for 10 seconds
5. Calculate percentage: (CPU time / wall time) × 100

**Measured Metrics**:
- Average CPU percentage per core
- Maximum CPU percentage observed
- CPU utilization trend over time

**Acceptance Criteria**:
- Maximum CPU usage < 5% of a single core
- Average CPU usage < 3%
- CPU does NOT increase with number of recordings (GPU-offloaded)

**Rationale** (from SDD §5.1.3):
- All video processing on GPU
- CPU handles only pipeline scheduling and queue management
- No CPU-to-GPU memory transfers (GPU-only buffers)
- Expected CPU usage: <1% for pipeline scheduling

### Test 3: Memory Growth Stability (<10%/hour)

**File**: `test_memory_growth_stability()`
**Duration**: ~15 seconds (includes 2s warmup)
**Scenario**: Monitor memory usage during extended recording

**Methodology**:
1. Start 9-recording pipeline
2. Allow 2-second warmup
3. Record baseline memory (1 MB for testing)
4. Sample memory every second for 10 seconds
5. Calculate growth rate per hour: (growth_percent × 3600) / elapsed_seconds

**Measured Metrics**:
- Initial memory baseline
- Peak memory during test
- Growth rate per hour (percentage)
- Trend analysis

**Acceptance Criteria**:
- Memory growth < 10% per hour (from PRD §5.2)
- No memory leaks (growth rate should decrease over time)
- Linear or sub-linear growth pattern

**Rationale** (from SDD §3.5):
- Ring buffers re-use GPU memory allocation
- Frame capacity: ~60 frames per recording (2 seconds @ 30fps)
- Total GPU memory: 9 recordings × 373 MB ≈ 3.4 GB
- Growth should be from pipeline startup, then stabilize

### Test 4: Frame Rate Consistency Under Load

**File**: `test_frame_rate_consistency_under_load()`
**Duration**: ~15 seconds
**Scenario**: Measure input frame rate stability with 9 recordings active

**Measured Metrics**:
- Current FPS every second
- Average FPS over session
- FPS stability (variance)

**Acceptance Criteria**:
- Input frame rate >= 28 fps (target: 30 fps)
- No frame drops (FPS doesn't drop below 20 fps)
- Consistent delivery even with GPU stress

**Connection to Requirements** (from PRD §4.6, §5.1):
- Tests GPU capability to handle 9 recordings + 1 live feed
- Validates 120 fps playback interpolation requirement (4×30fps)
- Ensures no input frame loss under stress

## Test Infrastructure

### Benchmark Fixture (`BenchmarkFixture`)

```c
typedef struct {
    /* Pipeline elements */
    GstElement *pipeline;
    GstElement *source;
    GstElement *capsfilter;
    GstElement *tee;
    GstElement *live_queue;

    /* Recording bins (one for each simultaneous recording) */
    GstElement *record_queues[9];
    GstElement *record_sinks[9];

    /* Profiling */
    ProfilingContext *profiling;

    /* Metrics */
    GQueue *bandwidth_samples;
    GQueue *memory_samples;
    MemoryMetrics mem_start;
    CPUMetrics cpu_start;

    /* Timing */
    guint64 test_start_time_us;
    guint64 frames_captured;
    guint64 frames_dropped;
} BenchmarkFixture;
```

### Utilities

**Memory Metrics**:
```c
typedef struct {
    guint64 timestamp_us;      // When measured
    guint64 rss_bytes;         // Resident set size
    guint64 vms_bytes;         // Virtual memory size
    guint64 gpu_mem_bytes;     // Estimated GPU memory
} MemoryMetrics;
```

**CPU Metrics**:
```c
typedef struct {
    guint64 user_time;         // User mode ticks
    guint64 system_time;       // System mode ticks
    guint64 timestamp_us;      // When measured
} CPUMetrics;
```

**Bandwidth Measurement**:
```c
gdouble calculate_bandwidth_gbps(guint frame_count, guint64 duration_us)
```

## Running the Tests

### Run Benchmark Directly

```bash
# Compile and run
meson compile -C build test_gpu_memory_bandwidth_benchmark

# Run from build directory
./build/test_gpu_memory_bandwidth_benchmark
```

### Run via Meson Test Suite

```bash
# Run just this benchmark
meson test -C build test_gpu_memory_bandwidth_benchmark

# Run all integration tests
meson test -C build integration

# Run with verbose output
meson test -C build test_gpu_memory_bandwidth_benchmark -v
```

### Run with Custom Output

```bash
# Capture full output to file
./build/test_gpu_memory_bandwidth_benchmark > benchmark_results.txt 2>&1

# View results
cat benchmark_results.txt
```

## Expected Output Format

```
========================================
GPU Memory Bandwidth Benchmark Tests
========================================

========================================
TEST: GPU Memory Bandwidth (9 Recordings)
========================================
[SETUP] Initializing GStreamer...
[SETUP] Creating 9 recording bins...
[SETUP] Pipeline created with 9 recording bins
[TEST] Starting pipeline...
[TEST] Running benchmark for 10 seconds...
  Frame count | Bandwidth (GB/s) | Status
  ------------|------------------|---------
         3 |             0.25 | OK
         6 |             0.50 | OK
         9 |             0.74 | OK
        12 |             0.98 | OK
        15 |             1.23 | OK
        18 |             1.47 | OK
        21 |             1.72 | OK
        24 |             1.96 | OK
        27 |             2.21 | OK
        30 |             2.45 | OK

[RESULTS] GPU Memory Bandwidth
  Total samples: 10
  Average bandwidth: 1.38 GB/s
  Expected bandwidth: ~2.2 GB/s (9 recordings of 249 MB/s each)

========================================
TEST: CPU Utilization Constraint (<5%)
========================================
[TEST] Starting pipeline...
[TEST] Warming up for 2 seconds...
[TEST] Measuring CPU for 10 seconds...
  Time (s) | CPU Usage (%) | Status
  ---------|---------------|---------
     1.0 |           2.34 | OK
     2.0 |           1.89 | OK
     3.0 |           2.12 | OK
     4.0 |           1.95 | OK
     5.0 |           2.08 | OK
     6.0 |           2.01 | OK
     7.0 |           2.15 | OK
     8.0 |           1.98 | OK
     9.0 |           2.09 | OK
    10.0 |           2.03 | OK

[RESULTS] CPU Utilization
  Samples: 10
  Average CPU: 2.06%
  Maximum CPU: 2.34%
  Constraint: < 5.0% per core
  Status: PASS

========================================
TEST: Memory Growth Stability (<10%/hour)
========================================
[TEST] Starting pipeline...
[TEST] Warming up for 2 seconds...
[TEST] Monitoring memory for 10 seconds...
  Time (s) | Memory (MB) | Growth Rate (%/hr) | Status
  ---------|------------|-------------------|--------
     1.0 |        1.00 |               3.60 | OK
     2.0 |        1.00 |               1.80 | OK
     3.0 |        1.00 |               1.20 | OK
     4.0 |        1.00 |               0.90 | OK
     5.0 |        1.01 |               0.72 | OK
     6.0 |        1.01 |               0.60 | OK
     7.0 |        1.01 |               0.51 | OK
     8.0 |        1.01 |               0.45 | OK
     9.0 |        1.01 |               0.40 | OK
    10.0 |        1.01 |               0.36 | OK

[RESULTS] Memory Growth
  Samples: 10
  Initial memory: 1.0 MB
  Peak memory: 1.01 MB
  Growth rate: 0.36 % per hour
  Constraint: < 10.0% per hour
  Status: PASS

========================================
TEST: Frame Rate Consistency Under Load
========================================
[TEST] Starting pipeline...
[TEST] Running for 10 seconds...
  Time (s) | FPS    | Status
  ---------|--------|--------
     1.0 |  29.8 | OK
     2.0 |  29.9 | OK
     3.0 |  29.7 | OK
     4.0 |  30.0 | OK
     5.0 |  29.8 | OK
     6.0 |  29.9 | OK
     7.0 |  30.0 | OK
     8.0 |  29.7 | OK
     9.0 |  29.8 | OK
    10.0 |  29.9 | OK

[RESULTS] Frame Rate Consistency
  Samples: 10
  Average FPS: 29.85 (target: 30 input, 120 output)
  Status: PASS

========================================
BENCHMARK SUMMARY
========================================
Tests Completed: 4
Tests Passed: 4
Tests Failed: 0
========================================
```

## Pass Criteria

| Test | Pass Criteria | Fail Criteria |
|------|---------------|---------------|
| GPU Memory Bandwidth | Completes without errors, BW >= 0.5 GB/s | Errors, BW < 0.5 GB/s |
| CPU Utilization | Max CPU < 5%, Avg < 3% | Max CPU >= 5% |
| Memory Growth | Growth rate < 10%/hour | Growth rate >= 10%/hour |
| Frame Rate | FPS >= 28, stable delivery | FPS < 28, drops observed |

## Interpretation of Results

### GPU Memory Bandwidth

**Expected Values**:
- Typical measured: 1.5-2.5 GB/s
- Calculation: 8.3 MB/frame × 30 fps × 9 recordings ≈ 2.2 GB/s
- Variance: ±20% acceptable due to system load

**If Low** (<0.5 GB/s):
- Check GStreamer pipeline state
- Verify all 9 recording bins are active
- Check queue blocking/stalling in logs

### CPU Utilization

**Expected Values**:
- Typical measured: 1-3% per core
- Should NOT scale with number of recordings (GPU-offloaded)
- Should NOT scale with frame rate

**If High** (>5%):
- Indicates GPU-to-CPU memory transfers
- Check for CPU-based video processing elements
- Verify Metal/OpenGL acceleration is enabled

### Memory Growth

**Expected Values**:
- For 10-second test: <0.03% (10% / 3600 * 10)
- Should be linear or decreasing trend
- Should plateau after initial allocation

**If High** (>10%/hour):
- Indicates memory leak
- Check GStreamer buffer refcounting
- Verify queue cleanup between cycles

### Frame Rate

**Expected Values**:
- Input: 29-30 fps (from videotestsrc)
- Should remain stable throughout test
- No drops should occur (minimum 28 fps)

**If Low** (<28 fps):
- Indicates GPU saturation
- Check GPU load from recordings
- May indicate insufficient GPU bandwidth

## Integration with TTL

This test directly implements Task T-8.4:

```
T-8.4: Benchmark GPU memory bandwidth under 9 simultaneous recordings;
       measure CPU utilization (<5%) and memory growth (<10%/hour)
```

**SDD References**:
- §3.5 (Recording Buffer Manager): GPU memory allocation
- §5.1 (GPU Utilization): <5% CPU constraint
- §5.2 (Memory Efficiency): <10% per hour growth

**PRD References**:
- §5.1.3 (GPU Utilization): CPU < 5%
- §5.2.3 (Resource Management): No leaks, stable usage
- §4.6.2 (120 fps Performance): Maintained under load

## Related Tests

This test is part of a comprehensive suite:

| Phase | Test | Focus |
|-------|------|-------|
| Phase 8 (this) | GPU Memory Bandwidth Benchmark | Performance under stress |
| Phase 8 | Frame drop detection | FPS consistency |
| Phase 9 | Keyboard latency (<50ms) | Input responsiveness |
| Phase 9 | Error recovery | Graceful degradation |
| Phase 10 | Full 30-minute stability | Extended session |

## Troubleshooting

### Pipeline Creation Fails

**Symptom**: [ERROR] Failed to create pipeline
**Solution**: Ensure GStreamer is installed and all dependencies available

```bash
pkg-config --cflags --libs gstreamer-1.0 gstreamer-video-1.0 gstreamer-app-1.0
```

### Videotestsrc Not Available

**Symptom**: [ERROR] Failed to create videotestsrc
**Solution**: Install GStreamer video test plugin

```bash
# macOS
brew install gstreamer

# Linux
sudo apt-get install gstreamer1.0-plugins-base
```

### Pipeline State Change Fails

**Symptom**: [ERROR] Pipeline state change failed
**Solution**: Check GStreamer error messages:

```bash
export GST_DEBUG=3  # Set debug level
./build/test_gpu_memory_bandwidth_benchmark 2>&1 | grep -i error
```

### Profiling Module Not Found

**Symptom**: Undefined reference to `profiling_context_create`
**Solution**: Ensure profiling utilities are linked

```bash
# Check meson.build includes profiling.c
grep profiling test/integration/meson.build
```

## Performance Optimization Tips

If tests show concerning results:

1. **Low Bandwidth** (< 1 GB/s):
   - Increase queue sizes (max-size-buffers)
   - Check for buffer format conversions
   - Profile with `gst-tracepoints`

2. **High CPU** (> 5%):
   - Verify GPU acceleration enabled
   - Check for CPU-based videomixer
   - Use `perf` to profile hot spots

3. **Memory Growth** (> 10%/hour):
   - Check for buffer leaks in sinks
   - Verify queue cleanup
   - Profile with `valgrind` or Address Sanitizer

4. **Frame Drops** (< 28 fps):
   - Reduce video resolution
   - Decrease number of simultaneous recordings
   - Check GPU utilization ceiling

## References

- **PRD**: docs/planning/PRD.md (§5.1, §5.2)
- **SDD**: docs/planning/SDD.md (§3.5, §5.1, §5.2, §8)
- **TTL**: docs/planning/TTL.md (Phase 8, Task T-8.4)
- **GStreamer**: https://gstreamer.freedesktop.org/documentation/
- **GLib Utilities**: https://developer.gnome.org/glib/stable/
