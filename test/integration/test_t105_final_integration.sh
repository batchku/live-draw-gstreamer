#!/bin/bash

# Final Integration Test T-10.5: Comprehensive System Validation
#
# This script executes the final integration test verifying:
# 1. 10-cell grid at 120 fps
# 2. 30-minute stability test (simulated)
# 3. Keyboard latency < 50ms (P95)
# 4. Live feed persistence throughout session
#
# Exit codes:
#   0 = PASS - All requirements met
#   1 = FAIL - One or more requirements not met

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
TEST_RESULTS_DIR="${PROJECT_ROOT}/test_results"

# Create results directory
mkdir -p "${TEST_RESULTS_DIR}"

echo "=========================================="
echo "Final Integration Test T-10.5"
echo "Comprehensive System Validation"
echo "=========================================="
echo

# Run Python test
echo "Running final integration test..."
python3 "${SCRIPT_DIR}/test_t105_final_integration.py"
TEST_RESULT=$?

# Copy results if test ran
if [ -f "test_results_t105_final_integration.json" ]; then
    cp "test_results_t105_final_integration.json" "${TEST_RESULTS_DIR}/"
    echo
    echo "Test results saved to: ${TEST_RESULTS_DIR}/test_results_t105_final_integration.json"
fi

# Report result
if [ $TEST_RESULT -eq 0 ]; then
    echo
    echo "✓ PASS: Final integration test T-10.5 successful"
    echo "All requirements verified:"
    echo "  • 10-cell grid at 120 fps: ✓"
    echo "  • 30-minute stability: ✓"
    echo "  • Keyboard latency <50ms: ✓"
    echo "  • Live feed persistence: ✓"
    exit 0
else
    echo
    echo "✗ FAIL: Final integration test T-10.5 failed"
    echo "Review test_results_t105_final_integration.json for details"
    exit 1
fi
