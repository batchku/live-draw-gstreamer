# Project Structure - Video Looper for macOS

## Overview

This document describes the project structure created for Task T-1.1.

## Directory Tree

```
video-looper-osx-5/
├── docs/
│   └── planning/
│       ├── BRIEF.md           (Project brief)
│       ├── PRD.md             (Product Requirements Document)
│       ├── SDD.md             (Software Design Document)
│       └── TTL.md             (Technical Task List)
│
├── src/                       (Source code)
│   ├── main.c                 (Application entry point)
│   │
│   ├── app/                   (Application core)
│   │   ├── app_context.h/c    (App state management)
│   │   └── app_error.h/c      (Error handling)
│   │
│   ├── camera/                (Camera subsystem)
│   │   ├── camera_source.h/c  (Camera initialization & control)
│   │   └── camera_permissions.m (Objective-C wrapper - TBD)
│   │
│   ├── gstreamer/             (GStreamer pipeline)
│   │   ├── pipeline_builder.h/c    (Pipeline construction)
│   │   └── gst_elements.h/c        (Element creation helpers)
│   │
│   ├── recording/             (Recording subsystem)
│   │   ├── recording_state.h/c     (Record state machine)
│   │   └── buffer_manager.h/c      (GPU ring buffers)
│   │
│   ├── playback/              (Playback subsystem)
│   │   ├── playback_manager.h/c    (Palindrome playback logic)
│   │   └── playback_bin.h/c        (Playback GStreamer bin)
│   │
│   ├── input/                 (Input handling)
│   │   ├── keyboard_handler.h/c    (Keyboard capture)
│   │   └── key_codes.h (TBD)       (Key mappings)
│   │
│   ├── osx/                   (macOS integration)
│   │   ├── window.h/c              (Cocoa window management)
│   │   └── window.mm (TBD)         (Objective-C++ implementation)
│   │
│   └── utils/                 (Utilities)
│       ├── logging.h/c             (Logging system)
│       ├── memory.h/c              (Memory tracking)
│       └── timing.h/c              (High-res timing)
│
├── test/                      (Test suite)
│   ├── unit/
│   │   ├── test_recording_state.c  (Recording state tests - TBD)
│   │   ├── test_buffer_manager.c   (Buffer tests - TBD)
│   │   ├── test_playback_manager.c (Playback tests - TBD)
│   │   ├── test_keyboard_handler.c (Input tests - TBD)
│   │   └── meson.build
│   │
│   ├── integration/
│   │   ├── test_gstreamer_pipeline.c (Pipeline tests - TBD)
│   │   ├── test_recording_flow.c     (E2E flow tests - TBD)
│   │   ├── test_120fps_rendering.c   (Performance tests - TBD)
│   │   └── meson.build
│   │
│   ├── e2e/                   (End-to-end tests)
│   │   ├── test_app_launch.sh  (Startup time - TBD)
│   │   ├── test_camera_permission.sh  (Permissions - TBD)
│   │   └── test_grid_display.sh       (Display - TBD)
│   │
│   └── meson.build
│
├── scripts/                   (Build & development scripts)
│   ├── build.sh              (Meson build automation)
│   ├── run.sh                (Execute application)
│   ├── test.sh               (Run test suite)
│   ├── profile.sh            (Performance profiling)
│   └── format_code.sh        (Code formatting)
│
├── build/                     (Generated artifacts - created by Meson)
│   ├── video-looper          (Final executable)
│   ├── [object files]
│   └── [other build artifacts]
│
├── meson.build              (Root build configuration)
├── meson_options.txt        (Build options)
├── README.md                (User documentation)
├── CONTRIBUTING.md          (Developer guidelines)
├── .gitignore               (Git ignore patterns)
├── LICENSE                  (MIT license - to be created)
├── CHANGELOG.md             (Version history - to be created)
└── PROJECT_STRUCTURE.md     (This file)
```

## Build System

### Meson Build Configuration

- **Root**: `meson.build` - Defines project, dependencies, and main executable
- **Options**: `meson_options.txt` - Build options (debug/release, warnings, tests)
- **Subdirectories**:
  - `test/meson.build` - Test configuration
  - `test/unit/meson.build` - Unit test build
  - `test/integration/meson.build` - Integration test build

### Build Scripts

All scripts are located in `scripts/` and are executable:

| Script | Purpose |
|--------|---------|
| `build.sh` | Compile project with Meson/Ninja |
| `run.sh` | Execute the application |
| `test.sh` | Run test suite |
| `profile.sh` | Performance profiling |
| `format_code.sh` | Code formatting with clang-format |

## Component Organization

### By Layer

**Application Layer**
- `src/main.c` - Entry point
- `src/app/` - Application context and error handling

**Input Layer**
- `src/input/` - Keyboard event handling
- `src/osx/` - Native window management

**Processing Layer**
- `src/camera/` - Camera capture
- `src/gstreamer/` - GStreamer pipeline
- `src/recording/` - Record state and buffers
- `src/playback/` - Palindrome playback

**Utility Layer**
- `src/utils/` - Logging, memory, timing

### By Responsibility

| Component | Responsibility |
|-----------|-----------------|
| `app_context` | Global application state |
| `app_error` | Error codes and handling |
| `camera_source` | Camera initialization & format negotiation |
| `pipeline_builder` | Main GStreamer pipeline construction |
| `gst_elements` | GStreamer element creation helpers |
| `recording_state` | Keyboard → recording state machine |
| `buffer_manager` | GPU memory ring buffers |
| `playback_manager` | Palindrome playback algorithm |
| `playback_bin` | GStreamer playback bin |
| `keyboard_handler` | Keyboard event capture & dispatch |
| `window` | Cocoa window & rendering |
| `logging` | Centralized logging system |
| `memory` | Memory allocation tracking |
| `timing` | High-resolution timing utilities |

## File Naming Conventions

- **Headers**: `component_name.h`
- **Implementations**: `component_name.c`
- **Objective-C files**: `.m` extension
- **Objective-C++ files**: `.mm` extension
- **Tests**: `test_component_name.c`
- **Shell scripts**: `.sh` extension

## Documentation

- **User Docs**: `README.md` - Getting started and usage
- **Developer Docs**: `CONTRIBUTING.md` - Development guidelines
- **Design Docs**: `docs/planning/` - PRD, SDD, TTL

## Development Workflow

1. **Build**: `./scripts/build.sh`
2. **Run**: `./scripts/run.sh`
3. **Test**: `./scripts/test.sh`
4. **Profile**: `./scripts/profile.sh`
5. **Format**: `./scripts/format_code.sh`

## Integration Points (SDD §6)

This project structure matches exactly the directory structure specified in SDD §6:

- ✅ `src/` - All source code
- ✅ `test/` - Unit, integration, E2E tests
- ✅ `docs/` - Planning and architecture documentation
- ✅ `scripts/` - Build and development scripts
- ✅ `build/` - Generated artifacts (created by Meson)
- ✅ Component subdirectories (camera, gstreamer, recording, etc.)
- ✅ Utility modules in `src/utils/`

## Implementation Status

### Completed (T-1.1)
- [x] Directory structure created
- [x] Meson build system configured
- [x] Build scripts implemented
- [x] Utility modules (logging, memory, timing)
- [x] Application context and error handling
- [x] Stub implementations for all major components
- [x] Header files with function signatures
- [x] Documentation (README, CONTRIBUTING)
- [x] .gitignore configuration

### Pending (Future Tasks)
- [ ] Full component implementations
- [ ] Unit tests
- [ ] Integration tests
- [ ] E2E tests
- [ ] Performance optimization
- [ ] Camera permission handling (Objective-C)
- [ ] Cocoa window integration (Objective-C++)
- [ ] GStreamer pipeline building
- [ ] Keyboard input handling

## Dependencies

### External
- **GStreamer**: 1.20+ (video pipeline)
- **GLib**: 2.70+ (data structures, main loop)
- **Cocoa**: macOS system framework (window management)
- **AVFoundation**: macOS system framework (camera access)

### Build Tools
- **Meson**: 1.0+ (build system)
- **Ninja**: (build backend)
- **pkg-config**: (dependency detection)
- **clang**: C99 compiler

## Project Characteristics

- **Language**: C (C99 standard)
- **Platform**: macOS 15.7+
- **Build System**: Meson
- **Testing Framework**: GTest (for unit tests)
- **Code Style**: clang-format

## Next Steps

After T-1.1 (Project Foundation) is complete:

1. **Phase 2**: Camera and window integration
2. **Phase 3**: GStreamer pipeline core
3. **Phase 4**: Recording and playback logic
4. **Phase 5**: Recording pipeline
5. **Phase 6**: Playback pipeline
6. **Phase 7**: Application integration
7. **Phase 8**: Performance optimization
8. **Phase 9**: Error handling
9. **Phase 10**: Testing and QA

---

**Created**: January 27, 2026
**Task**: T-1.1 - Create project structure and initialize Meson build system
**Status**: Complete
