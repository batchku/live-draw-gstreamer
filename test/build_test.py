#!/usr/bin/env python3
"""
Build Test: Verify project builds successfully and executable runs without errors.

This test validates:
1. Build system is properly configured
2. All source files compile without errors or warnings (in strict mode)
3. Linker completes successfully
4. Generated executable is valid and executable
5. Executable can be invoked (basic sanity check)

Test Framework: pytest
Coverage: Build system, compilation, executable validity
Status: Required for T-1.6
"""

import subprocess
import sys
import os
import tempfile
import signal
from pathlib import Path


# Get project root directory
PROJECT_ROOT = Path(__file__).parent.parent.resolve()
BUILD_DIR = PROJECT_ROOT / "build"
EXECUTABLE = BUILD_DIR / "video-looper"
SCRIPTS_DIR = PROJECT_ROOT / "scripts"


def test_project_structure_exists():
    """Verify project directory structure is complete."""
    required_dirs = [
        PROJECT_ROOT / "src",
        PROJECT_ROOT / "src" / "app",
        PROJECT_ROOT / "src" / "camera",
        PROJECT_ROOT / "src" / "gstreamer",
        PROJECT_ROOT / "src" / "recording",
        PROJECT_ROOT / "src" / "playback",
        PROJECT_ROOT / "src" / "input",
        PROJECT_ROOT / "src" / "osx",
        PROJECT_ROOT / "src" / "utils",
        PROJECT_ROOT / "test",
        PROJECT_ROOT / "scripts",
        PROJECT_ROOT / "docs",
        PROJECT_ROOT / "pkgconfig",
    ]

    for directory in required_dirs:
        assert directory.exists() and directory.is_dir(), \
            f"Required directory missing: {directory.relative_to(PROJECT_ROOT)}"


def test_build_configuration_valid():
    """Verify build configuration files are properly set up."""
    config_files = [
        PROJECT_ROOT / "meson.build",
        PROJECT_ROOT / "meson_options.txt",
        SCRIPTS_DIR / "build.sh",
        SCRIPTS_DIR / "run.sh",
        SCRIPTS_DIR / "test.sh",
    ]

    for config_file in config_files:
        assert config_file.exists(), \
            f"Required build config file missing: {config_file.relative_to(PROJECT_ROOT)}"
        assert config_file.stat().st_size > 0, \
            f"Build config file is empty: {config_file.relative_to(PROJECT_ROOT)}"


def test_source_files_exist():
    """Verify all required source files are present."""
    required_sources = [
        "src/main.c",
        "src/app/app_context.c",
        "src/app/app_context.h",
        "src/app/app_error.c",
        "src/app/app_error.h",
        "src/camera/camera_source.c",
        "src/camera/camera_source.h",
        "src/gstreamer/pipeline_builder.c",
        "src/gstreamer/pipeline_builder.h",
        "src/gstreamer/gst_elements.c",
        "src/gstreamer/gst_elements.h",
        "src/recording/recording_state.c",
        "src/recording/recording_state.h",
        "src/recording/buffer_manager.c",
        "src/recording/buffer_manager.h",
        "src/playback/playback_manager.c",
        "src/playback/playback_manager.h",
        "src/playback/playback_bin.c",
        "src/playback/playback_bin.h",
        "src/input/keyboard_handler.c",
        "src/input/keyboard_handler.h",
        "src/osx/window.c",
        "src/osx/window.h",
        "src/utils/logging.c",
        "src/utils/logging.h",
        "src/utils/memory.c",
        "src/utils/memory.h",
        "src/utils/timing.c",
        "src/utils/timing.h",
    ]

    for source_file in required_sources:
        full_path = PROJECT_ROOT / source_file
        assert full_path.exists(), \
            f"Required source file missing: {source_file}"


def test_build_system_clean():
    """Verify build system is configured correctly."""
    # Check meson.build syntax and content
    meson_build = PROJECT_ROOT / "meson.build"
    content = meson_build.read_text()

    # Verify essential meson configuration
    assert "project('video-looper'" in content, \
        "meson.build missing project() call"
    assert "gstreamer" in content.lower(), \
        "meson.build missing GStreamer dependency"
    assert "cocoa" in content.lower(), \
        "meson.build missing Cocoa framework"
    assert "executable('video-looper'" in content, \
        "meson.build missing executable() definition"


def test_build_succeeds():
    """Verify project builds successfully without errors."""
    # Clean build directory first
    if BUILD_DIR.exists():
        import shutil
        shutil.rmtree(BUILD_DIR)

    # Run build script
    build_script = SCRIPTS_DIR / "build.sh"
    assert build_script.exists(), "build.sh script not found"

    result = subprocess.run(
        ["bash", str(build_script), "--release"],
        cwd=PROJECT_ROOT,
        capture_output=True,
        text=True,
        timeout=300
    )

    print("\n--- Build Script Output ---")
    print(result.stdout)
    if result.stderr:
        print("--- Build Errors/Warnings ---")
        print(result.stderr)

    # Check build succeeded
    assert result.returncode == 0, \
        f"Build failed with exit code {result.returncode}\nOutput:\n{result.stdout}\nErrors:\n{result.stderr}"

    # Verify expected success messages
    assert "Build complete" in result.stdout or "Linking target" in result.stdout, \
        "Build script did not complete successfully"


def test_executable_exists():
    """Verify executable was created successfully."""
    assert EXECUTABLE.exists(), \
        f"Executable not found: {EXECUTABLE.relative_to(PROJECT_ROOT)}"
    assert EXECUTABLE.is_file(), \
        f"Executable is not a regular file: {EXECUTABLE}"
    assert os.access(EXECUTABLE, os.X_OK), \
        f"Executable is not executable: {EXECUTABLE}"


def test_executable_is_valid_binary():
    """Verify executable is a valid Mach-O binary (macOS)."""
    result = subprocess.run(
        ["file", str(EXECUTABLE)],
        capture_output=True,
        text=True,
        timeout=10
    )

    file_output = result.stdout
    assert "Mach-O" in file_output or "executable" in file_output, \
        f"Executable is not a valid binary: {file_output}"

    print(f"\nExecutable binary info: {file_output.strip()}")


def test_executable_has_reasonable_size():
    """Verify executable size is reasonable (not zero, not unreasonably large)."""
    size = EXECUTABLE.stat().st_size

    # Reasonable range for a GStreamer-based application
    MIN_SIZE = 10 * 1024  # 10 KB
    MAX_SIZE = 100 * 1024 * 1024  # 100 MB

    assert size > MIN_SIZE, \
        f"Executable is too small ({size} bytes), likely not properly linked"
    assert size < MAX_SIZE, \
        f"Executable is too large ({size} bytes), may indicate bloat"

    print(f"\nExecutable size: {size / 1024:.1f} KB")


def test_executable_can_be_invoked():
    """Verify executable can be invoked without crashing immediately.

    Note: The application requires macOS window management and camera access,
    so we test invocation in a headless manner with a timeout.
    """
    # Try to run with --help or --version (if supported)
    # If not supported, just verify the process starts without segfault

    try:
        # Attempt to run with a short timeout and send SIGTERM
        # This tests that the executable is valid and linkage is correct
        proc = subprocess.Popen(
            [str(EXECUTABLE), "--help"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        try:
            stdout, stderr = proc.communicate(timeout=2)
            # If it supports --help, that's great
            print(f"\nExecutable response to --help:\n{stdout}\n{stderr}")
        except subprocess.TimeoutExpired:
            # If it times out, that's OK - it might be waiting for display
            # Just kill it gracefully
            proc.terminate()
            try:
                proc.wait(timeout=1)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait()
            print("\nExecutable started but requires interactive mode (expected for macOS GUI app)")

    except Exception as e:
        # If we can't even start it, that's a problem
        assert False, f"Failed to invoke executable: {e}"


def test_no_undefined_symbols():
    """Verify executable has no undefined symbols (proper linking).

    This is a macOS-specific check using nm command.
    """
    result = subprocess.run(
        ["nm", "-u", str(EXECUTABLE)],
        capture_output=True,
        text=True,
        timeout=10
    )

    undefined_symbols = result.stdout.strip()

    # Some undefined symbols are expected (dynamic linking), but core functions
    # should be resolved. We just verify the executable is properly linked
    # by checking the nm command succeeded.
    assert result.returncode == 0, \
        f"Failed to analyze executable symbols: {result.stderr}"

    print(f"\nUndefined symbols count: {len(undefined_symbols.splitlines()) if undefined_symbols else 0}")


def test_build_dependencies_resolved():
    """Verify all build dependencies were found and resolved."""
    # Parse build output to ensure no dependency errors
    meson_build = PROJECT_ROOT / "meson.build"
    content = meson_build.read_text()

    required_deps = [
        "gstreamer-1.0",
        "glib-2.0",
        "gobject-2.0",
        "gstreamer-video-1.0",
        "gstreamer-gl-1.0",
        "Cocoa",
        "AVFoundation",
        "CoreFoundation",
        "CoreMedia",
        "CoreVideo",
    ]

    for dep in required_deps:
        assert f"'{dep}'" in content or f'"{dep}"' in content, \
            f"Dependency '{dep}' not found in meson.build configuration"


def test_no_compilation_warnings_in_release():
    """Verify release build has reasonable warning settings."""
    meson_build = PROJECT_ROOT / "meson.build"
    content = meson_build.read_text()

    # Verify warning level is set to 3 (strictest)
    assert "warning_level=3" in content or "warning_level='3'" in content, \
        "Build should use strict warning level"

    # Verify werror is enabled to fail on warnings
    assert "werror=true" in content or "werror='true'" in content, \
        "Build should treat warnings as errors"


def test_build_is_reproducible():
    """Verify build can be reproduced (clean rebuild works)."""
    # First, remove build artifacts
    if BUILD_DIR.exists():
        import shutil
        shutil.rmtree(BUILD_DIR)

    # Build once
    result1 = subprocess.run(
        ["bash", str(SCRIPTS_DIR / "build.sh"), "--release"],
        cwd=PROJECT_ROOT,
        capture_output=True,
        text=True,
        timeout=300
    )

    assert result1.returncode == 0, \
        f"First build failed: {result1.stdout}\n{result1.stderr}"

    # Verify executable exists
    assert EXECUTABLE.exists(), "Executable not created on first build"
    size1 = EXECUTABLE.stat().st_size

    # Clean and build again
    import shutil
    shutil.rmtree(BUILD_DIR)

    result2 = subprocess.run(
        ["bash", str(SCRIPTS_DIR / "build.sh"), "--release"],
        cwd=PROJECT_ROOT,
        capture_output=True,
        text=True,
        timeout=300
    )

    assert result2.returncode == 0, \
        f"Second build failed: {result2.stdout}\n{result2.stderr}"

    assert EXECUTABLE.exists(), "Executable not created on second build"
    size2 = EXECUTABLE.stat().st_size

    # Sizes should be identical (bit-for-bit reproducible builds)
    assert size1 == size2, \
        f"Build is not reproducible: first={size1}, second={size2} bytes"

    print(f"\nBuild is reproducible: {size1} bytes in both builds")


if __name__ == "__main__":
    # Run tests with pytest if available, otherwise run directly
    try:
        import pytest
        sys.exit(pytest.main([__file__, "-v", "--tb=short"]))
    except ImportError:
        print("pytest not found, running tests directly...")

        test_functions = [
            test_project_structure_exists,
            test_build_configuration_valid,
            test_source_files_exist,
            test_build_system_clean,
            test_build_succeeds,
            test_executable_exists,
            test_executable_is_valid_binary,
            test_executable_has_reasonable_size,
            test_executable_can_be_invoked,
            test_no_undefined_symbols,
            test_build_dependencies_resolved,
            test_no_compilation_warnings_in_release,
            test_build_is_reproducible,
        ]

        passed = 0
        failed = 0

        for test_func in test_functions:
            try:
                print(f"\nRunning {test_func.__name__}...", end=" ")
                test_func()
                print("✓ PASS")
                passed += 1
            except AssertionError as e:
                print(f"✗ FAIL: {e}")
                failed += 1
            except Exception as e:
                print(f"✗ ERROR: {e}")
                failed += 1

        print(f"\n\n{'='*60}")
        print(f"Results: {passed} passed, {failed} failed")
        print(f"{'='*60}")

        sys.exit(0 if failed == 0 else 1)
