#!/bin/bash

# Test T-8.5: Sustained 120 fps performance test
# Tests maintained 120 fps across 10 cells with CPU/memory/GPU stability

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
TEST_RESULTS_DIR="${PROJECT_ROOT}/test_results"

# Create results directory
mkdir -p "${TEST_RESULTS_DIR}"

echo "=========================================="
echo "Performance Test T-8.5"
echo "Sustained 120 fps across 10 cells"
echo "=========================================="
echo

# Run Python performance test
echo "Running sustained performance test..."
python3 "${SCRIPT_DIR}/test_sustained_performance.py" \
    --duration 30 \
    --target-fps 120 \
    --tolerance 2 \
    --max-cpu 5.0 \
    --max-memory-growth 10.0

TEST_RESULT=$?

# Copy results if test ran
if [ -f "test_results_sustained_120fps.json" ]; then
    cp "test_results_sustained_120fps.json" "${TEST_RESULTS_DIR}/"
    echo
    echo "Test results saved to: ${TEST_RESULTS_DIR}/test_results_sustained_120fps.json"
fi

# Report result
if [ $TEST_RESULT -eq 0 ]; then
    echo
    echo "✓ PASS: Sustained 120 fps performance test successful"
    exit 0
else
    echo
    echo "✗ FAIL: Sustained 120 fps performance test failed"
    exit 1
fi
