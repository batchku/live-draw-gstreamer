# Technical Task List: Video Looper for macOS

**Project**: Video Looper for macOS
**Version**: 1.0 (Right-Sized)
**Date**: January 27, 2026
**Status**: Ready for Implementation

---

## Size Assessment

| Dimension | Score | Evidence |
|-----------|-------|----------|
| Product surface | 6 | Multiple flows (camera→record, recording→playback, playback→display, keyboard→state) but focused scope |
| Artifacts/platforms | 5 | Single macOS application, one monolithic artifact with GStreamer pipeline |
| Data model | 3 | Simple: recording buffers, playback state, grid cells; no persistence |
| Integrations | 5 | 3 critical: AVFoundation (camera), GStreamer (pipeline), Cocoa (window) |
| Security/compliance | 2 | Basic camera permission handling only; no auth or user data |
| Ops/reliability | 3 | Local application; uptime/stability important but no multi-env monitoring |
| Performance/scale | 7 | **Strict**: 120 fps sustained, <50ms keyboard latency, <5% CPU, GPU-only, 3.4GB GPU memory constraint |
| UX/design complexity | 2 | Keyboard-driven, minimal UI; simple 10-cell grid layout |
| Migration/legacy impact | 0 | Pure greenfield application |
| Uncertainty buffer | 6 | GStreamer GPU on macOS (platform-specific), 120 fps consistency, palindrome complexity, permissions model |
| **TOTAL** | **39** | **Medium (35–46 band)** |

**Band**: Medium
**Target Phases**: 7–12
**Target Tasks**: 60–130
**Current State**: 16 phases, 116 tasks → **Over-phased, needs consolidation to 9-10 phases**

---

## Phase 1: Project Foundation & Build System

**Goal**: Establish project structure, build system, and development environment.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1 | Create project structure (src/, test/, docs/, scripts/, build/) and initialize Meson build system | §6 | - |
| T-1.2 | Configure pkg-config for GStreamer, Cocoa, and AVFoundation dependencies | §5 | - |
| T-1.3 | Create main.c entry point stub with GStreamer initialization and event loop skeleton | §3.1 | §2 |
| T-1.4 | Set up build scripts (build.sh, run.sh, test.sh) and verify clean compilation | §6 | - |
| T-1.5 | Create logging, memory, and timing utility modules (src/utils/) | §7.4, §6 | - |
| T-1.6 | Test project builds successfully and executable runs without errors | §6 | - |

---

## Phase 2: Camera Input & macOS Window Integration

**Goal**: Implement camera source initialization, permission handling, and native Cocoa window rendering.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-2.1 | Create camera_source module with AVFoundation integration for camera detection and permission request | §3.2 | §4.1, §4.2 |
| T-2.2 | Implement camera format negotiation (1920x1080 preferred, 1280x720 fallback) with error handling | §3.2 | §4.2 |
| T-2.3 | Create window module with NSWindow creation, sizing (3200×180 for 10×1 grid), and Cocoa integration | §3.8 | §4.1, §4.3 |
| T-2.4 | Implement osxvideosink integration with Cocoa NSView for Metal/OpenGL rendering | §3.8 | §4.3 |
| T-2.5 | Test camera initialization (with mock camera), permission dialogs, and window creation/sizing | §8.2, §8.4 | - |

---

## Phase 3: GStreamer Pipeline Core & Live Feed

**Goal**: Build main GStreamer pipeline with camera source, live queue, and videomixer composition.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-3.1 | Create pipeline_builder module with main GStreamer pipeline construction (camera source, live queue, tee) | §3.4 | - |
| T-3.2 | Implement videomixer compositor for 10-cell grid layout with proper pad configuration (zorder, positioning) | §3.4 | §4.3 |
| T-3.3 | Implement composite_caps element for format conversion and osxvideosink linkage | §3.4 | - |
| T-3.4 | Implement GStreamer bus message handler for error/warning/info handling and logging | §3.4, §7 | - |
| T-3.5 | Implement pipeline state transitions (NULL→READY→PLAYING) and lifecycle management | §3.4 | - |
| T-3.6 | Test pipeline creation, state transitions, bus message handling, and cleanup for resource leaks | §8.3 | - |

---

## Phase 4: Recording & Playback Infrastructure

**Goal**: Implement recording state machine, buffer management, and palindrome playback logic.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-4.1 | Create keyboard_handler module with key capture and callback dispatch (keys 1-9, Escape) | §3.7 | §4.4 |
| T-4.2 | Create recording_state module with key press/release handling and circular cell assignment logic | §3.3 | §4.4, §4.5 |
| T-4.3 | Create buffer_manager module with GPU ring buffer allocation, write/read, and memory exhaustion handling | §3.5 | §4.5, §4.6 |
| T-4.4 | Create playback_manager module implementing palindrome playback algorithm (forward/reverse/cycle) | §3.6 | §4.5 |
| T-4.5 | Test keyboard input, recording state transitions, buffer operations, and palindrome sequences | §8.2 | - |

---

## Phase 5: Recording Bins & Live Stream Routing

**Goal**: Create GStreamer recording bins and integrate live feed tee/queue into pipeline.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-5.1 | Create gst_elements helper module for building record/playback bins with queue, capsfilter, fakesink | §3.4 | - |
| T-5.2 | Implement record bin creation and integration into pipeline with signal-based start/stop control | §3.4 | §4.5 |
| T-5.3 | Implement live_queue element for cell 1 live feed with proper caps negotiation | §3.4 | §4.2 |
| T-5.4 | Implement tee element to split live stream to recording bins and videomixer without deadlock | §3.4 | - |
| T-5.5 | Implement pipeline_add_record_bin() and pipeline_remove_record_bin() for dynamic bin management | §3.4 | - |
| T-5.6 | Test record bin creation/removal, live feed routing, frame flow through recording pipeline, and grid composition | §8.3 | - |

---

## Phase 6: Playback Bins & Full E2E Recording→Playback Flow

**Goal**: Create playback bins from recorded buffers and integrate complete record→capture→playback workflow.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-6.1 | Create playback_bin module with appsrc element for emitting frames from PlaybackLoop | §3.6 | - |
| T-6.2 | Implement playback_create_bin() to build and link playback bin to videomixer with proper pad properties | §3.6 | §4.5 |
| T-6.3 | Implement pipeline_add_playback_bin() and pipeline_remove_playback_bin() for dynamic playback management | §3.4 | - |
| T-6.4 | Implement end-to-end flow: keyboard → recording_state → record_bin → buffer → playback_bin → videomixer → display | §3.3, §3.4, §3.5, §3.6 | §4.5 |
| T-6.5 | Implement cell assignment and circular wraparound (key 1→cell 2, key 9→cell 10, next record→cell 2) | §3.3 | §4.5 |
| T-6.6 | Test record key 1→playback in cell 2, multiple simultaneous recordings, cell wraparound, and palindrome playback in grid | §8.3 | - |

---

## Phase 7: Main Application Integration & Event Loop

**Goal**: Integrate all components into main application with error handling and startup/shutdown management.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-7.1 | Create app_context and app_error modules for application state and error code framework | §3.1, §7 | - |
| T-7.2 | Implement main() entry point orchestrating GStreamer init, camera/window/pipeline/keyboard setup in correct order | §3.1 | §2, §4.1 |
| T-7.3 | Implement GTK+ main event loop with signal handling (SIGINT, SIGTERM) and graceful shutdown (Escape/Ctrl+C) | §3.1 | - |
| T-7.4 | Implement atexit handlers and cleanup routines for pipeline, window, camera, and memory deallocation | §3.1 | - |
| T-7.5 | Implement comprehensive error handling: camera not found, permission denied, GStreamer failures, state transitions | §3.1, §3.2, §7 | §4.1, §5.2 |
| T-7.6 | Test application launch/shutdown, keyboard responsiveness, error dialogs, resource cleanup, and no memory leaks | §8.4 | - |

---

## Phase 8: Performance Optimization & 120 FPS Target

**Goal**: Profile pipeline, optimize queue/synchronization settings, and achieve sustained 120 fps playback.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-8.1 | Profile GStreamer pipeline using gst-tracepoints and perf; identify bottlenecks in queue buffering and synchronization | §3.4, §5.1 | - |
| T-8.2 | Optimize queue sizes (max-size-buffers, latency settings) and osxvideosink sync settings for 120 fps stability | §3.4, §3.8 | §4.6, §5.1 |
| T-8.3 | Implement frame drop detection, logging, and frame rate measurement utilities for validation | §3.4, §7.4 | - |
| T-8.4 | Benchmark GPU memory bandwidth under 9 simultaneous recordings; measure CPU utilization (<5%) and memory growth (<10%/hour) | §3.5, §5.1, §5.2 | - |
| T-8.5 | Test sustained 120 fps playback across all 10 cells for 30-minute session, CPU/memory/GPU stability, and no frame drops | §8.4 | §5.1, §5.2 |

---

## Phase 9: Comprehensive Error Handling & Edge Cases

**Goal**: Implement error recovery, edge case handling, and robustness improvements.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-9.1 | Implement camera error handling: camera not found, permission denied, disconnection during session, reconnection logic | §3.2, §7 | §4.1, §5.2 |
| T-9.2 | Implement GStreamer error handling: bus errors, state change failures, deadlock detection with recovery attempts | §3.4, §7 | - |
| T-9.3 | Implement recording buffer edge cases: overflow handling, short key presses (<33ms), multiple recordings beyond 9 cells | §3.5, §7 | §4.5, §5.2 |
| T-9.4 | Implement keyboard and window event error recovery; display user-friendly error dialogs for fatal errors | §3.7, §3.8, §7 | - |
| T-9.5 | Test camera permission denied, camera not found, brief camera disconnect, short key press (<1 frame), and wraparound beyond cell 10 | §8.4 | - |

---

## Phase 10: Comprehensive Testing & Quality Assurance

**Goal**: Execute complete test suite (unit, integration, E2E) and verify code quality standards.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-10.1 | Create and run unit tests for recording_state, buffer_manager, playback_manager, keyboard_handler (25 tests, 85%+ coverage) | §8.2 | - |
| T-10.2 | Create and run integration tests for pipeline building, recording→playback flow, 120 fps measurement (9 tests, 60%+ coverage) | §8.3 | - |
| T-10.3 | Create and run E2E tests: app launch time (<2s), camera permission handling, grid display verification (3 tests) | §8.4 | §5.1 |
| T-10.4 | Run clang-format for code style consistency; verify no compiler warnings in Release build | §6 | - |
| T-10.5 | Run final integration test: 10-cell grid at 120 fps, 30-minute stability test, <50ms keyboard latency, live feed persistence | §8.4 | §4.3, §4.8, §5.1 |

---

## Summary of Deliverables by Phase

| Phase | Focus | Key Deliverable |
|-------|-------|-----------------|
| **1** | Foundation | Project structure, build system, utilities |
| **2** | Input/Output | Camera initialization, Cocoa window, osxvideosink |
| **3** | Pipeline | Core GStreamer pipeline, videomixer, bus messages |
| **4** | Recording/Playback Logic | Keyboard input, recording state, buffer management, palindrome algorithm |
| **5** | Recording Pipeline | Record bins, live tee, frame routing to GPU buffers |
| **6** | Playback Pipeline | Playback bins, complete E2E flow (record→playback→display) |
| **7** | Integration | Main app, event loop, error handling, startup/shutdown |
| **8** | Performance | 120 fps optimization, profiling, benchmarking |
| **9** | Robustness | Error recovery, edge cases, graceful degradation |
| **10** | Quality | Unit/integration/E2E tests, code style, final validation |

---

## Implementation Notes

### Right-Sizing Rationale

**Original**: 16 phases, 116 tasks → Over-phased with redundant phase boundaries
**Right-Sized**: 10 phases, 58 explicit tasks + integrated testing = ~80-90 total work items

**Consolidation Strategy**:
- Merged phases 1-2 (setup): Foundation + tool config combined
- Merged phases 2-4 (I/O): Camera, window, osxvideosink in one coherent phase
- Merged phases 3, 6-10: Pipeline, infrastructure, and composition simplified into 3 phases (core pipeline, live feed, grid composition)
- Merged phases 5, 8-9: Recording/playback logic consolidated into single phase
- Merged phases 8-9: Record/playback bins logic merged into single phase covering full E2E flow
- Merged phases 11-12: Main integration + E2E flow coordination into single phase
- Merged phases 13-14: Performance + error handling into final two phases (8-9)
- Merged phases 15-16: Testing + documentation consolidated into Phase 10

### MVP-First Delivery (Phases 1-7)

Phases 1-7 deliver complete MVP:
- Single record/playback loop
- Live feed in cell 1
- 10-cell grid at 120 fps
- Keyboard control
- Full error handling
- Main event loop

All core functionality is operational after Phase 7.

### Performance-Critical Path (Phases 8-9)

Phases 8-9 focus on non-functional requirements:
- Sustained 120 fps validation
- GPU memory optimization
- Error resilience
- Edge case handling

These are sequential to MVP but enable production-grade quality.

### Quality Assurance (Phase 10)

Final phase integrates comprehensive testing:
- All test categories (unit, integration, E2E)
- Code quality (formatting, warnings)
- Documentation (included as task notes, not separate phase)

### Task Dependencies

```
Phase 1 (Foundation)
  ↓
Phase 2 (Camera + Window)
  ↓
Phase 3 (Pipeline Core)
  ↓
Phase 4 (Recording/Playback Logic)
  ↓
Phase 5 (Recording Pipeline)
  ↓
Phase 6 (Playback Pipeline + E2E)
  ↓
Phase 7 (Integration + Event Loop) ← MVP Complete
  ↓
Phase 8 (Performance) ← Can run in parallel with Phase 9
Phase 9 (Error Handling) ↑
  ↓
Phase 10 (Testing + QA)
```

### Success Criteria per Phase

| Phase | Pass Criteria |
|-------|---------------|
| 1 | Project builds, no errors |
| 2 | Camera detected, window visible, osxvideosink renders |
| 3 | Pipeline creates, state transitions work, live queue feeds cell 1 |
| 4 | Keyboard captures input, recording state tracks correctly, palindrome algorithm verified |
| 5 | Record bins created, frames flow to GPU buffer, no deadlock |
| 6 | Playback bins created, complete flow: key press→record→playback in grid cell |
| 7 | App launches, accepts input, exits cleanly, resources freed |
| 8 | 120 fps sustained, <5% CPU, <10% memory growth/hour |
| 9 | Error dialogs appear, camera disconnect recovered, edge cases handled |
| 10 | All tests pass, code formatted, no warnings, 85%+ coverage |

---

**Document Version**: 1.0 (Right-Sized)
**Last Updated**: January 27, 2026
**Total Phases**: 10 (was 16)
**Total Explicit Tasks**: 58
**Total Work Items** (including implicit test tasks): ~80-90
**Status**: Ready for Implementation

## Phase 11: Finalization

**Goal**: Perform final cleanup, organization, and validation to ensure the project is production-ready.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-11.1 | System Integration Audit: Scan all source code files for incomplete implementations including TODO, FIXME, TBD, XXX, HACK markers, stub functions, and empty or partial implementations. Analyze each placeholder's context to distinguish actionable incomplete work from legitimate documentation notes. For actionable items: implement stub functions with actual logic, complete partial implementations, add missing error handling, and resolve or remove hack markers. Skip placeholders for out-of-scope features and third-party/vendored code. Additionally, within the generated project, audit the main application entry point and execution paths to confirm every core subsystem is reachable (services, modules, plugins, layers, integrations defined in the PRD/SDD). Ensure wiring from the main entry point instantiates and connects all required components for the unified system. If any component is orphaned or unreachable, fix the integration so all required pieces are connected. Run syntax checks and tests ONLY if code modifications were made. Report success when no actionable placeholders remain, all have been resolved, and the generated application's entry point links all required subsystems. | - | - |
| T-11.2 | TTL Verification: Review each task in the TTL in order. For each, read the task description and verify it aligns with the referenced SDD and PRD requirements (refs already provided in the original agent prompt). Then confirm the code implements the task as described. If the implementation matches, return SUCCESS. If there are discrepancies, complete partial implementations or fix bugs to achieve consistency. If verification or remediation cannot be completed, return FAILURE. | - | - |
| T-11.3 | Documentation Organization: Reorganize documentation files in the project root directory into a structured docs/ directory with logical subdirectories. Evaluate each file before moving. Preserve README.md, CHANGELOG, LICENSE, NOTICE, and build-related files (requirements.txt, package.json, etc.) in root. Report success when documentation is properly organized or no reorganization was needed. | - | - |
