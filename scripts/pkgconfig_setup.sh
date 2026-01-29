#!/bin/bash
#
# pkg-config Setup and Validation Script for Video Looper
#
# This script:
# 1. Verifies pkg-config is installed
# 2. Configures PKG_CONFIG_PATH to include framework definitions
# 3. Validates all required dependencies are discoverable
# 4. Displays dependency information
#
# Usage: ./scripts/pkgconfig_setup.sh
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( dirname "$SCRIPT_DIR" )"
PKGCONFIG_DIR="$PROJECT_ROOT/pkgconfig"

echo -e "${BLUE}=========================================="
echo "pkg-config Setup for Video Looper"
echo "==========================================${NC}"
echo ""

# ============================================================================
# 1. Verify pkg-config is installed
# ============================================================================
echo -e "${BLUE}[1/4] Checking pkg-config installation...${NC}"
if ! command -v pkg-config &> /dev/null; then
    echo -e "${RED}ERROR: pkg-config not found!${NC}"
    echo "Please install pkg-config:"
    echo "  brew install pkg-config"
    exit 1
fi

PKG_CONFIG_VERSION=$(pkg-config --version)
echo -e "${GREEN}✓${NC} pkg-config found: $PKG_CONFIG_VERSION"
echo ""

# ============================================================================
# 2. Verify custom .pc files exist
# ============================================================================
echo -e "${BLUE}[2/4] Checking custom .pc files...${NC}"
REQUIRED_PC_FILES=(
    "Cocoa.pc"
    "AVFoundation.pc"
    "CoreFoundation.pc"
    "CoreMedia.pc"
    "CoreVideo.pc"
)

MISSING_FILES=0
for pc_file in "${REQUIRED_PC_FILES[@]}"; do
    if [ -f "$PKGCONFIG_DIR/$pc_file" ]; then
        echo -e "${GREEN}✓${NC} $pc_file found"
    else
        echo -e "${RED}✗${NC} $pc_file NOT found"
        MISSING_FILES=$((MISSING_FILES + 1))
    fi
done

if [ $MISSING_FILES -gt 0 ]; then
    echo -e "${RED}ERROR: $MISSING_FILES .pc file(s) missing!${NC}"
    exit 1
fi
echo ""

# ============================================================================
# 3. Configure PKG_CONFIG_PATH and test discovery
# ============================================================================
echo -e "${BLUE}[3/4] Testing dependency discovery...${NC}"
export PKG_CONFIG_PATH="$PKGCONFIG_DIR:$PKG_CONFIG_PATH"

REQUIRED_DEPS=(
    "gstreamer-1.0"
    "glib-2.0"
    "gobject-2.0"
    "gstreamer-video-1.0"
    "gstreamer-gl-1.0"
    "Cocoa"
    "AVFoundation"
    "CoreFoundation"
    "CoreMedia"
    "CoreVideo"
)

FAILED_DEPS=0
for dep in "${REQUIRED_DEPS[@]}"; do
    if pkg-config --exists "$dep" 2>/dev/null; then
        VERSION=$(pkg-config --modversion "$dep" 2>/dev/null)
        echo -e "${GREEN}✓${NC} $dep found (version: $VERSION)"
    else
        echo -e "${RED}✗${NC} $dep NOT found${NC}"
        FAILED_DEPS=$((FAILED_DEPS + 1))
    fi
done

if [ $FAILED_DEPS -gt 0 ]; then
    echo -e "${RED}ERROR: $FAILED_DEPS dependency(ies) not discoverable!${NC}"
    echo ""
    echo "Debug info:"
    echo "PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
    exit 1
fi
echo ""

# ============================================================================
# 4. Display dependency information
# ============================================================================
echo -e "${BLUE}[4/4] Dependency Information${NC}"
echo ""
echo "GStreamer Dependencies:"
for dep in "gstreamer-1.0" "glib-2.0" "gobject-2.0" "gstreamer-video-1.0" "gstreamer-gl-1.0"; do
    VERSION=$(pkg-config --modversion "$dep")
    echo "  • $dep: $VERSION"
done

echo ""
echo "macOS Framework Dependencies:"
for dep in "Cocoa" "AVFoundation" "CoreFoundation" "CoreMedia" "CoreVideo"; do
    if pkg-config --exists "$dep"; then
        CFLAGS=$(pkg-config --cflags "$dep")
        LIBS=$(pkg-config --libs "$dep")
        echo "  • $dep"
        echo "    Cflags: $CFLAGS"
        echo "    Libs: $LIBS"
    fi
done

echo ""
echo -e "${BLUE}=========================================="
echo "pkg-config Setup Complete!"
echo "==========================================${NC}"
echo ""
echo "To build the project with proper pkg-config configuration:"
echo "  ./scripts/build.sh"
echo ""
echo "To permanently set PKG_CONFIG_PATH in your shell:"
echo "  export PKG_CONFIG_PATH='$PKGCONFIG_DIR:\$PKG_CONFIG_PATH'"
echo ""
