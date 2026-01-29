# Technical Task List: Video Looper for macOS

**Project**: Video Looper for macOS
**Version**: 1.0
**Date**: January 27, 2026
**Status**: Ready for Implementation

---

## Phase 1: Project Foundation & Build System Setup

**Goal**: Establish project structure, build system, and verify all development tools are functional.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1 | Create project directory structure (src/, test/, docs/, scripts/, build/) | §6 | - |
| T-1.2 | Initialize Meson build system with meson.build files | §6 | - |
| T-1.3 | Configure pkg-config for GStreamer dependency discovery | §5 | - |
| T-1.4 | Create main.c entry point with GStreamer initialization stub | §3.1 | §2 |
| T-1.5 | Set up build scripts (build.sh, run.sh, test.sh) | §6 | - |
| T-1.6 | Configure Cocoa framework linking for macOS integration | §3.8 | - |
| T-1.7 | Test project builds successfully without errors | §6 | - |
| T-1.8 | Test executable runs and exits cleanly with code 0 | §3.1 | - |

---

## Phase 2: Camera Input & Permission Handling

**Goal**: Implement camera source initialization and macOS permission handling.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-2.1 | Create camera_source.h interface for camera initialization | §3.2 | - |
| T-2.2 | Implement camera_source_init() to detect built-in camera via AVFoundation | §3.2 | §4.2 |
| T-2.3 | Implement camera_request_permission() with native macOS permission dialogs | §3.2 | §4.1 |
| T-2.4 | Create camera_permissions.m Objective-C wrapper for AVFoundation APIs | §3.2 | - |
| T-2.5 | Implement camera format negotiation (1920x1080 preferred, 1280x720 fallback) | §3.2 | §4.2 |
| T-2.6 | Create GStreamer avfvideosrc element wrapper | §3.2 | - |
| T-2.7 | Implement error handling for permission denied and camera not found | §3.2, §7 | §4.1 |
| T-2.8 | Test camera initialization without actual hardware (mock camera) | §8.2 | - |
| T-2.9 | Test permission denied error handling with graceful exit | §8.2 | - |
| T-2.10 | Test camera capabilities query and format negotiation | §8.2 | - |

---

## Phase 3: GStreamer Pipeline Builder & Core Infrastructure

**Goal**: Build the main GStreamer pipeline framework and core infrastructure components.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-3.1 | Create pipeline_builder.h interface for pipeline construction | §3.4 | - |
| T-3.2 | Implement pipeline_create() to build main GStreamer pipeline | §3.4 | - |
| T-3.3 | Implement live_queue and live_caps elements for cell 1 live feed | §3.4 | §4.2 |
| T-3.4 | Create videomixer compositor element for 10-cell grid layout | §3.4 | §4.3 |
| T-3.5 | Implement osxvideosink integration for Cocoa window rendering | §3.4, §3.8 | §4.3 |
| T-3.6 | Create GStreamer bus message handler for error handling | §3.4, §7 | - |
| T-3.7 | Implement pipeline_set_state() for state transitions (NULL→READY→PLAYING) | §3.4 | - |
| T-3.8 | Create pipeline_cleanup() for proper resource deallocation | §3.4 | - |
| T-3.9 | Implement logging infrastructure (logging.c, logging.h) | §7.4 | - |
| T-3.10 | Implement memory management utilities (memory.c, memory.h) | §6 | - |
| T-3.11 | Implement high-resolution timing utilities (timing.c, timing.h) | §6 | - |
| T-3.12 | Test pipeline creation and state transitions (NULL→READY→PLAYING→PAUSED→READY) | §8.3 | - |
| T-3.13 | Test GStreamer bus error messages are logged correctly | §8.3 | - |
| T-3.14 | Test pipeline cleanup releases all resources without leaks | §8.3 | - |

---

## Phase 4: macOS Window Management & Rendering

**Goal**: Create native Cocoa window and integrate with GStreamer video rendering.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-4.1 | Create window.h interface for Cocoa window management | §3.8 | - |
| T-4.2 | Implement window_create() to create NSWindow with video view | §3.8 | §4.1 |
| T-4.3 | Implement window sizing: 320px per cell × 10 cells = 3200×height pixels | §3.8 | §4.3 |
| T-4.4 | Implement aspect ratio preservation from camera input | §3.8 | §4.2 |
| T-4.5 | Configure osxvideosink integration with Cocoa NSView | §3.8 | - |
| T-4.6 | Implement window event handler for close/resize events | §3.8 | - |
| T-4.7 | Implement Metal/OpenGL rendering context setup (via osxvideosink) | §3.8 | - |
| T-4.8 | Create window.mm Objective-C++ implementation for native Cocoa APIs | §3.8 | - |
| T-4.9 | Integrate osxvideosink into main application event loop | §3.1, §3.8 | - |
| T-4.10 | Test window creation with correct dimensions and title | §8.4 | - |
| T-4.11 | Test window resize handling and grid layout adaptation | §8.4 | - |
| T-4.12 | Test osxvideosink renders frames to window without corruption | §8.4 | - |

---

## Phase 5: Recording State Management & Keyboard Input

**Goal**: Implement keyboard-driven recording state machine.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-5.1 | Create keyboard_handler.h interface for keyboard input capture | §3.7 | - |
| T-5.2 | Implement keyboard_init() to set up GTK+/native event handlers | §3.7 | §4.4 |
| T-5.3 | Implement keyboard key mapping (1-9 → logical key numbers) | §3.7 | §4.4 |
| T-5.4 | Create recording_state.h interface for recording state machine | §3.3 | - |
| T-5.5 | Implement recording_state_init() to initialize state tracker | §3.3 | - |
| T-5.6 | Implement recording_on_key_press() to start recording | §3.3 | §4.4 |
| T-5.7 | Implement recording_on_key_release() to stop recording | §3.3 | §4.4 |
| T-5.8 | Implement recording state query functions (is_recording, get_duration) | §3.3 | - |
| T-5.9 | Implement circular cell assignment logic (cell 2 → cell 3 → ... → cell 10 → cell 2) | §3.3 | §4.5 |
| T-5.10 | Implement keyboard callback dispatch to recording state manager | §3.7 | - |
| T-5.11 | Create key_codes.h with key code mappings and constants | §3.7 | - |
| T-5.12 | Test keyboard press/release events are processed correctly | §8.2 | - |
| T-5.13 | Test multiple simultaneous key presses are handled independently | §8.2 | §4.4 |
| T-5.14 | Test recording state transitions (idle → recording → stopped) | §8.2 | - |
| T-5.15 | Test keyboard input latency < 50ms from press to state change | §8.2 | §5.1 |

---

## Phase 6: GPU Memory Buffer Management for Recording

**Goal**: Implement ring buffer storage for recorded video frames in GPU memory.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-6.1 | Create buffer_manager.h interface for ring buffer operations | §3.5 | - |
| T-6.2 | Implement buffer_create() to allocate GPU memory for ring buffer | §3.5 | - |
| T-6.3 | Implement buffer_write_frame() to store GStreamer buffers in ring | §3.5 | §4.5 |
| T-6.4 | Implement ring buffer wraparound logic (write_pos, read_pos management) | §3.5 | - |
| T-6.5 | Implement buffer_read_frame() to retrieve frames for playback | §3.5 | - |
| T-6.6 | Implement buffer_get_frame_count() and buffer_get_duration() queries | §3.5 | - |
| T-6.7 | Implement memory exhaustion handling (discard oldest frame on overflow) | §3.5 | §7 |
| T-6.8 | Implement buffer_cleanup() for proper GPU memory release | §3.5 | - |
| T-6.9 | Implement frame format validation (caps checking) | §3.5 | - |
| T-6.10 | Test ring buffer allocation and frame write/read operations | §8.2 | - |
| T-6.11 | Test ring buffer wraparound at capacity boundary | §8.2 | - |
| T-6.12 | Test buffer exhaustion handling (oldest frame discarded) | §8.2 | - |
| T-6.13 | Test GPU memory is not transferred to CPU (GPU-only storage) | §8.2 | §4.6 |

---

## Phase 7: Palindrome Playback Logic Implementation

**Goal**: Implement palindrome looping algorithm for recorded video playback.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-7.1 | Create playback_manager.h interface for palindrome playback | §3.6 | - |
| T-7.2 | Implement PlaybackLoop state structure (current_frame, direction, total_frames) | §3.6 | - |
| T-7.3 | Implement playback_loop_create() to initialize playback from recorded buffer | §3.6 | - |
| T-7.4 | Implement playback_advance_frame() to step through palindrome sequence | §3.6 | - |
| T-7.5 | Implement forward direction logic (frame 0→1→2→...→N-1) | §3.6 | §4.5 |
| T-7.6 | Implement reverse direction logic (frame N-1→N-2→...→1→0) | §3.6 | §4.5 |
| T-7.7 | Implement direction state transitions at boundaries | §3.6 | - |
| T-7.8 | Implement playback_get_next_frame() to retrieve next frame for rendering | §3.6 | - |
| T-7.9 | Implement playback_loop_cleanup() for resource release | §3.6 | - |
| T-7.10 | Test palindrome sequence matches expected pattern (0,1,2,...,N-1,N-2,...,1,0) | §8.2 | - |
| T-7.11 | Test direction changes at forward and reverse boundaries | §8.2 | - |
| T-7.12 | Test playback loop continues correctly after multiple cycles | §8.2 | - |
| T-7.13 | Test playback with 1 frame, 10 frames, and 60-frame buffers | §8.2 | - |

---

## Phase 8: Recording Bin Creation & Management

**Goal**: Create GStreamer bins for recording video into GPU buffers.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-8.1 | Create gst_elements.h interface for GStreamer element helpers | §3.4 | - |
| T-8.2 | Implement record bin creation with queue, capsfilter, and fakesink | §3.4 | - |
| T-8.3 | Implement pipeline_add_record_bin() to add recording bin to pipeline | §3.4 | §4.5 |
| T-8.4 | Implement pipeline_remove_record_bin() to remove inactive recording bin | §3.4 | - |
| T-8.5 | Implement recording signal handling to start/stop GStreamer bin recording | §3.3, §3.4 | - |
| T-8.6 | Implement queue management to prevent deadlocks in recording | §3.4 | - |
| T-8.7 | Implement caps negotiation between live stream and record bins | §3.4 | - |
| T-8.8 | Test record bin creation with valid GStreamer elements | §8.3 | - |
| T-8.9 | Test record bin add/remove during pipeline playback | §8.3 | - |
| T-8.10 | Test frames flow through record bin to GPU buffer without corruption | §8.3 | - |

---

## Phase 9: Playback Bin Creation & Integration

**Goal**: Create and integrate GStreamer bins for palindrome playback into compositor.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-9.1 | Create playback_bin.h interface for playback bin creation | §3.6 | - |
| T-9.2 | Implement playback_create_bin() to create GStreamer bin from PlaybackLoop | §3.6 | - |
| T-9.3 | Implement playback source (appsrc) to emit frames from PlaybackLoop | §3.6 | - |
| T-9.4 | Implement playback bin linking to videomixer for grid cell output | §3.6 | - |
| T-9.5 | Implement dynamic playback bin lifecycle management (create on recording end) | §3.4, §3.6 | - |
| T-9.6 | Implement pipeline_add_playback_bin() to add bin to running pipeline | §3.4 | - |
| T-9.7 | Implement pipeline_remove_playback_bin() to remove playback bin | §3.4 | - |
| T-9.8 | Implement videomixer pad properties (zorder, position, size) | §3.4 | - |
| T-9.9 | Test playback bin creation and frame emission from PlaybackLoop | §8.3 | - |
| T-9.10 | Test playback bin integrates with videomixer without deadlock | §8.3 | - |
| T-9.11 | Test dynamic add/remove of playback bins during pipeline playback | §8.3 | - |

---

## Phase 10: Live Feed Integration & Grid Composition

**Goal**: Complete live feed routing and videomixer grid composition setup.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-10.1 | Implement live_queue element in pipeline for cell 1 live feed | §3.4 | §4.2 |
| T-10.2 | Implement tee element to split live stream to recording bins and videomixer | §3.4 | - |
| T-10.3 | Implement videomixer configuration for 10-cell grid layout | §3.4 | §4.3 |
| T-10.4 | Implement videomixer pad ordering (zorder 0-9 for cells 1-10) | §3.4 | - |
| T-10.5 | Implement composite_caps element for format conversion before osxvideosink | §3.4 | - |
| T-10.6 | Implement grid cell positioning (320px width × 10, height from aspect ratio) | §3.4 | §4.3 |
| T-10.7 | Implement aspect ratio preservation in videomixer pads | §3.4 | §4.2 |
| T-10.8 | Test live feed appears in cell 1 and updates at 120fps | §8.3 | §4.2 |
| T-10.9 | Test grid composition produces correct 10-cell layout | §8.3 | §4.3 |
| T-10.10 | Test recording bins receive live stream without affecting cell 1 display | §8.3 | - |
| T-10.11 | Test videomixer properly blends all 10 cells into single output | §8.3 | - |

---

## Phase 11: Main Application Integration & Event Loop

**Goal**: Integrate all components into main application with GTK+ event loop.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-11.1 | Create app_context.h structure for application state management | §3.1, §4.3 | - |
| T-11.2 | Create app_error.h error codes and handling framework | §7 | - |
| T-11.3 | Implement main() entry point with GStreamer initialization | §3.1 | §2 |
| T-11.4 | Implement camera initialization and permission request in main() | §3.1, §3.2 | §4.1 |
| T-11.5 | Implement window creation in main() | §3.1, §3.8 | §4.1 |
| T-11.6 | Implement GStreamer pipeline creation in main() | §3.1, §3.4 | - |
| T-11.7 | Implement keyboard handler initialization in main() | §3.1, §3.7 | - |
| T-11.8 | Implement GTK+ main event loop with g_main_loop_run() | §3.1 | - |
| T-11.9 | Implement graceful shutdown on Escape/Ctrl+C | §3.7 | - |
| T-11.10 | Implement cleanup on exit (pipeline, window, camera resources) | §3.1 | - |
| T-11.11 | Implement atexit handlers for emergency cleanup | §3.1 | - |
| T-11.12 | Test application launches and shows live feed within 2 seconds | §8.4 | §5.1 |
| T-11.13 | Test application responds to keyboard input while running | §8.4 | §5.1 |
| T-11.14 | Test application exits cleanly on Escape/Ctrl+C | §8.4 | - |
| T-11.15 | Test all resources are freed on exit (no memory leaks) | §8.4 | - |

---

## Phase 12: Recording to Playback Flow Integration

**Goal**: Complete end-to-end flow from key press to palindrome playback in grid.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-12.1 | Implement coordinate signal flow: keyboard → recording_state → pipeline | §3.3, §3.4 | - |
| T-12.2 | Implement recording start signal from recording_state to record bin | §3.3, §3.4 | - |
| T-12.3 | Implement recording stop signal and buffer management | §3.3, §3.5 | - |
| T-12.4 | Implement recording completion detection in buffer_manager | §3.5, §3.6 | - |
| T-12.5 | Implement playback bin creation trigger on recording completion | §3.6, §3.4 | - |
| T-12.6 | Implement playback bin integration into videomixer immediately | §3.6, §3.4 | §4.5 |
| T-12.7 | Implement cell assignment logic (key 1→cell 2, key 2→cell 3, etc.) | §3.3 | §4.4 |
| T-12.8 | Implement circular cell management (wrap around after cell 10 to cell 2) | §3.3 | §4.5 |
| T-12.9 | Implement visual feedback for recording state (border flash or indicator) | §3.3, §3.8 | §4.4 |
| T-12.10 | Test record key 1 → capture frames → start playback in cell 2 | §8.3 | - |
| T-12.11 | Test record key 2 → capture frames → start playback in cell 3 | §8.3 | - |
| T-12.12 | Test multiple simultaneous recordings (keys 1,2,3 pressed together) | §8.3 | - |
| T-12.13 | Test palindrome playback plays forward then backward correctly in grid | §8.3 | §4.5 |
| T-12.14 | Test cell wraparound (record 9 videos fills cells 2-10, 10th video wraps to cell 2) | §8.3 | §4.5 |

---

## Phase 13: Performance Optimization & 120 FPS Target

**Goal**: Optimize pipeline for sustained 120 fps playback across all 10 cells.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-13.1 | Profile pipeline with GStreamer tools (gst-tracepoints, perf) | §5, §8 | - |
| T-13.2 | Measure and optimize queue buffer sizes in pipeline | §3.4 | - |
| T-13.3 | Optimize videomixer latency settings (latency=0) | §3.4 | - |
| T-13.4 | Optimize osxvideosink synchronization (sync=true) for 120fps | §3.4, §3.8 | - |
| T-13.5 | Implement frame drop detection and logging | §7.4 | - |
| T-13.6 | Benchmark GPU memory bandwidth for 9 simultaneous recordings | §3.5 | - |
| T-13.7 | Measure CPU utilization for video processing (target < 5%) | §5.1 | - |
| T-13.8 | Profile memory usage with multiple video loops | §5.1 | - |
| T-13.9 | Optimize GStreamer CPU thread affinity if needed | §3.4 | - |
| T-13.10 | Test sustained 120 fps playback for 30-minute session | §8.4 | §5.1 |
| T-13.11 | Test CPU utilization stays below 5% during playback | §8.4 | §5.1 |
| T-13.12 | Test memory growth stays below 10% over 1-hour session | §8.4 | §5.2 |
| T-13.13 | Test GPU memory remains stable (no leaks) over extended session | §8.4 | §5.2 |

---

## Phase 14: Error Handling & Edge Cases

**Goal**: Implement comprehensive error handling and edge case management.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-14.1 | Implement camera not found error handling with user dialog | §3.2, §7 | §4.1 |
| T-14.2 | Implement camera permission denied error handling | §3.2, §7 | §4.1 |
| T-14.3 | Implement camera disconnection detection during session | §3.2, §7 | §5.2 |
| T-14.4 | Implement camera reconnection recovery logic | §3.2, §7 | §5.2 |
| T-14.5 | Implement GStreamer bus error handling with recovery attempts | §3.4, §7 | - |
| T-14.6 | Implement pipeline state change failure recovery | §3.4, §7 | - |
| T-14.7 | Implement recording buffer overflow handling (discard oldest frame) | §3.5, §7 | - |
| T-14.8 | Implement graceful shutdown on fatal errors | §7 | - |
| T-14.9 | Implement keyboard handler failure recovery | §3.7, §7 | - |
| T-14.10 | Implement window creation failure handling | §3.8, §7 | - |
| T-14.11 | Test camera permission denied shows error dialog and exits cleanly | §8.4 | - |
| T-14.12 | Test camera not found shows error message and exits | §8.4 | - |
| T-14.13 | Test brief camera disconnect is handled gracefully | §8.4 | - |
| T-14.14 | Test recording with very short key press (< 33ms) still captures at least 1 frame | §8.2 | - |
| T-14.15 | Test multiple recordings exceed 9 cells (wraparound to cell 2) | §8.2 | - |

---

## Phase 15: Comprehensive Testing & QA

**Goal**: Execute full test suite covering unit, integration, and E2E scenarios.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-15.1 | Run all unit tests (25 tests across 4 test files) | §8.2 | - |
| T-15.2 | Run integration tests for GStreamer pipeline | §8.3 | - |
| T-15.3 | Run integration tests for recording→playback flow | §8.3 | - |
| T-15.4 | Run integration tests for 120fps frame rate measurement | §8.3 | - |
| T-15.5 | Run E2E test for application launch time (< 2 seconds) | §8.4 | §5.1 |
| T-15.6 | Run E2E test for camera permission handling | §8.4 | - |
| T-15.7 | Run E2E test for grid display and cell layout | §8.4 | - |
| T-15.8 | Verify code coverage meets targets (unit: 85%, integration: 60%) | §8 | - |
| T-15.9 | Run memory leak detection with valgrind (if available on macOS) | §8.4 | - |
| T-15.10 | Run clang-format for code style consistency | §6 | - |
| T-15.11 | Test application with 10 simultaneous cells playing at 120fps | §8.4 | §4.3 |
| T-15.12 | Test application stability over 30-minute continuous session | §8.4 | §5.2 |
| T-15.13 | Test keyboard input responsiveness (latency < 50ms) | §8.4 | §5.1 |
| T-15.14 | Test live feed persists during all recording and playback operations | §8.4 | §4.8 |
| T-15.15 | Verify no CPU video processing (GPU-only requirement met) | §8.4 | §4.6 |

---

## Phase 16: Documentation & Release

**Goal**: Complete all documentation and prepare for release.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-16.1 | Create ARCHITECTURE.md documenting system design and rationale | §1, §2 | - |
| T-16.2 | Create GSTREAMER_PIPELINE.md with detailed pipeline configuration | §1, §2 | - |
| T-16.3 | Create comprehensive README.md with build and usage instructions | §6 | - |
| T-16.4 | Create CONTRIBUTING.md with development guidelines | §6 | - |
| T-16.5 | Create API documentation for all public interfaces (Doxygen comments) | §3 | - |
| T-16.6 | Document key mapping and keyboard controls | §3.7 | §4.4 |
| T-16.7 | Document error codes and error handling procedures | §7 | - |
| T-16.8 | Create troubleshooting guide for common issues | §7 | - |
| T-16.9 | Document performance tuning and optimization options | §5 | - |
| T-16.10 | Create changelog documenting version 1.0 release | §6 | - |
| T-16.11 | Review all documentation for accuracy and completeness | §6 | - |
| T-16.12 | Verify README instructions work for fresh build setup | §6 | - |
| T-16.13 | Test build and run from clean environment | §6 | - |
| T-16.14 | Create final release artifact with compiled binary | §6 | - |

---

## Summary of Deliverables by Phase

| Phase | Focus | Key Deliverable |
|-------|-------|-----------------|
| **1** | Foundation | Project structure, build system functional |
| **2** | Camera | Camera initialization and permission handling |
| **3** | Pipeline | Core GStreamer pipeline framework |
| **4** | Window | Native Cocoa window and rendering |
| **5** | Input | Keyboard-driven recording state machine |
| **6** | GPU Buffers | Ring buffer video storage on GPU |
| **7** | Playback | Palindrome looping algorithm |
| **8** | Recording Bins | GStreamer recording pipeline elements |
| **9** | Playback Bins | GStreamer playback pipeline elements |
| **10** | Composition | Live feed + grid composition in videomixer |
| **11** | Integration | Main event loop and component coordination |
| **12** | E2E Flow | Complete record→playback workflow |
| **13** | Performance | 120 fps optimization and profiling |
| **14** | Robustness | Error handling and edge cases |
| **15** | Quality | Comprehensive testing suite execution |
| **16** | Release | Documentation and final packaging |

---

## Implementation Notes

### MVP-First Strategy
The TTL is structured for MVP-first delivery:
- **Phase 1-11**: Core MVP (single record/playback loop, live feed, 10-cell grid)
- **Phase 12-14**: Feature completeness (multiple loops, error handling, edge cases)
- **Phase 15-16**: Production quality (testing, documentation, release)

### Task Dependencies
- Recording requires: Camera, Pipeline, Window, Keyboard (Phases 2-5)
- Playback requires: Buffer Management, Playback Logic (Phases 6-7)
- Grid requires: Recording Bins, Playback Bins, Live Feed Integration (Phases 8-10)
- Full app requires: Main integration and Event Loop (Phase 11)

### Testing Strategy
- **Unit tests**: Recording state, buffer management, palindrome logic, keyboard
- **Integration tests**: Pipeline building, recording→playback flow, FPS measurement
- **E2E tests**: Application launch, permission handling, grid display

### Performance Targets
- **120 fps**: Sustained playback across all 10 cells
- **< 50ms latency**: Keyboard input responsiveness
- **< 5% CPU**: Video processing on GPU only
- **< 10% memory growth**: Over 1-hour session

---

**Document Version**: 1.0
**Last Updated**: January 27, 2026
**Total Tasks**: 116
**Total Test Tasks**: 46
**Estimated Phases**: 16
