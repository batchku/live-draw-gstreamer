#!/bin/bash
#
# Run script for Video Looper
#
# Usage: ./scripts/run.sh [options]
# Options:
#   --build          Build before running
#   --debug          Run with debug output
#   --valgrind       Run under Valgrind memory profiler
#

set -e

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( dirname "$SCRIPT_DIR" )"
BUILD_DIR="$PROJECT_ROOT/build"

# Default options
BUILD_FIRST=false
DEBUG_MODE=false
USE_VALGRIND=false

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --build)
      BUILD_FIRST=true
      shift
      ;;
    --debug)
      DEBUG_MODE=true
      shift
      ;;
    --valgrind)
      USE_VALGRIND=true
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

# Run the executable
echo "=========================================="
echo "Video Looper - Starting Application"
echo "=========================================="
echo "Executable: $BUILD_DIR/video-looper"
echo "Debug Mode: $DEBUG_MODE"
echo ""

if [ "$USE_VALGRIND" = true ]; then
  # Run under Valgrind for memory profiling
  echo "Running under Valgrind memory profiler..."
  echo ""
  valgrind --leak-check=full \
           --show-leak-kinds=all \
           --track-origins=yes \
           --verbose \
           "$BUILD_DIR/video-looper"
elif [ "$DEBUG_MODE" = true ]; then
  # Run with GStreamer debug output
  export GST_DEBUG=3
  export GST_DEBUG_FILE="$BUILD_DIR/gstreamer.log"
  echo "GStreamer debug enabled (output to $GST_DEBUG_FILE)"
  "$BUILD_DIR/video-looper"
else
  # Run normally
  "$BUILD_DIR/video-looper"
fi

echo ""
echo "=========================================="
echo "Application terminated"
echo "=========================================="
