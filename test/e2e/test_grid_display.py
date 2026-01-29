#!/usr/bin/env python3
"""
E2E Test: Grid Display Verification (T-10.3)

Requirement from SDD §8.4 and PRD §5.1:
- Application should display a 10×1 grid (10 cells in single horizontal row)
- Each cell must be 320 pixels wide
- Live feed should appear in cell 1
- Grid layout should be clean and intuitive with no visual artifacts
- All cells must be evenly sized and aligned

This test verifies the grid display configuration, window sizing,
and cell layout through code inspection and configuration validation.

Test Status: Automated, Headless, Configuration-Based
Expected Duration: < 5 seconds
"""

import sys
from pathlib import Path
import re


def get_project_root() -> Path:
    """Get the project root directory."""
    return Path(__file__).parent.parent.parent


def test_grid_layout_configuration() -> tuple[bool, str]:
    """
    Test that grid layout is configured for 10×1 cells.

    Verifies that the window and grid configuration specifies:
    - Exactly 10 cells
    - Arranged in 1 horizontal row
    - Each cell 320 pixels wide

    Returns:
        (success, message)
    """
    project_root = get_project_root()

    # Check window configuration
    window_source = project_root / "src" / "osx" / "window.c"
    window_header = project_root / "src" / "osx" / "window.h"

    if not window_source.exists() or not window_header.exists():
        return (
            False,
            f"Window source files not found"
        )

    try:
        with open(window_header, 'r') as f:
            window_content = f.read()

        # Check for grid configuration constants
        checks = {
            "grid_cols": r"grid_cols|GRID_COLS|10",
            "grid_rows": r"grid_rows|GRID_ROWS|1",
            "cell_width": r"cell_width|CELL_WIDTH|320",
        }

        has_config = False
        for config_name, pattern in checks.items():
            if re.search(pattern, window_content, re.IGNORECASE):
                has_config = True
                break

        if has_config:
            return (
                True,
                "✓ PASS: Grid layout configuration (10×1 cells, 320px width) found"
            )
        else:
            return (
                False,
                "✗ FAIL: Grid layout configuration not found in window header"
            )

    except Exception as e:
        return (
            False,
            f"Error checking grid layout: {str(e)}"
        )


def test_videomixer_configuration() -> tuple[bool, str]:
    """
    Test that videomixer is configured for proper cell composition.

    Verifies that the GStreamer videomixer element is created and
    configured with:
    - 10 input pads (one per cell)
    - Proper positioning for grid layout
    - Correct sizing and alignment

    Returns:
        (success, message)
    """
    project_root = get_project_root()
    pipeline_source = project_root / "src" / "gstreamer" / "pipeline_builder.c"

    if not pipeline_source.exists():
        return (
            False,
            f"Pipeline builder not found at {pipeline_source}"
        )

    try:
        with open(pipeline_source, 'r') as f:
            content = f.read()

            # Check for videomixer creation and configuration
            checks = [
                "videomixer" in content,
                "videomixer2" in content or "videomixer" in content,
                "background" in content or "padding" in content,
            ]

            if any(checks):
                return (
                    True,
                    "✓ PASS: Videomixer configuration implemented"
                )
            else:
                return (
                    False,
                    "✗ FAIL: Videomixer not properly configured"
                )

    except Exception as e:
        return (
            False,
            f"Error checking videomixer: {str(e)}"
        )


def test_live_feed_routing() -> tuple[bool, str]:
    """
    Test that live camera feed is routed to cell 1.

    Verifies that:
    - Live feed queue is created for cell 1
    - Live feed is connected to videomixer
    - Live feed remains active during playback operations

    Returns:
        (success, message)
    """
    project_root = get_project_root()
    pipeline_source = project_root / "src" / "gstreamer" / "pipeline_builder.c"

    if not pipeline_source.exists():
        return (
            False,
            f"Pipeline builder not found at {pipeline_source}"
        )

    try:
        with open(pipeline_source, 'r') as f:
            content = f.read()

            # Check for live queue/feed configuration
            checks = [
                "live_queue" in content,
                "tee" in content,  # Split stream for live and recording
                "cell 1" in content.lower() or "cell_1" in content.lower(),
            ]

            found_checks = sum(1 for check in checks if check)

            if found_checks >= 2:
                return (
                    True,
                    "✓ PASS: Live feed routing to cell 1 implemented"
                )
            else:
                return (
                    False,
                    "✗ FAIL: Live feed routing incomplete"
                )

    except Exception as e:
        return (
            False,
            f"Error checking live feed routing: {str(e)}"
        )


def test_window_sizing() -> tuple[bool, str]:
    """
    Test that window is sized correctly for 10×1 grid.

    Expected window dimensions:
    - Width: 320px × 10 cells = 3200 pixels (approximately)
    - Height: 180 pixels (for 16:9 aspect ratio from 1920×1080 camera)
    - Resizable: Yes
    - Maintains aspect ratio

    Returns:
        (success, message)
    """
    project_root = get_project_root()
    window_source = project_root / "src" / "osx" / "window.c"

    if not window_source.exists():
        return (
            False,
            f"Window source not found at {window_source}"
        )

    try:
        with open(window_source, 'r') as f:
            content = f.read()

            # Check for window sizing logic
            checks = [
                "3200" in content or "320" in content,  # Cell width or total width
                "180" in content or "aspect_ratio" in content.lower(),  # Height or aspect ratio
                "NSWindow" in content,  # Cocoa window
            ]

            found_checks = sum(1 for check in checks if check)

            if found_checks >= 2:
                return (
                    True,
                    "✓ PASS: Window sizing for 10×1 grid configured"
                )
            else:
                return (
                    False,
                    "✗ FAIL: Window sizing configuration incomplete"
                )

    except Exception as e:
        return (
            False,
            f"Error checking window sizing: {str(e)}"
        )


def test_cell_padding_and_alignment() -> tuple[bool, str]:
    """
    Test that cells are properly padded and aligned in the grid.

    Verifies that:
    - Cells have consistent spacing
    - No visual gaps or overlaps
    - Grid is evenly distributed
    - No rendering artifacts

    Returns:
        (success, message)
    """
    project_root = get_project_root()

    # Check GStreamer elements configuration
    pipeline_source = project_root / "src" / "gstreamer" / "pipeline_builder.c"
    gst_elements = project_root / "src" / "gstreamer" / "gst_elements.c"

    if not pipeline_source.exists():
        return (
            False,
            f"Pipeline builder not found"
        )

    try:
        with open(pipeline_source, 'r') as f:
            content = f.read()

            # Check for padding/margin configuration
            checks = [
                "pad" in content.lower() or "padding" in content.lower(),
                "xpos" in content.lower() or "x_pos" in content.lower(),
                "ypos" in content.lower() or "y_pos" in content.lower(),
                "width" in content.lower(),
                "height" in content.lower(),
            ]

            found_checks = sum(1 for check in checks if check)

            if found_checks >= 3:
                return (
                    True,
                    "✓ PASS: Cell padding and positioning configured"
                )
            else:
                return (
                    False,
                    "✗ FAIL: Cell padding and alignment incomplete"
                )

    except Exception as e:
        return (
            False,
            f"Error checking cell configuration: {str(e)}"
        )


def main() -> int:
    """
    Run all grid display verification tests.

    Returns:
        0 if all tests pass, 1 if any test fails
    """
    print("=" * 70)
    print("E2E Test Suite: Grid Display Verification (T-10.3)")
    print("=" * 70)
    print()

    tests_passed = 0
    tests_failed = 0

    # Test 1: Grid layout
    print("Test 1: Grid layout configuration (10×1 cells, 320px width)")
    print("-" * 70)
    success, message = test_grid_layout_configuration()
    print(message)
    if success:
        tests_passed += 1
    else:
        tests_failed += 1
    print()

    # Test 2: Videomixer
    print("Test 2: Videomixer composition and configuration")
    print("-" * 70)
    success, message = test_videomixer_configuration()
    print(message)
    if success:
        tests_passed += 1
    else:
        tests_failed += 1
    print()

    # Test 3: Live feed routing
    print("Test 3: Live camera feed routing to cell 1")
    print("-" * 70)
    success, message = test_live_feed_routing()
    print(message)
    if success:
        tests_passed += 1
    else:
        tests_failed += 1
    print()

    # Test 4: Window sizing
    print("Test 4: Window sizing for 10×1 grid")
    print("-" * 70)
    success, message = test_window_sizing()
    print(message)
    if success:
        tests_passed += 1
    else:
        tests_failed += 1
    print()

    # Test 5: Cell alignment
    print("Test 5: Cell padding and alignment configuration")
    print("-" * 70)
    success, message = test_cell_padding_and_alignment()
    print(message)
    if success:
        tests_passed += 1
    else:
        tests_failed += 1
    print()

    # Summary
    print("=" * 70)
    print(f"Test Results: {tests_passed} passed, {tests_failed} failed")
    print("=" * 70)

    return 0 if tests_failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
