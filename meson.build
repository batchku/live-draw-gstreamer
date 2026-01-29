project('video-looper', 'c', 'objc',
  version: '1.0.0',
  license: 'MIT',
  meson_version: '>= 1.0',
  default_options: [
    'buildtype=release',
    'warning_level=3',
    'werror=true',
    'c_std=c11'  # Changed from c99 to c11 for GStreamer 1.20+ compatibility
  ]
)

# Compiler and build configuration
cc = meson.get_compiler('c')
objc_compiler = meson.get_compiler('objc')

# ============================================================================
# GStreamer Dependencies via pkg-config
# ============================================================================
# GStreamer is the primary video processing framework. It provides:
# - gstreamer-1.0: Core pipeline and element management
# - glib-2.0 & gobject-2.0: GStreamer's object system dependencies
# - gstreamer-video-1.0: Video-specific elements and capabilities
# - gstreamer-gl-1.0: GPU acceleration support (optional but required for 120fps target)

gstreamer_dep = dependency('gstreamer-1.0', version: '>= 1.20',
  fallback: [],
  required: true
)
glib_dep = dependency('glib-2.0', version: '>= 2.70',
  fallback: [],
  required: true
)
gobject_dep = dependency('gobject-2.0',
  fallback: [],
  required: true
)
gstreamer_video_dep = dependency('gstreamer-video-1.0',
  fallback: [],
  required: true
)

# GPU acceleration (Metal/OpenGL) - required for 120fps target on macOS
gstreamer_gl_dep = dependency('gstreamer-gl-1.0', version: '>= 1.20',
  fallback: [],
  required: true,
  kwargs: {'method': 'pkg-config'}
)

# GStreamer app library for appsrc/appsink elements
# Required for playback bin frame emission via appsrc
gstreamer_app_dep = dependency('gstreamer-app-1.0', version: '>= 1.20',
  fallback: [],
  required: true
)

# ============================================================================
# macOS Framework Dependencies via pkg-config
# ============================================================================
# Cocoa: Native macOS window management and UI integration
# AVFoundation: Camera hardware access and permission handling
# CoreFoundation: Fundamental data structures and utilities
# CoreMedia: Low-level media data handling
# CoreVideo: Video frame management and GPU memory
#
# Strategy:
# 1. Ensure PKG_CONFIG_PATH includes $PROJECT_ROOT/pkgconfig
# 2. Use custom .pc files for framework definitions
# 3. Require all frameworks on macOS systems (fail-fast on missing deps)
#
# Configuration:
# The build.sh script automatically sets PKG_CONFIG_PATH to include
# the project's pkgconfig/ directory which contains .pc files for
# all required frameworks.

# Verify we're on macOS
if host_machine.system() != 'darwin'
  error('Video Looper requires macOS/Darwin. Current system: @0@'.format(host_machine.system()))
endif

# macOS Frameworks - use native declare_dependency for frameworks
# These are built-in macOS frameworks not available via pkg-config

# Cocoa framework for native macOS window management
# Provides: NSWindow, NSView, Metal/OpenGL rendering integration
cocoa_dep = declare_dependency(
  link_args: ['-framework', 'Cocoa']
)

# AVFoundation framework for camera access and media capture
# Provides: AVCaptureDevice, AVCaptureSession, camera permission handling
avfoundation_dep = declare_dependency(
  link_args: ['-framework', 'AVFoundation']
)

# CoreFoundation is a dependency of AVFoundation
# Provides: CFArray, CFDictionary, memory management utilities
core_foundation_dep = declare_dependency(
  link_args: ['-framework', 'CoreFoundation']
)

# CoreMedia for efficient media data handling
# Required by AVFoundation for frame operations
core_media_dep = declare_dependency(
  link_args: ['-framework', 'CoreMedia']
)

# CoreVideo for GPU-accelerated video frame management
# Critical for keeping video frames on GPU throughout pipeline
core_video_dep = declare_dependency(
  link_args: ['-framework', 'CoreVideo']
)

# Include directories
inc_dirs = include_directories(
  'src',
  'src/app',
  'src/camera',
  'src/gstreamer',
  'src/recording',
  'src/playback',
  'src/input',
  'src/osx',
  'src/utils'
)

# Source files - organized by component
source_files = [
  # Utilities
  'src/utils/logging.c',
  'src/utils/memory.c',
  'src/utils/timing.c',
  'src/utils/profiling.c',

  # Application core
  'src/app/app_context.c',
  'src/app/app_error.c',
  'src/app/error_dialog.m',
  'src/app/cleanup_handlers.c',
  'src/app/e2e_coordinator.c',

  # Camera subsystem
  'src/camera/camera_source.c',
  'src/camera/camera_permissions.m',

  # GStreamer pipeline
  'src/gstreamer/pipeline_builder.c',
  'src/gstreamer/composite_caps.c',
  'src/gstreamer/gst_elements.c',
  'src/gstreamer/live_queue.c',
  'src/gstreamer/live_tee.c',
  'src/gstreamer/record_bin.c',
  'src/gstreamer/performance_config.c',
  'src/gstreamer/gstreamer_error_handler.c',
  'src/gstreamer/pipeline_error_recovery.c',

  # Recording subsystem
  'src/recording/recording_state.c',
  'src/recording/buffer_manager.c',

  # Playback subsystem
  'src/playback/playback_manager.c',
  'src/playback/playback_bin.c',

  # Input handling
  'src/input/keyboard_handler.c',

  # macOS window management
  'src/osx/window.c',
  'src/osx/window.m',

  # Main entry point
  'src/main.c'
]

# Compiler flags
c_flags = [
  '-Wall',
  '-Wextra',
  '-Wpedantic',
  '-fPIC',
  '-Wno-gnu-zero-variadic-macro-arguments'  # Allow variadic macros (used by GStreamer logging)
]

# Platform-specific compiler flags
if host_machine.system() == 'darwin'
  c_flags += [
    '-fno-common',
    '-fobjc-arc'  # Automatic Reference Counting for Objective-C
  ]
endif

# Build the main executable
# Collect all dependencies for the executable
all_dependencies = [
  gstreamer_dep,
  glib_dep,
  gobject_dep,
  gstreamer_video_dep,
  gstreamer_gl_dep,
  gstreamer_app_dep,
  cocoa_dep,
  avfoundation_dep,
  core_foundation_dep,
  core_media_dep,
  core_video_dep
]

# Build the main executable
# All framework dependencies are resolved via pkg-config
# Embed Info.plist for macOS camera permission (NSCameraUsageDescription)
info_plist_path = meson.current_source_dir() / 'Info.plist'

video_looper_exe = executable('video-looper',
  source_files,
  dependencies: all_dependencies,
  include_directories: inc_dirs,
  c_args: c_flags,
  install: true,
  win_subsystem: 'windows',  # GUI application (replaces deprecated gui_app)
  objc_args: [
    '-fno-common',
    '-Wno-gnu-zero-variadic-macro-arguments'  # Allow variadic macros in Objective-C
  ],
  link_args: [
    '-sectcreate', '__TEXT', '__info_plist', info_plist_path
  ]
)

# Subdirectory for tests
subdir('test')

# Summary
summary = '''
Video Looper for macOS - Build Summary
========================================
Project: video-looper
Version: @0@
Build type: @1@
C Compiler: @2@
Dependencies:
  - GStreamer: @3@
  - GLib: @4@
  - Cocoa: @5@
'''.format(
  meson.project_version(),
  get_option('buildtype'),
  cc.get_id(),
  gstreamer_dep.version(),
  glib_dep.version(),
  'system library'
)

message(summary)
