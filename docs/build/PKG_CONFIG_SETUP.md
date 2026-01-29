# pkg-config Configuration for Video Looper

## Overview

This document explains how Video Looper uses pkg-config to discover and link against GStreamer, Cocoa, and AVFoundation dependencies on macOS.

## The Problem: Framework Discovery on macOS

On macOS, native frameworks (Cocoa, AVFoundation, etc.) are not standard system libraries. They exist as framework bundles in `/System/Library/Frameworks/` and are not discoverable by standard pkg-config without custom configuration.

**Without proper setup:**
- pkg-config cannot find Cocoa, AVFoundation, or other frameworks
- Build systems must hardcode framework paths using `-framework` compiler flags
- Dependency management becomes fragile and platform-specific

**With proper setup (this project):**
- All dependencies are discoverable via `pkg-config`
- Meson can use standard dependency resolution
- Dependencies are explicitly versioned and managed
- Build configuration is portable and reproducible

## Solution: Custom .pc Files

Video Looper includes custom pkg-config definition files (.pc) for all required macOS frameworks:

```
pkgconfig/
├── Cocoa.pc              # Native macOS window management
├── AVFoundation.pc       # Camera access and media capture
├── CoreFoundation.pc     # Fundamental system utilities
├── CoreMedia.pc          # Media data structure handling
└── CoreVideo.pc          # GPU video frame management
```

### .pc File Format

Each .pc file follows the standard pkg-config format:

```ini
prefix=/System/Library/Frameworks
frameworkdir=${prefix}/Cocoa.framework
incdir=${frameworkdir}/Headers
libdir=${frameworkdir}

Name: Cocoa
Description: macOS Cocoa framework for native application development
Version: 15.7
Cflags: -I${incdir}
Libs: -F${prefix} -framework Cocoa
```

**Key components:**
- `prefix`: Root path to macOS frameworks
- `Name`: Identifier used in `pkg-config --modversion Cocoa`
- `Version`: macOS minimum version
- `Cflags`: Compiler flags (include directories)
- `Libs`: Linker flags (framework linking)
- `Requires`: Dependencies on other frameworks

## Configuration

### Automatic Setup (Recommended)

The build system automatically configures pkg-config:

1. **Via build.sh:**
   ```bash
   ./scripts/build.sh
   ```
   This script sets `PKG_CONFIG_PATH` to include `pkgconfig/` before invoking Meson.

2. **Via pkgconfig_setup.sh (Manual Validation):**
   ```bash
   ./scripts/pkgconfig_setup.sh
   ```
   This script validates all dependencies are discoverable and displays version info.

### Manual Setup

If building outside the provided scripts, set PKG_CONFIG_PATH:

```bash
export PKG_CONFIG_PATH="$(pwd)/pkgconfig:$PKG_CONFIG_PATH"
meson setup build
```

### Permanent Setup

Add to your shell configuration (`~/.zshrc`, `~/.bashrc`, etc.):

```bash
export PKG_CONFIG_PATH="/path/to/video-looper/pkgconfig:$PKG_CONFIG_PATH"
```

## Verification

### Check pkg-config Installation

```bash
pkg-config --version
```

Expected output: `pkg-config 0.29.2` (version may vary)

### Check Framework Discovery

```bash
export PKG_CONFIG_PATH="$(pwd)/pkgconfig:$PKG_CONFIG_PATH"
pkg-config --modversion Cocoa
pkg-config --modversion AVFoundation
```

Expected output:
```
15.7
15.7
```

### Check Compiler Flags

```bash
pkg-config --cflags Cocoa
pkg-config --libs Cocoa
```

Expected output:
```
-I/System/Library/Frameworks/Cocoa.framework/Headers -fno-objc-arc
-F/System/Library/Frameworks -framework Cocoa
```

### Run Validation Script

```bash
./scripts/pkgconfig_setup.sh
```

This script:
1. Verifies pkg-config installation
2. Checks all .pc files exist
3. Validates dependency discovery
4. Displays version information

## Dependencies

### GStreamer Dependencies

Resolved via system pkg-config (installed via Homebrew or macOS ports):

- `gstreamer-1.0` (>= 1.20)
- `glib-2.0` (>= 2.70)
- `gobject-2.0`
- `gstreamer-video-1.0`
- `gstreamer-gl-1.0` (for GPU acceleration)

**Installation:**
```bash
brew install gstreamer gstreamer-plugins-base
```

### macOS Framework Dependencies

Defined via custom .pc files in `pkgconfig/`:

- `Cocoa` (macOS window management)
- `AVFoundation` (camera and media)
- `CoreFoundation` (system utilities)
- `CoreMedia` (media data handling)
- `CoreVideo` (GPU video frames)

These frameworks are part of the macOS SDK and available on all modern Macs.

## Meson Integration

The project's `meson.build` uses pkg-config for dependency resolution:

```python
cocoa_dep = dependency('Cocoa',
  version: '>= 15.7',
  method: 'pkg-config',
  required: true
)

avfoundation_dep = dependency('AVFoundation',
  version: '>= 15.7',
  method: 'pkg-config',
  required: true
)
```

**Benefits:**
- Explicit version requirements (fail-fast if version mismatch)
- Standard Meson dependency API
- Portable across different build environments
- Clear dependency declarations

## Troubleshooting

### Error: "Dependency 'Cocoa' not found"

**Cause:** PKG_CONFIG_PATH doesn't include `pkgconfig/` directory

**Solution:**
```bash
export PKG_CONFIG_PATH="$(pwd)/pkgconfig:$PKG_CONFIG_PATH"
./scripts/build.sh
```

Or run the automatic setup:
```bash
./scripts/pkgconfig_setup.sh
```

### Error: "Version '15.7' not satisfied"

**Cause:** System doesn't meet minimum macOS version requirements

**Solution:** Update macOS or edit `.pc` files to match your system version (not recommended)

### pkg-config cannot find GStreamer

**Cause:** GStreamer not installed or not in system PKG_CONFIG_PATH

**Solution:**
```bash
brew install gstreamer
brew install gstreamer-plugins-base
```

Verify installation:
```bash
pkg-config --modversion gstreamer-1.0
```

## Best Practices

1. **Always run build via provided scripts:**
   ```bash
   ./scripts/build.sh
   ```

2. **Validate setup before building:**
   ```bash
   ./scripts/pkgconfig_setup.sh
   ```

3. **Check PKG_CONFIG_PATH in build logs:**
   The build script displays PKG_CONFIG_PATH in its header.

4. **Don't modify .pc files** unless updating macOS version requirements.

5. **Install dependencies via Homebrew:**
   ```bash
   brew install gstreamer gstreamer-plugins-base
   ```

## Reference

### pkg-config Documentation

- Official Manual: https://pkg-config.freedesktop.org/
- Wiki: https://wiki.freedesktop.org/pkg-config/

### macOS Framework Documentation

- Cocoa: https://developer.apple.com/documentation/appkit/cocoa
- AVFoundation: https://developer.apple.com/documentation/avfoundation
- CoreFoundation: https://developer.apple.com/documentation/corefoundation
- CoreMedia: https://developer.apple.com/documentation/coremedia
- CoreVideo: https://developer.apple.com/documentation/corevideo

### Meson Dependency Resolution

- Meson Dependency Objects: https://mesonbuild.com/Dependency-control.html
- pkg-config Method: https://mesonbuild.com/Dependency-control.html#pkg-config

---

**Last Updated:** January 27, 2026
**Document Version:** 1.0
**Status:** Complete
