#!/bin/bash
#
# Build script for Video Looper
#
# Usage: ./scripts/build.sh [options]
# Options:
#   --debug          Build with debug symbols
#   --release        Build optimized release (default)
#   --clean          Clean build directory before building
#   --verbose        Show detailed build output
#

set -e

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( dirname "$SCRIPT_DIR" )"

# Configure pkg-config to find framework .pc files
export PKG_CONFIG_PATH="$PROJECT_ROOT/pkgconfig:$PKG_CONFIG_PATH"

# Default options
BUILD_TYPE="release"
CLEAN_BUILD=false
VERBOSE=false

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --debug)
      BUILD_TYPE="debug"
      shift
      ;;
    --release)
      BUILD_TYPE="release"
      shift
      ;;
    --clean)
      CLEAN_BUILD=true
      shift
      ;;
    --verbose)
      VERBOSE=true
      shift
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

# Print header
echo "=========================================="
echo "Video Looper - Build Script"
echo "=========================================="
echo "Build Type: $BUILD_TYPE"
echo "Project Root: $PROJECT_ROOT"
echo "PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
echo ""

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
  echo "Cleaning build directory..."
  if [ -d "$PROJECT_ROOT/build" ]; then
    rm -rf "$PROJECT_ROOT/build"
  fi
  echo "Build directory cleaned."
  echo ""
fi

# Create build directory if it doesn't exist
if [ ! -d "$PROJECT_ROOT/build" ]; then
  echo "Creating build directory..."
  mkdir -p "$PROJECT_ROOT/build"
  echo ""
fi

# Run Meson configuration if not already configured
if [ ! -f "$PROJECT_ROOT/build/meson-private/coredata.dat" ]; then
  echo "Configuring Meson build system..."
  cd "$PROJECT_ROOT"

  MESON_ARGS="--buildtype=$BUILD_TYPE"

  if [ "$VERBOSE" = true ]; then
    MESON_ARGS="$MESON_ARGS --debug"
  fi

  meson setup build $MESON_ARGS
  echo ""
fi

# Compile the project
echo "Compiling project..."
cd "$PROJECT_ROOT/build"

if [ "$VERBOSE" = true ]; then
  ninja -v
else
  ninja
fi

echo ""
echo "=========================================="
echo "Build complete!"
echo "Executable: $PROJECT_ROOT/build/video-looper"
echo "=========================================="
echo ""
