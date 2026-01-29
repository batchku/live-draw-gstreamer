#!/bin/bash
# Test Verification Script
# Verifies that all unit test files exist and have proper structure

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'  # No Color

test_dir="test/unit"
total_tests=0
total_assertions=0
modules_ok=0

echo "=========================================="
echo "Unit Test File Verification"
echo "=========================================="
echo

# Test 1: Recording State
echo -e "${YELLOW}Checking: test_recording_state.c${NC}"
if [ -f "$test_dir/test_recording_state.c" ]; then
    tests=$(grep -c "^static int test_\|^static gboolean test_" "$test_dir/test_recording_state.c" || true)
    assertions=$(grep -c "assert(" "$test_dir/test_recording_state.c" || true)
    echo -e "${GREEN}✓ File exists${NC}"
    echo "  - Test functions: $tests"
    echo "  - Assertions: $assertions"
    total_tests=$((total_tests + tests))
    total_assertions=$((total_assertions + assertions))
    modules_ok=$((modules_ok + 1))
else
    echo -e "${RED}✗ File not found${NC}"
fi
echo

# Test 2: Buffer Manager
echo -e "${YELLOW}Checking: test_buffer_manager.c${NC}"
if [ -f "$test_dir/test_buffer_manager.c" ]; then
    tests=$(grep -c "^static void test_" "$test_dir/test_buffer_manager.c" || true)
    assertions=$(grep -c "ASSERT_" "$test_dir/test_buffer_manager.c" || true)
    echo -e "${GREEN}✓ File exists${NC}"
    echo "  - Test functions: $tests"
    echo "  - Assertions: $assertions"
    total_tests=$((total_tests + tests))
    total_assertions=$((total_assertions + assertions))
    modules_ok=$((modules_ok + 1))
else
    echo -e "${RED}✗ File not found${NC}"
fi
echo

# Test 3: Playback Manager
echo -e "${YELLOW}Checking: test_playback_manager.c${NC}"
if [ -f "$test_dir/test_playback_manager.c" ]; then
    tests=$(grep -c "^static gboolean test_" "$test_dir/test_playback_manager.c" || true)
    assertions=$(grep -c "fprintf(stderr.*FAIL\|fprintf(stdout.*PASS" "$test_dir/test_playback_manager.c" || true)
    echo -e "${GREEN}✓ File exists${NC}"
    echo "  - Test functions: $tests"
    echo "  - Assertions/Checks: $assertions"
    total_tests=$((total_tests + tests))
    total_assertions=$((total_assertions + assertions))
    modules_ok=$((modules_ok + 1))
else
    echo -e "${RED}✗ File not found${NC}"
fi
echo

# Test 4: Keyboard Handler
echo -e "${YELLOW}Checking: test_keyboard_handler.c${NC}"
if [ -f "$test_dir/test_keyboard_handler.c" ]; then
    tests=$(grep -c "^static void test_" "$test_dir/test_keyboard_handler.c" || true)
    assertions=$(grep -c "ASSERT_" "$test_dir/test_keyboard_handler.c" || true)
    echo -e "${GREEN}✓ File exists${NC}"
    echo "  - Test functions: $tests"
    echo "  - Assertions: $assertions"
    total_tests=$((total_tests + tests))
    total_assertions=$((total_assertions + assertions))
    modules_ok=$((modules_ok + 1))
else
    echo -e "${RED}✗ File not found${NC}"
fi
echo

echo "=========================================="
echo "Summary"
echo "=========================================="
echo "Modules with tests: $modules_ok/4"
echo "Total test functions: $total_tests"
echo "Total assertions: $total_assertions"
echo

if [ $total_tests -ge 25 ]; then
    echo -e "${GREEN}✓ PASS: Minimum 25 tests requirement met ($total_tests tests)${NC}"
    exit 0
else
    echo -e "${RED}✗ FAIL: Minimum 25 tests requirement not met ($total_tests tests)${NC}"
    exit 1
fi
