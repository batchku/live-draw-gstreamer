#!/bin/bash
#
# Performance profiling script for Video Looper
#
# Profiles GStreamer pipeline using:
# - gst-tracepoints for element latency and queue buffering
# - perf for CPU profiling
# - GStreamer debug logging for synchronization analysis
#
# Usage: ./scripts/profile.sh [options]
# Options:
#   --build          Build before profiling
#   --gst            Profile GStreamer pipeline (GST_TRACERS) - DEFAULT
#   --cpu            Profile CPU usage (perf)
#   --memory         Profile memory usage (valgrind)
#   --gpu            Profile GPU usage (requires gputop or similar)
#   --duration N     Run profiler for N seconds (default: 30)
#

set -e

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( dirname "$SCRIPT_DIR" )"
BUILD_DIR="$PROJECT_ROOT/build"
PROFILING_DIR="$PROJECT_ROOT/profiling_results"

# Default options
BUILD_FIRST=false
PROFILE_GST=true
PROFILE_CPU=false
PROFILE_MEMORY=false
PROFILE_GPU=false
DURATION=30

# Create profiling results directory
mkdir -p "$PROFILING_DIR"

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --build)
      BUILD_FIRST=true
      shift
      ;;
    --gst)
      PROFILE_GST=true
      PROFILE_CPU=false
      PROFILE_MEMORY=false
      PROFILE_GPU=false
      shift
      ;;
    --cpu)
      PROFILE_GST=false
      PROFILE_CPU=true
      PROFILE_MEMORY=false
      PROFILE_GPU=false
      shift
      ;;
    --memory)
      PROFILE_GST=false
      PROFILE_CPU=false
      PROFILE_MEMORY=true
      PROFILE_GPU=false
      shift
      ;;
    --gpu)
      PROFILE_GST=false
      PROFILE_CPU=false
      PROFILE_MEMORY=false
      PROFILE_GPU=true
      shift
      ;;
    --duration)
      shift
      DURATION=$1
      shift
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

# Build if requested
if [ "$BUILD_FIRST" = true ]; then
  echo "Building project..."
  "$SCRIPT_DIR/build.sh"
  echo ""
fi

# Check if executable exists
if [ ! -f "$BUILD_DIR/video-looper" ]; then
  echo "Error: Executable not found at $BUILD_DIR/video-looper"
  echo "Please run: ./scripts/build.sh"
  exit 1
fi

# Print header
echo "=========================================="
echo "Video Looper - Performance Profiling"
echo "=========================================="
echo "Duration: $DURATION seconds"
echo "GST Profile: $PROFILE_GST"
echo "CPU Profile: $PROFILE_CPU"
echo "Memory Profile: $PROFILE_MEMORY"
echo "GPU Profile: $PROFILE_GPU"
echo "Results: $PROFILING_DIR"
echo ""

# GStreamer tracepoint profiling
if [ "$PROFILE_GST" = true ]; then
  echo "GStreamer Pipeline Profiling (gst-tracepoints)..."
  echo ""

  # Enable GStreamer tracepoints
  export GST_TRACERS="latency(flags=pipeline+element)"
  export GST_DEBUG="GST_TRACER:7"
  export GST_DEBUG_FILE="$PROFILING_DIR/gst_debug_$(date +%s).log"

  echo "  GST_TRACERS=$GST_TRACERS"
  echo "  GST_DEBUG=$GST_DEBUG"
  echo "  Output: $GST_DEBUG_FILE"
  echo ""
  echo "  Monitoring:"
  echo "  - Queue buffering depth and utilization"
  echo "  - Element latency (source, compositor, sink)"
  echo "  - Buffer synchronization timing"
  echo "  - Frame drop detection"
  echo ""

  # Run application with tracing
  echo "  Starting profiling run for $DURATION seconds..."
  timeout "$DURATION" "$BUILD_DIR/video-looper" > "$PROFILING_DIR/app_output_$(date +%s).log" 2>&1 || true

  echo ""
  echo "  Analysis:"
  if [ -f "$GST_DEBUG_FILE" ]; then
    echo "    - Queue statistics extracted from debug log"
    grep -i "queue\|latency\|buffer" "$GST_DEBUG_FILE" | head -20 || echo "    (No queue/latency data in debug log)"
  fi

elif [ "$PROFILE_CPU" = true ]; then
  echo "CPU Profiling..."

  if command -v perf &> /dev/null; then
    perf record -o "$BUILD_DIR/perf.data" \
      -d "$DURATION" \
      "$BUILD_DIR/video-looper"
    echo ""
    echo "CPU Profile Report:"
    perf report -i "$BUILD_DIR/perf.data"
  else
    echo "Warning: 'perf' not found. Install 'linux-tools' or similar."
    echo "Running without profiler..."
    "$BUILD_DIR/video-looper" &
    PID=$!
    sleep "$DURATION"
    kill $PID || true
  fi

elif [ "$PROFILE_MEMORY" = true ]; then
  echo "Memory Profiling (Valgrind)..."

  if command -v valgrind &> /dev/null; then
    valgrind --tool=massif \
      --massif-out-file="$BUILD_DIR/massif.out" \
      "$BUILD_DIR/video-looper" &
    PID=$!
    sleep "$DURATION"
    kill $PID || true

    echo ""
    echo "Memory Profile Report:"
    if command -v ms_print &> /dev/null; then
      ms_print "$BUILD_DIR/massif.out" | head -50
    fi
  else
    echo "Error: Valgrind not found. Install 'valgrind' package."
    exit 1
  fi

elif [ "$PROFILE_GPU" = true ]; then
  echo "GPU Profiling..."

  if command -v gputop &> /dev/null; then
    echo "Running GStreamer with GPU monitoring..."
    export GST_DEBUG=3
    gputop "$BUILD_DIR/video-looper" &
    PID=$!
    sleep "$DURATION"
    kill $PID || true
  else
    echo "Note: GPU profiling requires gputop or similar tools."
    echo "Running application with GStreamer debug output..."
    export GST_DEBUG=3
    export GST_DEBUG_FILE="$BUILD_DIR/gstreamer-profile.log"
    "$BUILD_DIR/video-looper" &
    PID=$!
    sleep "$DURATION"
    kill $PID || true
    echo "GStreamer debug log: $BUILD_DIR/gstreamer-profile.log"
  fi
fi

echo ""
echo "=========================================="
echo "Profiling completed"
echo "=========================================="
echo ""
echo "Profiling Results Directory: $PROFILING_DIR"
echo "Key Output Files:"
ls -lh "$PROFILING_DIR" 2>/dev/null | tail -10 || echo "(No profiling output files)"
echo ""
echo "To analyze results:"
echo "  - GStreamer debug log: cat $PROFILING_DIR/gst_debug_*.log"
echo "  - Application output: cat $PROFILING_DIR/app_output_*.log"
echo "  - Performance report: scripts/analyze_profiling.sh"
echo ""
