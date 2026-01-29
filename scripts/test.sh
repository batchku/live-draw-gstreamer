#!/bin/bash
#
# Test script for Video Looper
#
# Usage: ./scripts/test.sh [options]
# Options:
#   --build          Build before testing
#   --unit           Run only unit tests
#   --integration    Run only integration tests
#   --coverage       Generate code coverage report
#   --verbose        Show detailed test output
#

set -e

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( dirname "$SCRIPT_DIR" )"
BUILD_DIR="$PROJECT_ROOT/build"

# Default options
BUILD_FIRST=false
TEST_UNIT=true
TEST_INTEGRATION=true
COVERAGE=false
VERBOSE=false

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --build)
      BUILD_FIRST=true
      shift
      ;;
    --unit)
      TEST_UNIT=true
      TEST_INTEGRATION=false
      shift
      ;;
    --integration)
      TEST_UNIT=false
      TEST_INTEGRATION=true
      shift
      ;;
    --coverage)
      COVERAGE=true
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

# Build if requested
if [ "$BUILD_FIRST" = true ]; then
  echo "Building project..."
  "$SCRIPT_DIR/build.sh"
  echo ""
fi

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
  echo "Error: Build directory not found at $BUILD_DIR"
  echo "Please run: ./scripts/build.sh"
  exit 1
fi

# Print header
echo "=========================================="
echo "Video Looper - Test Suite"
echo "=========================================="
echo "Unit Tests: $TEST_UNIT"
echo "Integration Tests: $TEST_INTEGRATION"
echo "Code Coverage: $COVERAGE"
echo ""

# Run tests using Meson
cd "$BUILD_DIR"

if [ "$COVERAGE" = true ]; then
  # Run tests with coverage
  echo "Running tests with code coverage..."
  meson test --coverage
  echo ""
  echo "Generating coverage report..."
  # Generate HTML coverage report if possible
  if command -v gcovr &> /dev/null; then
    gcovr --html-details coverage.html --root "$PROJECT_ROOT"
    echo "Coverage report generated: coverage.html"
  fi
else
  # Run tests normally
  TEST_ARGS="--no-rebuild"

  if [ "$VERBOSE" = true ]; then
    TEST_ARGS="$TEST_ARGS --print-errorlogs"
  fi

  if [ "$TEST_UNIT" = true ] && [ "$TEST_INTEGRATION" = false ]; then
    meson test $TEST_ARGS --suite unit
  elif [ "$TEST_UNIT" = false ] && [ "$TEST_INTEGRATION" = true ]; then
    meson test $TEST_ARGS --suite integration
  else
    meson test $TEST_ARGS
  fi
fi

echo ""
echo "=========================================="
echo "Test execution completed"
echo "=========================================="
