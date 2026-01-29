#!/bin/bash

##
## @file analyze_profiling.sh
## @brief Analyze GStreamer pipeline profiling results
##
## Parses GStreamer debug logs and profiling output to identify:
## - Queue buffering bottlenecks
## - Synchronization issues
## - Frame rate consistency
## - Bottleneck elements
##
## Usage:
##   ./scripts/analyze_profiling.sh [profiling_results_dir]
##

set -e

PROFILING_DIR="${1:-.profiling_results}"

if [ ! -d "$PROFILING_DIR" ]; then
    echo "Error: Profiling directory not found: $PROFILING_DIR"
    echo "Run './scripts/profile.sh' first to generate profiling data"
    exit 1
fi

echo "=========================================="
echo "GStreamer Pipeline Profiling Analysis"
echo "=========================================="
echo ""
echo "Profiling Directory: $PROFILING_DIR"
echo ""

# Find latest debug log
DEBUG_LOG=$(ls -t "$PROFILING_DIR"/gst_debug_*.log 2>/dev/null | head -1)
if [ -z "$DEBUG_LOG" ]; then
    echo "No GStreamer debug logs found in $PROFILING_DIR"
    exit 1
fi

echo "Analyzing: $(basename "$DEBUG_LOG")"
echo ""

# Create report file
REPORT_FILE="$PROFILING_DIR/profiling_analysis_$(date +%Y%m%d_%H%M%S).txt"

{
    echo "GStreamer Pipeline Profiling Analysis Report"
    echo "==========================================="
    echo "Generated: $(date)"
    echo "Analysis file: $DEBUG_LOG"
    echo ""

    # Queue Analysis
    echo "1. Queue Buffering Analysis"
    echo "============================="
    echo ""
    echo "Queue Elements Detected:"
    grep -i "queue" "$DEBUG_LOG" | grep -i "buffer\|size\|level" | head -10 || \
        echo "  (No queue statistics in debug log)"
    echo ""

    # Latency Analysis
    echo "2. Element Latency Analysis"
    echo "============================"
    echo ""
    echo "Latency Measurements:"
    grep -i "latency" "$DEBUG_LOG" | head -10 || \
        echo "  (No latency data in debug log)"
    echo ""

    # Synchronization Analysis
    echo "3. Synchronization & Buffer Drop Analysis"
    echo "=========================================="
    echo ""
    echo "Synchronization Events:"
    grep -i "sync\|drop\|skip\|overflow" "$DEBUG_LOG" | head -10 || \
        echo "  (No sync/drop events in debug log)"
    echo ""

    # Frame Rate Analysis
    echo "4. Frame Rate Statistics"
    echo "========================"
    echo ""
    echo "Target: 120 fps"
    echo "Input (camera): 30 fps"
    echo "Playback interpolation: 4x (30 → 120 fps)"
    echo ""
    echo "Frame timing events:"
    grep -i "frame\|fps\|framerate" "$DEBUG_LOG" | head -10 || \
        echo "  (No frame timing data in debug log)"
    echo ""

    # Element Performance
    echo "5. Element Performance"
    echo "======================"
    echo ""
    echo "Processing time per element:"
    grep -i "time\|duration\|processing" "$DEBUG_LOG" | grep -i "element\|pad" | head -10 || \
        echo "  (No per-element timing in debug log)"
    echo ""

    # Warnings and Errors
    echo "6. Warnings and Errors"
    echo "======================"
    echo ""
    echo "Critical Issues:"
    grep -i "error\|critical" "$DEBUG_LOG" | head -5 || \
        echo "  (No critical errors)"
    echo ""

    echo "Warnings:"
    grep -i "warning" "$DEBUG_LOG" | head -5 || \
        echo "  (No warnings)"
    echo ""

    # Optimization Recommendations
    echo "7. Optimization Recommendations"
    echo "==============================="
    echo ""

    # Check for queue overflows
    if grep -q "queue.*full\|buffer.*overflow" "$DEBUG_LOG" 2>/dev/null; then
        echo "⚠ ISSUE DETECTED: Queue buffer overflow"
        echo "  Recommendation: Increase max-size-buffers in queue elements"
        echo "  Current config: max-size-buffers=10"
        echo "  Suggested: max-size-buffers=15-20 (for 30fps → 120fps interpolation)"
        echo ""
    fi

    # Check for high latency
    if grep -q "latency.*[0-9][0-9][0-9][0-9]" "$DEBUG_LOG" 2>/dev/null; then
        echo "⚠ ISSUE DETECTED: High element latency"
        echo "  Recommendation: Profile individual elements with gst-launch-1.0"
        echo "  Command: gst-launch-1.0 -v <pipeline> 2>&1 | grep latency"
        echo ""
    fi

    # Check for frame drops
    if grep -q "drop\|skip" "$DEBUG_LOG" 2>/dev/null; then
        echo "⚠ ISSUE DETECTED: Frame drops detected"
        echo "  Recommendation: Reduce pipeline complexity or increase GPU resources"
        echo ""
    fi

    # Standard recommendations
    echo "✓ Standard Optimization Checklist:"
    echo "  1. Queue max-size-buffers: minimum 10 (1 second at 30fps)"
    echo "  2. osxvideosink sync=true for frame-accurate rendering"
    echo "  3. videomixer latency=0 to minimize composition delay"
    echo "  4. GPU-only processing: no CPU/GPU frame transfers"
    echo "  5. Monitor CPU load: target < 5% for video processing"
    echo ""

    # Generate Summary Table
    echo "8. Configuration Summary"
    echo "========================"
    echo ""
    cat <<'SUMMARY'
Key Pipeline Configuration:
  Source:      videotestsrc (or AVFoundation camera)
  Resolution:  1920×1080
  Frame Rate:  30 fps (input)
  Format:      BGRx (GPU-friendly)

Recording Queues (9 total):
  Max Buffers: 10 per queue
  Strategy:    Ring buffer, drop oldest if full
  Purpose:     Store frames during keyboard-triggered recording

Playback Path:
  Videomixer:  Composite 10 video streams to grid
  Playback:    Palindrome loop (forward → reverse → repeat)
  Output:      10-cell horizontal layout (320px per cell)

Rendering Target:
  FPS Target:  120 fps sustained
  Variance:    ±2 fps acceptable (118-122 fps)
  CPU Load:    < 5% for video processing
  GPU Memory:  ~3.4 GB for 9 simultaneous recordings

SUMMARY

} | tee "$REPORT_FILE"

echo ""
echo "Analysis saved to: $REPORT_FILE"
echo ""
echo "=========================================="
echo "Analysis Complete"
echo "=========================================="
