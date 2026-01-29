# Video Looper for macOS

A lightweight, GPU-accelerated real-time video looping application for macOS. Record and playback multiple short video loops simultaneously in a 10-cell grid layout with keyboard control.

## Quick Start

### Requirements

- macOS 15.7 or later
- GStreamer 1.20 or later
- Meson build system (1.0+)
- Ninja build tool
- C99 compiler (clang, gcc)

### Installation

1. **Install dependencies** (macOS with Homebrew):

```bash
brew install gstreamer glib meson ninja pkg-config
```

2. **Build the project**:

```bash
./scripts/build.sh
```

3. **Run the application**:

```bash
./scripts/run.sh
```

## Usage

### Application Controls

- **Press and hold keys 1-9**: Record video in cells 2-10
- **Release key**: Stop recording and begin palindrome playback
- **Escape or Ctrl+C**: Exit application

### Grid Layout

The application displays a 10-cell horizontal grid:
- **Cell 1** (leftmost): Live camera feed
- **Cells 2-10**: Recorded video loops with palindrome playback

Each cell is 320 pixels wide. The window height is calculated to maintain the camera's aspect ratio.

### Video Features

- **Real-time capture** from built-in camera at 120 fps
- **Palindrome playback**: Videos play forward then backward in a seamless loop
- **GPU acceleration**: All video processing occurs on GPU
- **Multiple simultaneous recordings**: Hold multiple keys at once to record different loops

## Development

### Build Commands

```bash
# Build (release optimized)
./scripts/build.sh

# Build with debug symbols
./scripts/build.sh --debug

# Clean build
./scripts/build.sh --clean

# Build with verbose output
./scripts/build.sh --verbose
```

### Run Commands

```bash
# Run application
./scripts/run.sh

# Build and run
./scripts/run.sh --build

# Run with debug output
./scripts/run.sh --debug

# Run under Valgrind memory profiler
./scripts/run.sh --valgrind
```

### Test Commands

```bash
# Run all tests
./scripts/test.sh

# Run unit tests only
./scripts/test.sh --unit

# Run integration tests only
./scripts/test.sh --integration

# Generate code coverage report
./scripts/test.sh --coverage

# Build and run tests
./scripts/test.sh --build
```

### Performance Profiling

```bash
# Profile CPU usage (requires 'perf')
./scripts/profile.sh --cpu

# Profile memory usage (requires Valgrind)
./scripts/profile.sh --memory

# Profile GPU usage (requires gputop)
./scripts/profile.sh --gpu

# Run profiler for 60 seconds
./scripts/profile.sh --duration 60
```

### Code Formatting

```bash
# Format all source files
./scripts/format_code.sh

# Check formatting without modifying
./scripts/format_code.sh --check

# Dry run (show what would be formatted)
./scripts/format_code.sh --dry-run
```

## Project Structure

```
video-looper-osx-5/
├── src/                          # Source code
│   ├── main.c                    # Application entry point
│   ├── app/                      # Application context and error handling
│   ├── camera/                   # Camera source and permission handling
│   ├── gstreamer/                # GStreamer pipeline building
│   ├── recording/                # Recording state and buffer management
│   ├── playback/                 # Playback logic and bin management
│   ├── input/                    # Keyboard input handling
│   ├── osx/                      # macOS-specific window management
│   └── utils/                    # Logging, memory, timing utilities
├── test/                         # Tests
│   ├── unit/                     # Unit tests
│   ├── integration/              # Integration tests
│   └── e2e/                      # End-to-end tests
├── docs/                         # Documentation
│   └── planning/                 # PRD, SDD, TTL
├── scripts/                      # Build and development scripts
├── build/                        # Generated build artifacts (created by Meson)
├── meson.build                   # Meson build configuration
├── meson_options.txt            # Build options
└── README.md                     # This file
```

## Documentation

- **[PRD](docs/planning/PRD.md)** - Product Requirements Document
- **[SDD](docs/planning/SDD.md)** - Software Design Document
- **[TTL](docs/planning/TTL.md)** - Technical Task List

## Architecture

The application uses a **GStreamer Pipeline Architecture**:

1. **Camera Source** → **Live Tee** → Splits to:
   - Live queue (Cell 1 display)
   - Recording bins (Keys 1-9)

2. **Recording Bins** → **GPU Ring Buffers** → **Playback Bins** → **Videomixer** → **osxvideosink** → **Cocoa Window**

All video processing occurs on the GPU. The CPU handles only control flow (keyboard input, state management).

### Performance Targets

- **Frame Rate**: 120 fps sustained across all 10 cells
- **Input Latency**: < 50 ms keyboard response
- **CPU Usage**: < 5% of single core
- **Memory**: Stable growth < 10%/hour
- **GPU Memory**: ~3.4 GB for 9 simultaneous recordings

## Error Handling

The application handles:
- Camera not found → Exit with error dialog
- Permission denied → Display system dialog, exit
- Camera disconnected → Log warning, graceful degradation
- Recording buffer full → Drop oldest frame, continue
- Keyboard handler failure → Attempt reinitialize, continue

## Testing

### Test Categories

1. **Unit Tests** (25 tests, 85% coverage)
   - Recording state machine
   - Buffer management
   - Palindrome playback algorithm
   - Keyboard input

2. **Integration Tests** (9 tests, 60% coverage)
   - Pipeline building and state transitions
   - Recording → playback flow
   - 120 fps rendering validation

3. **End-to-End Tests** (3 tests)
   - Application launch time
   - Camera permission handling
   - Grid display verification

### Code Coverage

```bash
./scripts/test.sh --coverage
```

## Troubleshooting

### Camera Not Detected
- Check System Preferences → Security & Privacy → Camera
- Ensure camera is not in use by another application
- Try restarting the application

### Low Frame Rate
- Check CPU/GPU utilization with Activity Monitor
- Reduce other running applications
- Ensure display is not in power saving mode

### Build Errors
- Ensure GStreamer is properly installed: `pkg-config --modversion gstreamer-1.0`
- Update Homebrew: `brew update && brew upgrade`
- Clean build: `./scripts/build.sh --clean`

### Permission Denied
- Grant camera permission in System Preferences → Security & Privacy → Camera
- The application will not function without camera access

## Technical Stack

| Component | Technology | Version |
|-----------|-----------|---------|
| Runtime | GStreamer | 1.20+ |
| Camera API | AVFoundation | macOS 15.7+ |
| Window Management | Cocoa/NSWindow | macOS 15.7+ |
| GPU Rendering | Metal/OpenGL | Via GStreamer |
| Build System | Meson | 1.0+ |
| Language | C | C99 |

## Performance Characteristics

### Memory Usage
- Application: ~50-100 MB
- Live feed buffer: ~15 MB
- Each recording buffer: ~373 MB (60 frames @ 1920×1080)
- Total for 9 recordings: ~3.4 GB

### Latency
- Keyboard to record: < 50 ms
- Record to playback: < 100 ms
- Frame delivery: 120 fps (8.3 ms per frame)

### CPU/GPU Distribution
- CPU: 2-5% for control flow
- GPU: 85-95% for video processing

## License

MIT License - See LICENSE file for details

## Contributing

See CONTRIBUTING.md for development guidelines.

## Status

- **Version**: 1.0.0
- **Status**: MVP Development
- **Last Updated**: January 27, 2026

---

**Built with ❤️ for creative professionals and developers**
