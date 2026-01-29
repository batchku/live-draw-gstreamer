#!/usr/bin/env python3
"""
E2E Test Runner for Video Looper (T-10.3)

Executes all E2E tests and generates a comprehensive test report.
This script runs:
1. Application launch time tests
2. Camera permission handling tests
3. Grid display verification tests

Usage:
    python3 test/e2e/run_e2e_tests.py

Exit Codes:
    0 = All tests passed
    1 = One or more tests failed
"""

import subprocess
import sys
from pathlib import Path
from datetime import datetime


def get_project_root() -> Path:
    """Get the project root directory."""
    return Path(__file__).parent.parent.parent


def run_test_module(test_file: Path) -> tuple[int, str]:
    """
    Run a single test module and capture output.

    Args:
        test_file: Path to the test Python file

    Returns:
        (exit_code, output)
    """
    try:
        result = subprocess.run(
            [sys.executable, str(test_file)],
            capture_output=True,
            text=True,
            timeout=30
        )
        return result.returncode, result.stdout + result.stderr
    except subprocess.TimeoutExpired:
        return 1, f"Test {test_file.name} timed out"
    except Exception as e:
        return 1, f"Error running {test_file.name}: {str(e)}"


def main() -> int:
    """
    Run all E2E tests and report results.

    Returns:
        0 if all tests pass, 1 if any fail
    """
    project_root = get_project_root()
    e2e_dir = project_root / "test" / "e2e"

    # Test modules to run
    test_modules = [
        e2e_dir / "test_app_launch.py",
        e2e_dir / "test_camera_permission.py",
        e2e_dir / "test_grid_display.py",
    ]

    print("=" * 80)
    print("E2E Test Suite Runner - Video Looper macOS")
    print("=" * 80)
    print(f"Start Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print()

    overall_passed = 0
    overall_failed = 0
    test_results = []

    # Run each test module
    for test_file in test_modules:
        if not test_file.exists():
            print(f"✗ SKIP: {test_file.name} not found")
            overall_failed += 1
            continue

        print(f"\n{'=' * 80}")
        print(f"Running: {test_file.name}")
        print("=" * 80)

        exit_code, output = run_test_module(test_file)

        # Parse test results from output
        test_name = test_file.stem
        status = "PASS" if exit_code == 0 else "FAIL"
        test_results.append((test_name, status))

        if exit_code == 0:
            overall_passed += 1
        else:
            overall_failed += 1

        # Print test output
        print(output)

    # Print summary report
    print("\n" + "=" * 80)
    print("E2E TEST SUMMARY REPORT")
    print("=" * 80)
    print()

    for test_name, status in test_results:
        symbol = "✓" if status == "PASS" else "✗"
        print(f"{symbol} {test_name}: {status}")

    print()
    print(f"Total Tests Run: {overall_passed + overall_failed}")
    print(f"Tests Passed: {overall_passed}")
    print(f"Tests Failed: {overall_failed}")
    print()
    print(f"End Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    if overall_failed == 0:
        print()
        print("✓ ALL E2E TESTS PASSED")
        print("=" * 80)
        return 0
    else:
        print()
        print("✗ SOME E2E TESTS FAILED")
        print("=" * 80)
        return 1


if __name__ == "__main__":
    sys.exit(main())
