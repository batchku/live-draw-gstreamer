# pkg-config Integration Architecture

**Document**: Architecture guide for pkg-config dependency discovery in Video Looper
**Applies To**: All build configurations and dependency management
**Related**: SDD §5 (Technology Stack), Meson build system

## Overview

Video Looper uses pkg-config as the primary dependency discovery mechanism. This document explains how pkg-config integrates with the build system and why this approach was chosen.

## Why pkg-config?

pkg-config is the standard Unix package discovery tool. Its advantages:

1. **Automatic Flags**: Provides correct -I, -L, -l flags automatically
2. **Version Checking**: Ensures minimum version requirements
3. **Dependency Resolution**: Automatically includes transitive dependencies
4. **Cross-Platform**: Works across Linux, macOS, and BSD
5. **Standard Practice**: Expected by developers familiar with GStreamer

## Dependency Categories

### Category 1: Traditional Libraries (pkg-config primary)

These libraries have standard pkg-config .pc files and are discovered via pkg-config:

- **gstreamer-1.0**: Core streaming framework
- **glib-2.0**: Low-level data structures and object system
- **gobject-2.0**: GObject type system
- **gstreamer-video-1.0**: Video handling elements
- **gstreamer-gl-1.0**: GPU acceleration (Metal/OpenGL abstraction)

**Why pkg-config**: These are Unix-standard libraries installed via package managers. pkg-config provides reliable discovery and correct compiler/linker flags.

**Configuration**:
```meson
gstreamer_dep = dependency('gstreamer-1.0', version: '>= 1.20', required: true)
```

### Category 2: Native Frameworks (Framework Linking)

These are macOS-specific frameworks with no pkg-config .pc files:

- **Cocoa**: Native window management (OS X AppKit)
- **AVFoundation**: Camera and media hardware access
- **CoreFoundation**: Core utilities and data structures
- **CoreMedia**: Video timing and metadata
- **CoreVideo**: GPU memory management

**Why frameworks**: These are system-provided on macOS and must be linked with `-framework` flags.

**Configuration**:
```meson
cocoa_dep = declare_dependency(
  link_args: ['-framework', 'Cocoa'],
  compile_args: []
)
```

## pkg-config Search Path

pkg-config searches for .pc files in a standard path. On macOS with Homebrew:

```
/opt/homebrew/lib/pkgconfig
/usr/local/lib/pkgconfig
/usr/lib/pkgconfig
```

If GStreamer is not found, the PKG_CONFIG_PATH environment variable can be extended:

```bash
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"
```

## Discovery Process

### For GStreamer (Required via pkg-config)

```
Meson build phase:
  1. dependency('gstreamer-1.0', required: true)
  2. Meson invokes pkg-config
  3. pkg-config searches PKG_CONFIG_PATH
  4. .pc file found at /opt/homebrew/lib/pkgconfig/gstreamer-1.0.pc
  5. Compiler/linker flags extracted
  6. Dependency returned to build
  7. Build continues with flags: -I/opt/homebrew/include/gstreamer-1.0 -lgstreamer-1.0
```

If not found: **Build fails** (required: true)

### For Cocoa (Three-tier fallback)

```
Meson build phase:
  1. dependency('Cocoa', method: 'pkg-config', required: false)
  2. pkg-config searches for Cocoa.pc
  3. File not found (macOS system framework, no .pc file)
  4. Dependency returns found: false
  5. Fallback: declare_dependency(link_args: ['-framework', 'Cocoa'])
  6. Compiler/linker flags: -framework Cocoa
  7. Build continues successfully
```

## Meson Integration

The Meson build system manages pkg-config integration:

### In meson.build

```meson
# Primary dependencies via pkg-config
gstreamer_dep = dependency('gstreamer-1.0',
  version: '>= 1.20',
  required: true,
  fallback: [],              # No fallback subproject
  method: 'pkg-config'       # Explicit method (default)
)

# Fallback for frameworks
cocoa_dep = dependency('Cocoa',
  required: false,            # Try but don't fail
  method: 'pkg-config'       # Try pkg-config first
)

if not cocoa_dep.found()
  # Fallback to framework linking
  cocoa_dep = declare_dependency(
    link_args: ['-framework', 'Cocoa']
  )
endif
```

### In executable declaration

```meson
executable('video-looper',
  source_files,
  dependencies: [
    gstreamer_dep,   # From pkg-config
    glib_dep,        # From pkg-config
    cocoa_dep,       # From fallback framework
    # ... more deps
  ],
  include_directories: inc_dirs,
  c_args: c_flags,
  link_args: macos_link_args
)
```

## Version Requirements

| Dependency | Minimum | Latest | Constraint |
|-----------|---------|--------|-----------|
| gstreamer-1.0 | 1.20 | 1.26.10 | >= 1.20 for GPU features |
| glib-2.0 | 2.70 | 2.86.3 | >= 2.70 for GStreamer 1.20 |
| gstreamer-gl-1.0 | 1.20 | 1.26.10 | >= 1.20 for Metal/OpenGL |
| macOS | 15.7 | current | >= 15.7 for AVFoundation API |

## Compile and Link Flags

### From pkg-config

GStreamer and GLib provide flags through their .pc files:

```bash
$ pkg-config --cflags gstreamer-1.0
-I/opt/homebrew/Cellar/gstreamer/1.26.10/include/gstreamer-1.0
-I/opt/homebrew/Cellar/glib/2.86.3/include
-I/opt/homebrew/Cellar/glib/2.86.3/include/glib-2.0

$ pkg-config --libs gstreamer-1.0
-L/opt/homebrew/Cellar/gstreamer/1.26.10/lib
-lgstreamer-1.0 -lglib-2.0 -lgobject-2.0
```

### From Framework Linking

Frameworks provide linker flags directly:

```bash
-framework Cocoa
-framework AVFoundation
-framework CoreFoundation
-framework CoreMedia
-framework CoreVideo
```

### Combined in Compilation

The final compiler command includes both:

```bash
clang \
  -I/opt/homebrew/include/gstreamer-1.0 \
  -I/opt/homebrew/include/glib-2.0 \
  -framework Cocoa \
  -framework AVFoundation \
  -lgstreamer-1.0 \
  -lglib-2.0 \
  ... other flags
```

## Troubleshooting Guide

### Problem: "gstreamer-1.0 not found"

**Cause**: GStreamer not installed or not in PKG_CONFIG_PATH

**Solution**:
```bash
# Check if installed
pkg-config --list-all | grep gstreamer

# Install via Homebrew
brew install gstreamer gstreamer-plugins-base gstreamer-plugins-good

# Add to path if installed elsewhere
export PKG_CONFIG_PATH="/path/to/gstreamer/lib/pkgconfig:$PKG_CONFIG_PATH"

# Reconfigure build
meson setup build --wipe
```

### Problem: "Cocoa framework not found"

**Cause**: macOS frameworks missing (very rare, means Xcode CommandLineTools not installed)

**Solution**:
```bash
# Install Xcode CommandLineTools
xcode-select --install

# Verify frameworks exist
ls /System/Library/Frameworks/Cocoa.framework
ls /System/Library/Frameworks/AVFoundation.framework
```

### Problem: "Wrong GStreamer version found"

**Cause**: Multiple GStreamer installations, using wrong one

**Solution**:
```bash
# Check which pkg-config is being used
which pkg-config

# List where .pc files are found
pkg-config --variable pc_path pkg-config

# Prioritize correct path
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"

# Verify version
pkg-config --modversion gstreamer-1.0

# Reconfigure
meson setup build --wipe
```

## Best Practices

### 1. Always Specify Version Requirements

```meson
# ✅ Good: Version requirement specified
gstreamer_dep = dependency('gstreamer-1.0', version: '>= 1.20')

# ❌ Bad: No version check
gstreamer_dep = dependency('gstreamer-1.0')
```

### 2. Use required: true/false Appropriately

```meson
# ✅ Good: Critical dependency marked required
gstreamer_dep = dependency('gstreamer-1.0', required: true)

# ✅ Good: Optional enhancement is optional
core_media_dep = dependency('CoreMedia', required: false)
```

### 3. Document Why pkg-config or Fallback

```meson
# ✅ Good: Comment explains the strategy
# Try pkg-config first (some systems provide Cocoa.pc)
# Fall back to framework linking if not found
cocoa_dep = dependency('Cocoa', required: false, method: 'pkg-config')
if not cocoa_dep.found()
  cocoa_dep = declare_dependency(link_args: ['-framework', 'Cocoa'])
endif
```

### 4. Test on Multiple Configurations

```bash
# Test with default Homebrew path
meson setup build

# Test with custom path
PKG_CONFIG_PATH=/custom/path meson setup build2

# Test after package manager update
brew upgrade gstreamer
meson setup build3 --wipe
```

## Environment Integration

### CI/CD Integration (GitHub Actions)

```yaml
- name: Setup pkg-config path
  run: echo "PKG_CONFIG_PATH=/opt/homebrew/lib/pkgconfig" >> $GITHUB_ENV

- name: Build
  run: |
    meson setup build
    meson compile -C build
```

### Local Development

```bash
# Add to ~/.zshrc or ~/.bashrc
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"

# Then use normally
meson setup build
meson compile -C build
```

## References

- **pkg-config Manual**: https://man.archlinux.org/man/pkg-config.1
- **GStreamer Build Guide**: https://gstreamer.freedesktop.org/documentation/frequently-asked-questions/index.html?gi-language=c#how-do-i-build-the-gst-plugins
- **Meson Dependencies**: https://mesonbuild.com/Dependencies.html
- **macOS Frameworks**: https://developer.apple.com/documentation/

---

**Document Version**: 1.0
**Last Updated**: January 27, 2026
**Status**: Informational (Architecture Guide)
