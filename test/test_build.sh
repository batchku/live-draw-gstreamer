#!/bin/bash
#
# Build Test Script: Verify project builds successfully
#
# This script tests:
# 1. Build system configuration is valid
# 2. Project compiles without errors
# 3. Executable is created and is valid
# 4. Executable can be invoked (basic sanity check)
#
# Usage: ./test/test_build.sh
# Exit code: 0 = success, 1 = failure
#

set -e

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Project paths
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
EXECUTABLE="$BUILD_DIR/video-looper"
SCRIPTS_DIR="$PROJECT_ROOT/scripts"

# Test results
PASSED=0
FAILED=0

# Utility functions
log_test() {
  echo -e "${YELLOW}[TEST]${NC} $1"
}

log_pass() {
  echo -e "${GREEN}[PASS]${NC} $1"
  ((PASSED++))
}

log_fail() {
  echo -e "${RED}[FAIL]${NC} $1"
  ((FAILED++))
}

# ============================================================================
# Test 1: Verify project structure
# ============================================================================
log_test "Verifying project structure..."

required_dirs=(
  "src"
  "src/app"
  "src/camera"
  "src/gstreamer"
  "src/recording"
  "src/playback"
  "src/input"
  "src/osx"
  "src/utils"
  "test"
  "scripts"
  "docs"
  "pkgconfig"
)

for dir in "${required_dirs[@]}"; do
  if [ -d "$PROJECT_ROOT/$dir" ]; then
    log_pass "Directory exists: $dir"
  else
    log_fail "Directory missing: $dir"
  fi
done

# ============================================================================
# Test 2: Verify build configuration
# ============================================================================
log_test "Verifying build configuration..."

config_files=(
  "meson.build"
  "meson_options.txt"
  "scripts/build.sh"
  "scripts/run.sh"
  "scripts/test.sh"
)

for file in "${config_files[@]}"; do
  if [ -f "$PROJECT_ROOT/$file" ] && [ -s "$PROJECT_ROOT/$file" ]; then
    log_pass "Configuration file exists: $file"
  else
    log_fail "Configuration file missing or empty: $file"
  fi
done

# ============================================================================
# Test 3: Verify source files exist
# ============================================================================
log_test "Verifying source files..."

required_sources=(
  "src/main.c"
  "src/app/app_context.c"
  "src/app/app_context.h"
  "src/camera/camera_source.c"
  "src/camera/camera_source.h"
  "src/gstreamer/pipeline_builder.c"
  "src/gstreamer/pipeline_builder.h"
  "src/recording/recording_state.c"
  "src/recording/recording_state.h"
  "src/playback/playback_manager.c"
  "src/playback/playback_manager.h"
  "src/input/keyboard_handler.c"
  "src/input/keyboard_handler.h"
  "src/osx/window.c"
  "src/osx/window.h"
  "src/utils/logging.c"
  "src/utils/logging.h"
)

for src in "${required_sources[@]}"; do
  if [ -f "$PROJECT_ROOT/$src" ]; then
    log_pass "Source file exists: $src"
  else
    log_fail "Source file missing: $src"
  fi
done

# ============================================================================
# Test 4: Build the project
# ============================================================================
log_test "Building project (this may take a minute)..."

if bash "$SCRIPTS_DIR/build.sh" --release > /tmp/build.log 2>&1; then
  log_pass "Build succeeded"
else
  log_fail "Build failed (see /tmp/build.log for details)"
  cat /tmp/build.log
fi

# ============================================================================
# Test 5: Verify executable exists
# ============================================================================
log_test "Verifying executable..."

if [ -f "$EXECUTABLE" ]; then
  log_pass "Executable exists: $EXECUTABLE"
else
  log_fail "Executable not created: $EXECUTABLE"
  exit 1
fi

if [ -x "$EXECUTABLE" ]; then
  log_pass "Executable is executable"
else
  log_fail "Executable is not executable"
fi

# ============================================================================
# Test 6: Verify executable is valid binary
# ============================================================================
log_test "Verifying binary format..."

if file "$EXECUTABLE" | grep -q "Mach-O"; then
  log_pass "Executable is valid Mach-O binary"
else
  log_fail "Executable is not a valid Mach-O binary"
fi

# ============================================================================
# Test 7: Verify executable size
# ============================================================================
log_test "Verifying executable size..."

size=$(stat -f%z "$EXECUTABLE")
size_kb=$((size / 1024))

if [ "$size" -gt 10240 ] && [ "$size" -lt 104857600 ]; then
  log_pass "Executable size is reasonable: $size_kb KB"
else
  log_fail "Executable size is suspicious: $size_kb KB"
fi

# ============================================================================
# Test 8: Verify no undefined symbols
# ============================================================================
log_test "Verifying symbol resolution..."

if nm -u "$EXECUTABLE" > /tmp/undefined.log 2>&1; then
  undef_count=$(wc -l < /tmp/undefined.log | tr -d ' ')
  if [ "$undef_count" -lt 100 ]; then
    log_pass "Executable has $undef_count undefined symbols (reasonable for dynamic linking)"
  else
    log_fail "Executable has too many undefined symbols: $undef_count"
  fi
else
  log_fail "Failed to analyze symbols"
fi

# ============================================================================
# Test 9: Verify executable can be invoked
# ============================================================================
log_test "Verifying executable invocation..."

# Run with timeout - the app requires GUI so it may hang
if timeout 2 "$EXECUTABLE" --help > /tmp/exec_test.log 2>&1 || [ $? -eq 124 ]; then
  log_pass "Executable can be invoked (started successfully)"
else
  log_fail "Executable failed to start"
  cat /tmp/exec_test.log
fi

# ============================================================================
# Summary
# ============================================================================
echo ""
echo "=============================================="
echo "Build Test Results"
echo "=============================================="
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo "=============================================="

if [ $FAILED -eq 0 ]; then
  echo -e "${GREEN}✓ All build tests passed!${NC}"
  exit 0
else
  echo -e "${RED}✗ Some build tests failed${NC}"
  exit 1
fi
