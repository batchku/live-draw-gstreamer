# pkg-config Configuration Implementation for Video Looper

## Task: T-1.2 - Configure pkg-config for GStreamer, Cocoa, and AVFoundation dependencies

**Status:** ✅ COMPLETED

**Date:** January 27, 2026

## Summary

Task T-1.2 configures pkg-config to discover and resolve GStreamer, Cocoa, and AVFoundation dependencies for the Video Looper macOS application build system. This enables Meson to use standard dependency resolution for all framework and library dependencies.

## What Was Implemented

### 1. Custom pkg-config Definition Files

Created five custom `.pc` (pkg-config) definition files in `pkgconfig/` directory:

- **Cocoa.pc** - Native macOS Cocoa framework for window management
- **AVFoundation.pc** - Camera access and media capture framework
- **CoreFoundation.pc** - Fundamental macOS system utilities
- **CoreMedia.pc** - Low-level media data handling
- **CoreVideo.pc** - GPU-accelerated video frame management

Each .pc file:
- Defines framework location: `/System/Library/Frameworks/`
- Specifies compiler flags (include directories)
- Specifies linker flags (framework linking)
- Declares framework dependencies
- Sets version requirements (macOS 15.7+)

### 2. Build System Integration

**Updated `scripts/build.sh`:**
- Automatically sets `PKG_CONFIG_PATH` to include `pkgconfig/` directory
- Ensures all builds use the custom framework definitions
- Displays PKG_CONFIG_PATH in build header for debugging

**Updated `meson.build`:**
- Changed from fallback framework linking to exclusive pkg-config discovery
- All framework dependencies now use `method: 'pkg-config'` with `required: true`
- Proper error propagation if frameworks cannot be found
- Fixed Meson deprecation warning (gui_app → win_subsystem)

### 3. pkg-config Validation Script

Created `scripts/pkgconfig_setup.sh`:
- Validates pkg-config installation
- Checks all .pc files exist
- Tests dependency discovery
- Displays version information
- Provides setup instructions

### 4. Documentation

**Created `/docs/build/PKG_CONFIG_SETUP.md`:**
- Comprehensive guide to pkg-config configuration
- Troubleshooting section
- Best practices
- Reference links

**Created `pkgconfig/README.md`:**
- Overview of framework definitions
- Usage instructions
- Technical details and customization guide

## Files Created

```
pkgconfig/
├── Cocoa.pc                           (Framework definition)
├── AVFoundation.pc                    (Framework definition)
├── CoreFoundation.pc                  (Framework definition)
├── CoreMedia.pc                       (Framework definition)
├── CoreVideo.pc                       (Framework definition)
└── README.md                          (Framework documentation)

scripts/
└── pkgconfig_setup.sh                 (Validation script)

docs/build/
├── PKG_CONFIG_SETUP.md               (Setup and troubleshooting guide)
└── PKG_CONFIG_IMPLEMENTATION.md       (This file)
```

## Files Modified

```
scripts/build.sh                       (Added PKG_CONFIG_PATH setup)
meson.build                            (Updated framework dependency resolution)
```

## Validation Results

### pkg-config Setup Validation

✅ All checks passed:

```
[✓] pkg-config found: 2.5.1
[✓] Cocoa.pc found
[✓] AVFoundation.pc found
[✓] CoreFoundation.pc found
[✓] CoreMedia.pc found
[✓] CoreVideo.pc found
[✓] gstreamer-1.0 discoverable (1.26.10)
[✓] glib-2.0 discoverable (2.86.3)
[✓] gobject-2.0 discoverable (2.86.3)
[✓] gstreamer-video-1.0 discoverable (1.26.10)
[✓] gstreamer-gl-1.0 discoverable (1.26.10)
[✓] Cocoa discoverable (15.7)
[✓] AVFoundation discoverable (15.7)
[✓] CoreFoundation discoverable (15.7)
[✓] CoreMedia discoverable (15.7)
[✓] CoreVideo discoverable (15.7)
```

### Meson Build Configuration

✅ Clean build configuration:

```
Found pkg-config: YES (/opt/homebrew/bin/pkg-config) 2.5.1
Run-time dependency gstreamer-1.0 found: YES 1.26.10
Run-time dependency glib-2.0 found: YES 2.86.3
Run-time dependency gobject-2.0 found: YES 2.86.3
Run-time dependency gstreamer-video-1.0 found: YES 1.26.10
Run-time dependency gstreamer-gl-1.0 found: YES 1.26.10
Run-time dependency cocoa found: YES 15.7
Run-time dependency avfoundation found: YES 15.7
Run-time dependency corefoundation found: YES 15.7
Run-time dependency coremedia found: YES 15.7
Run-time dependency corevideo found: YES 15.7
Build targets in project: 1
```

All frameworks properly resolved through pkg-config ✅

## How It Works

### 1. Dependency Discovery Flow

```
User runs: ./scripts/build.sh
            ↓
PKG_CONFIG_PATH=$PWD/pkgconfig:$PKG_CONFIG_PATH
            ↓
meson setup build
            ↓
Meson calls: pkg-config --modversion Cocoa
             pkg-config --cflags Cocoa
             pkg-config --libs Cocoa
            ↓
pkg-config reads: pkgconfig/Cocoa.pc
            ↓
Returns framework paths and compilation flags
            ↓
Meson configures build with proper include and linker flags
            ↓
ninja compiles with all frameworks properly linked
```

### 2. Framework Dependency Resolution

pkg-config automatically resolves framework dependencies:

```
AVFoundation depends on:
  → CoreMedia
  → CoreVideo
  → CoreFoundation

When requesting AVFoundation, pkg-config automatically includes:
  - Cflags for all dependencies
  - Libs for all dependencies
```

This is declared in `Requires:` field in the .pc files.

### 3. Compilation Example

When compiling with AVFoundation, the compiler receives:

```
Cflags:
  -I/System/Library/Frameworks/AVFoundation.framework/Headers
  -I/System/Library/Frameworks/CoreMedia.framework/Headers
  -I/System/Library/Frameworks/CoreVideo.framework/Headers
  -I/System/Library/Frameworks/CoreFoundation.framework/Headers

Libs:
  -F/System/Library/Frameworks
  -framework AVFoundation
  -framework CoreMedia
  -framework CoreVideo
  -framework CoreFoundation
```

All transitive dependencies automatically included!

## Usage

### Build the Project

```bash
# Automatic setup (recommended)
./scripts/build.sh

# Or with options
./scripts/build.sh --debug
./scripts/build.sh --release
./scripts/build.sh --clean
```

### Validate Configuration

```bash
# Run pkg-config setup validation
./scripts/pkgconfig_setup.sh
```

### Manual Build (If Needed)

```bash
export PKG_CONFIG_PATH="$(pwd)/pkgconfig:$PKG_CONFIG_PATH"
meson setup build --buildtype=release
cd build
ninja
```

## Benefits

### For Developers

1. **Clear Dependencies:** Explicit declaration of all required frameworks
2. **Version Safety:** Version mismatches caught at configuration time
3. **Portability:** Same build process works across different environments
4. **Debugging:** PKG_CONFIG_PATH visible in build output for troubleshooting

### For Build System

1. **Standard Resolution:** Uses industry-standard pkg-config mechanism
2. **Automatic Transitive Deps:** Framework dependencies resolved automatically
3. **Fail-Fast:** Missing frameworks caught immediately, not at link time
4. **Reproducible:** Same .pc files ensure consistent builds

### For Project Maintenance

1. **Centralized Configuration:** All framework setup in one place
2. **Easy Updates:** Change framework versions in one location
3. **Documentation:** Each .pc file serves as framework documentation
4. **Extensibility:** Adding new frameworks is straightforward

## SDD/PRD Compliance

### SDD Reference: §5 - Technology Stack

Task implements framework configuration specified in SDD §5:

| Technology | Status |
|-----------|--------|
| **GStreamer 1.20+** | ✅ Discoverable via system pkg-config |
| **Cocoa Framework** | ✅ Custom .pc file, version 15.7+ |
| **AVFoundation** | ✅ Custom .pc file, version 15.7+ |
| **CoreFoundation** | ✅ Custom .pc file, version 15.7+ |
| **CoreMedia** | ✅ Custom .pc file, version 15.7+ |
| **CoreVideo** | ✅ Custom .pc file, version 15.7+ |

All dependencies configured for discovery via pkg-config ✅

## Troubleshooting

### Issue: "Dependency 'Cocoa' not found"

**Solution:** Run the build script which sets PKG_CONFIG_PATH:
```bash
./scripts/build.sh
```

### Issue: "Version 15.7 not satisfied"

**Solution:** Update to macOS 15.7+ or edit the .pc files to match your version:
```bash
# Check your macOS version
sw_vers

# Edit .pc file if needed
sed -i '' 's/Version: 15.7/Version: YOUR.VERSION/g' pkgconfig/*.pc
```

### Issue: GStreamer not found

**Solution:** Install GStreamer:
```bash
brew install gstreamer gstreamer-plugins-base
```

Then verify:
```bash
pkg-config --modversion gstreamer-1.0
```

### Debug: Check all dependencies

```bash
./scripts/pkgconfig_setup.sh
```

This script:
- Verifies pkg-config installation
- Checks all .pc files exist
- Tests dependency discovery
- Displays version information

## Related Tasks

- **T-1.1:** Create project structure and initialize build system
- **T-1.3:** Create main.c entry point
- **T-1.4:** Set up build scripts
- **T-2.1:** Camera source initialization (uses these dependencies)
- **T-2.3:** Cocoa window management (uses these dependencies)

## Next Steps

After T-1.2 completion, the build system is ready for:

1. **T-1.3:** Implementing main.c entry point
2. **T-1.4:** Creating build scripts (already provided)
3. **T-2.x:** Implementing camera and window components using these frameworks

## References

- **pkg-config Manual:** https://pkg-config.freedesktop.org/
- **Meson Dependencies:** https://mesonbuild.com/Dependency-control.html
- **GStreamer Installation:** https://gstreamer.freedesktop.org/documentation/installing/on-macos.html
- **Cocoa Framework:** https://developer.apple.com/documentation/appkit/cocoa
- **AVFoundation:** https://developer.apple.com/documentation/avfoundation

---

**Document:** PKG_CONFIG_IMPLEMENTATION.md
**Version:** 1.0
**Status:** Complete
**Date:** January 27, 2026
