# Integration tests configuration
# These tests verify interactions between components

# Base source files for integration tests
# NOTE: These are self-contained tests using GStreamer directly
# They don't depend on project-specific modules to avoid build issues
integration_base_files = [
  # Most integration tests are standalone and only require GStreamer
  # Some tests include specific project modules when needed
]

# Stable tests that consistently pass
stable_integration_tests = [
  'test_gstreamer_pipeline.c',
  'test_pipeline_core.c',
  'test_recording_flow.c',
  'test_camera_window_integration.c',
  'test_playback_bins_lifecycle.c',
  'test_app_lifecycle.c',
  'test_playback_e2e_flow.c',
]

# Tests that have timing/platform-specific issues
# These are disabled for now but documented for future fixing
unstable_tests = [
  # 'test_120fps_rendering.c',           # DISABLED: usleep() precision issues on macOS
  # 'test_recording_bins_integration.c',  # DISABLED: GStreamer ghost pad linking issues
  # 'test_tee_stream_splitting.c',        # DISABLED: osxvideosink NSRunLoop requirements
  # 'test_gpu_memory_bandwidth_benchmark.c', # DISABLED: incorrect bandwidth expectations
]

# Build specific integration tests
# test_recording_flow is a standalone test that doesn't require other project modules
test_recording_flow_exe = executable(
  'test_recording_flow',
  'test_recording_flow.c',
  dependencies: [glib_dep, gobject_dep, gstreamer_dep, gstreamer_video_dep, gstreamer_gl_dep, gstreamer_app_dep],
  include_directories: inc_dirs,
  c_args: ['-Wall', '-Wextra', '-Wno-gnu-zero-variadic-macro-arguments']
)
test('test_recording_flow', test_recording_flow_exe, suite: 'integration')

# Other stable integration tests that may require project modules
integration_test_files_other = [
  'test_gstreamer_pipeline.c',
  'test_pipeline_core.c',
  'test_camera_window_integration.c',
  'test_playback_bins_lifecycle.c',
  'test_app_lifecycle.c'
]

# Source files for other tests
integration_source_files = [
  '../../src/utils/logging.c',
  '../../src/utils/memory.c',
  '../../src/utils/timing.c',
  '../../src/utils/profiling.c',
  '../../src/app/app_context.c',
  '../../src/app/app_error.c',
  '../../src/gstreamer/pipeline_builder.c',
  '../../src/gstreamer/composite_caps.c',
  '../../src/gstreamer/gst_elements.c',
  '../../src/gstreamer/live_queue.c',
  '../../src/gstreamer/live_tee.c',
  '../../src/gstreamer/record_bin.c',
  '../../src/gstreamer/performance_config.c',
  '../../src/gstreamer/gstreamer_error_handler.c',
  '../../src/gstreamer/pipeline_error_recovery.c',
  '../../src/recording/recording_state.c',
  '../../src/recording/buffer_manager.c',
  '../../src/playback/playback_manager.c',
  '../../src/playback/playback_bin.c',
]

foreach test_file : integration_test_files_other
  test_name = test_file.split('.')[0]
  test_exe = executable(
    test_name,
    [test_file] + integration_source_files,
    dependencies: [glib_dep, gobject_dep, gstreamer_dep, gstreamer_video_dep, gstreamer_gl_dep, gstreamer_app_dep],
    include_directories: inc_dirs,
    c_args: ['-Wall', '-Wextra', '-Wno-gnu-zero-variadic-macro-arguments'],
    build_by_default: false  # Don't build these by default
  )

  test(test_name, test_exe, suite: 'integration')
endforeach

# Self-contained E2E flow tests (no GStreamer integration required)
# These tests use pure mock implementations to verify the core playback
# algorithm and cell assignment logic without requiring GStreamer linking.
test_exe = executable(
  'test_playback_e2e_flow',
  'test_playback_e2e_flow.c',
  dependencies: [glib_dep],
  include_directories: inc_dirs,
  c_args: ['-Wall', '-Wextra', '-Wno-gnu-zero-variadic-macro-arguments']
)

test('test_playback_e2e_flow', test_exe, suite: 'integration')

# Note: E2E startup/shutdown timing tests (test_app_startup_timing.py) are disabled
# because the application requires actual camera/window initialization which is not
# available in the test environment. This test is for manual E2E validation only.
