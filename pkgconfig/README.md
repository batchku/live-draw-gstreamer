# pkg-config Definitions for Video Looper

This directory contains pkg-config (.pc) definition files for macOS frameworks required by Video Looper.

## Contents

### Framework Definitions

- **Cocoa.pc** - Native macOS window management and UI framework
- **AVFoundation.pc** - Camera access, media capture, and permission handling
- **CoreFoundation.pc** - Fundamental macOS system utilities and data structures
- **CoreMedia.pc** - Low-level media data structure handling
- **CoreVideo.pc** - GPU-accelerated video frame management

## Purpose

These .pc files enable the build system (Meson) to discover and link against macOS frameworks using the standard pkg-config mechanism. Without these definitions, framework discovery on macOS requires hardcoded path and framework flags.

## How It Works

1. **Build scripts automatically set PKG_CONFIG_PATH** to include this directory
2. **pkg-config finds the .pc files** in this directory
3. **Meson uses pkg-config** to discover framework paths and linking requirements
4. **Compilation and linking** use the proper include directories and library flags

## Usage

### Automatic (Recommended)

Use the provided build script:
```bash
./scripts/build.sh
```

This script automatically sets PKG_CONFIG_PATH before invoking Meson.

### Manual

Set PKG_CONFIG_PATH explicitly:
```bash
export PKG_CONFIG_PATH="$(pwd)/pkgconfig:$PKG_CONFIG_PATH"
meson setup build
```

## Verification

Validate that all frameworks are discoverable:

```bash
export PKG_CONFIG_PATH="$(pwd)/pkgconfig:$PKG_CONFIG_PATH"
pkg-config --list-all | grep -E "Cocoa|AVFoundation|CoreMedia|CoreVideo|CoreFoundation"
```

Expected output:
```
cocoa Cocoa - macOS Cocoa framework for native application development
avfoundation AVFoundation - macOS AVFoundation framework for media capture and processing
corefoundation CoreFoundation - macOS CoreFoundation framework for fundamental services
coremedia CoreMedia - macOS CoreMedia framework for media data structure handling
corevideo CoreVideo - macOS CoreVideo framework for video frame handling
```

## Technical Details

### .pc File Format

Each .pc file follows the standard pkg-config format:

```ini
prefix=/System/Library/Frameworks
frameworkdir=${prefix}/FrameworkName.framework
incdir=${frameworkdir}/Headers
libdir=${frameworkdir}

Name: FrameworkName
Description: Framework description
Version: 15.7
Cflags: -I${incdir}
Libs: -F${prefix} -framework FrameworkName
```

### Framework Dependency Chain

Some frameworks depend on others:

```
Cocoa
  (no dependencies)

AVFoundation
  → CoreMedia
  → CoreVideo
  → CoreFoundation

CoreMedia
  → CoreFoundation

CoreVideo
  → CoreFoundation

CoreFoundation
  (no dependencies)
```

The .pc files declare these dependencies using the `Requires:` field, so pkg-config automatically resolves transitive dependencies.

## Customization

### Updating Framework Versions

If you need to support a specific macOS version, update the `Version:` field in the corresponding .pc file:

```ini
Version: 15.7
```

Change `15.7` to your target minimum macOS version.

### Adding New Frameworks

To add a new macOS framework:

1. Create a new .pc file (e.g., `NewFramework.pc`)
2. Follow the format shown above
3. Add any dependencies in the `Requires:` field
4. Run the validation script to verify discovery:
   ```bash
   ./scripts/pkgconfig_setup.sh
   ```

## Troubleshooting

### Framework Not Found

If pkg-config cannot find a framework:

1. Verify PKG_CONFIG_PATH includes this directory:
   ```bash
   echo $PKG_CONFIG_PATH
   ```

2. Verify the .pc file exists:
   ```bash
   ls -la pkgconfig/FrameworkName.pc
   ```

3. Check if the framework is actually installed on your system:
   ```bash
   ls -d /System/Library/Frameworks/FrameworkName.framework
   ```

4. Run the setup validation:
   ```bash
   ./scripts/pkgconfig_setup.sh
   ```

### Version Mismatch

If you get a "version not satisfied" error, either:

1. Update the macOS version in the .pc file to match your system, or
2. Update your macOS to the required version

Check your macOS version:
```bash
sw_vers
```

## References

- [pkg-config Manual](https://pkg-config.freedesktop.org/)
- [macOS Frameworks](https://developer.apple.com/documentation/corefoundation)
- [Meson Dependency Resolution](https://mesonbuild.com/Dependency-control.html)

---

**Last Updated:** January 27, 2026
**Version:** 1.0
