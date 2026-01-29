# Composite Caps Element Implementation

## Overview

The `composite_caps` element is a critical component in the Video Looper pipeline that performs format conversion between the videomixer output and the osxvideosink renderer. This document describes the implementation, design decisions, and integration points.

## Architecture

### Pipeline Position

```
videomixer (10-cell grid output)
    ↓ (raw video from GPU composition)
composite_caps (format conversion capsfilter)
    ↓ (negotiated format suitable for osxvideosink)
osxvideosink (Cocoa window rendering)
```

### Purpose

The composite_caps element serves three key functions:

1. **Format Specification**: Declares the expected video format from the videomixer (dimensions, color space)
2. **Format Negotiation**: Negotiates compatible formats with downstream osxvideosink
3. **Frame Rate Control**: Ensures 120 fps target framerate is communicated through the pipeline

## Implementation Details

### Module Structure

**Files:**
- `src/gstreamer/composite_caps.h` - Public interface
- `src/gstreamer/composite_caps.c` - Implementation

**Core Function:**
```c
gboolean composite_caps_configure(GstElement *caps_element,
                                   gint grid_width,
                                   gint grid_height,
                                   gint framerate_num,
                                   gint framerate_den);
```

### Capabilities Configuration

The composite_caps element is configured with the following specification:

**Primary Format: BGRx**
- Byte order: Blue-Green-Red-X (padding byte for alignment)
- Selection rationale:
  - Native macOS Metal texture format
  - Optimal for GPU-to-GPU transfer without conversion
  - Direct compatibility with osxvideosink
  - Eliminates CPU-side format conversion overhead

**Fallback Format: RGB**
- Used if BGRx negotiation fails downstream
- Provides format flexibility for compatibility
- May require CPU-side conversion in osxvideosink

**Resolution**
- Width: 3200 pixels (10 cells × 320 pixels per cell)
- Height: Determined by camera aspect ratio (negotiated upstream)
- Fixed grid layout for consistent display

**Frame Rate**
- Default: 120/1 (120 fps)
- Configurable via parameters
- Controls pipeline synchronization and rendering speed
- Target for smooth playback across all 10 cells

### Configuration Algorithm

1. **Validate Input Parameters**
   - caps_element: Must not be NULL
   - Dimensions: Both width and height must be > 0
   - Frame rate: Both numerator and denominator must be > 0

2. **Create GStreamer Caps**
   - Build primary caps structure with BGRx format
   - Add alternative RGB format for fallback
   - Attach width, height, and framerate specifications

3. **Apply to Element**
   - Set the caps on the capsfilter element via GObject property
   - GStreamer's negotiation will select compatible format

4. **Error Handling**
   - Propagate validation errors with descriptive messages
   - Log successful configuration

### Integration Points

#### In pipeline_builder.c

```c
// Create composite capsfilter for format conversion
p->composite_caps = gst_element_factory_make("capsfilter", "composite-caps");

// Configure with grid parameters
composite_caps_configure(p->composite_caps,
                        COMPOSITE_GRID_WIDTH,      // 3200
                        COMPOSITE_GRID_HEIGHT,     // 1080 (negotiated)
                        TARGET_FRAMERATE_NUM,      // 120
                        TARGET_FRAMERATE_DEN);     // 1

// Link into pipeline
gst_element_link_many(p->videomixer, p->composite_caps, p->osxvideosink, NULL);
```

## Design Rationale

### Why BGRx?

On macOS with Metal/OpenGL rendering:
- BGRx is the native texture format for Metal on Apple Silicon
- Avoids runtime format conversion in osxvideosink
- Maintains GPU affinity (frames stay on GPU)
- Optimal performance for 120 fps target

### Why Capsfilter?

Compared to direct element linking:
- **Explicit**: Makes format requirements clear and debuggable
- **Negotiation**: Allows GStreamer to find compatible paths
- **Diagnostics**: Can introspect and verify format throughout pipeline
- **Flexibility**: Easy to modify format requirements during development

### Why Dual Format?

- **Robustness**: If BGRx not available in some configurations, RGB fallback works
- **Compatibility**: Handles different GStreamer plugin versions gracefully
- **Future-proof**: Allows easy addition of more formats if needed

## Usage

### Basic Configuration

```c
GstElement *composite_caps = gst_element_factory_make("capsfilter", "composite-caps");

gboolean success = composite_caps_configure(
    composite_caps,
    3200,  // grid_width (10 cells × 320px)
    1080,  // grid_height (aspect ratio-dependent)
    120,   // framerate numerator
    1      // framerate denominator (→ 120/1 fps)
);

if (!success) {
    // Handle error
    gst_object_unref(composite_caps);
    return NULL;
}
```

### Error Handling

The function returns FALSE and logs errors for:
- NULL element pointer
- Invalid dimensions (width or height ≤ 0)
- Invalid frame rate (numerator or denominator ≤ 0)
- Negative values for any parameter

Calling code should:
1. Check return value
2. Examine stderr for detailed error messages
3. Clean up resources if configuration fails

## Frame Rate Management

The composite_caps element communicates the 120 fps target through the pipeline:

1. **Caps String**: `framerate=(fraction)120/1`
2. **Clock Sync**: osxvideosink uses this to synchronize rendering
3. **Pipeline Clock**: GStreamer's clock mechanisms respect the specified rate
4. **Playback Loop**: Actual frame delivery synchronized by videomixer and playback bins

The 120 fps specification ensures:
- Smooth palindrome playback across all cells
- Consistent rendering performance
- Proper audio synchronization (if audio added in future)

## Testing

### Unit Tests

File: `test/unit/test_composite_caps.c`

Tests cover:
1. Valid configuration with standard parameters
2. NULL element error handling
3. Invalid width/height handling
4. Invalid frame rate handling
5. BGRx format verification in output caps

All tests pass successfully.

### Integration Testing

The composite_caps element is integrated into the full pipeline and tested via:
- `test/integration/test_gstreamer_pipeline.c` - Pipeline construction
- `test/integration/test_osxvideosink_integration.c` - Window rendering verification

## Performance Characteristics

- **CPU Impact**: Minimal (capsfilter is passive element)
- **GPU Impact**: None (format is declared, not converted)
- **Latency**: Negligible (no processing overhead)
- **Memory**: None (element has no buffers)

The capsfilter element is purely informational - it doesn't process data, only negotiates format.

## Debugging

### Inspect Negotiated Caps

```bash
# During pipeline operation, examine GStreamer debug output:
GST_DEBUG=3 ./build/video-looper
```

Look for:
- "composite_caps: Configured composite caps for..."
- Format negotiation messages from osxvideosink
- Any format mismatch warnings

### Verify Capabilities

```c
GstCaps *caps = NULL;
g_object_get(composite_caps, "caps", &caps, NULL);
gchar *caps_str = gst_caps_to_string(caps);
fprintf(stdout, "Composite caps: %s\n", caps_str);
g_free(caps_str);
gst_caps_unref(caps);
```

## Related Components

- **videomixer**: Produces composite 10-cell grid output
- **osxvideosink**: Consumes formatted video for Cocoa rendering
- **pipeline_builder**: Creates and links composite_caps into pipeline

## Future Enhancements

Potential improvements (out of current scope):

1. **Dynamic Format Selection**: Detect best format at runtime
2. **Format Negotiation Logging**: More verbose debugging info
3. **Frame Rate Adaptation**: Adjust fps based on system performance
4. **Alternative Formats**: YUV420P support for better compression

## References

### GStreamer Documentation
- [GStreamer Caps](https://gstreamer.freedesktop.org/documentation/gstreamer/gstcaps.html)
- [Capsfilter Element](https://gstreamer.freedesktop.org/documentation/coreelements/capsfilter.html)
- [videomixer Documentation](https://gstreamer.freedesktop.org/documentation/compositor/videomixer.html)

### macOS-Specific
- [Metal Texture Formats](https://developer.apple.com/documentation/metal/mtlpixelformat)
- [osxvideosink Plugin](https://gstreamer.freedesktop.org/documentation/osxvideo/osxvideosink.html)

## Code Example: Full Pipeline Integration

See `src/gstreamer/pipeline_builder.c` for complete integration:

```c
// In pipeline_create():
1. Create composite_caps element
2. Call composite_caps_configure() with grid dimensions
3. Link videomixer → composite_caps → osxvideosink
4. Handle errors appropriately
```

---

**Document Version**: 1.0
**Last Updated**: January 27, 2026
**Status**: Complete
