# Video Rendering Debug and Fix Report

## Problem Summary

The application was building and running successfully:
- Window opened
- Camera light activated
- **But no video was rendered**

## Root Cause Analysis

### Issue #1: Missing NSRunLoop Integration
The application was using `g_main_loop_run()` directly on the main thread, but GStreamer's `osxvideosink` requires an NSApplication event loop (NSRunLoop) to be running on the main thread for video rendering on macOS.

**Evidence from logs:**
```
** WARNING **: An NSRunLoop needs to be running on the main thread to ensure correct behaviour on macOS. Use gst_macos_main() or call [NSApplication sharedApplication] in your code before using this element.
```

While `[NSApplication sharedApplication]` was being called during window creation, the NSRunLoop itself was not actively running because the application was using GLib's event loop instead of integrating with macOS's Cocoa event loop.

### Issue #2: NSWindow Creation on Wrong Thread
After fixing Issue #1 by using `gst_macos_main()`, a second issue emerged: NSWindow objects **must** be created on the main thread, but `gst_macos_main()` runs the application logic on a secondary thread (by design, to allow the NSRunLoop to run on the main thread).

**Error encountered:**
```
*** Terminating app due to uncaught exception 'NSInternalInconsistencyException', reason: 'NSWindow should only be instantiated on the main thread!'
```

## Solutions Implemented

### Fix #1: Use gst_macos_main() (main.c)

**Changed:**
1. Added `#include <gst/gstmacos.h>` to include GStreamer's macOS integration
2. Renamed the original `main()` function to `app_main()` with signature matching `GstMainFunc`
3. Created new `main()` function that calls `gst_macos_main(app_main, argc, argv, NULL)`

**Result:**
- NSApplication now runs on the main thread with its event loop active
- Application logic runs on a secondary thread
- osxvideosink can properly render video frames

### Fix #2: Dispatch NSWindow Creation to Main Thread (window.m)

**Changed:**
1. Refactored `window_create_nswindow()` to check if it's running on the main thread
2. If not on main thread, use `dispatch_sync(dispatch_get_main_queue(), ^{...})` to dispatch NSWindow creation to the main thread
3. Extracted the actual window creation logic into `window_create_nswindow_internal()`
4. Fixed video_view reference to use `[nswindow contentView]` for proper memory management

**Result:**
- NSWindow is always created on the main thread, regardless of which thread calls the function
- No more crashes due to incorrect thread usage
- Proper Cocoa integration

## Code Changes

### src/main.c
```c
// Added include
#include <gst/gstmacos.h>

// Renamed main() to app_main()
int app_main(int argc, char *argv[], gpointer user_data)
{
    // ... existing main() logic ...
}

// New main() that uses gst_macos_main()
int main(int argc, char *argv[])
{
    return gst_macos_main(app_main, argc, argv, NULL);
}
```

### src/osx/window.m
```objc
gboolean window_create_nswindow(OSXWindow *win, CGFloat width, CGFloat height) {
    __block gboolean result = FALSE;

    if ([NSThread isMainThread]) {
        result = window_create_nswindow_internal(win, width, height);
    } else {
        LOG_DEBUG("Dispatching NSWindow creation to main thread");
        dispatch_sync(dispatch_get_main_queue(), ^{
            result = window_create_nswindow_internal(win, width, height);
        });
    }

    return result;
}

static gboolean window_create_nswindow_internal(OSXWindow *win, CGFloat width, CGFloat height) {
    // ... actual NSWindow creation logic ...

    // Fixed: Get view reference from window
    win->video_view = [nswindow contentView];
}
```

## Testing

To test the fix:

```bash
# Rebuild the application
meson compile -C builddir

# Run the application
./builddir/video-looper
```

**Expected behavior:**
1. Window should open
2. Camera light should turn on
3. **Live video feed should now be visible in the first cell of the grid**
4. No crashes or NSWindow thread errors
5. No NSRunLoop warnings

## Technical Notes

### Why gst_macos_main() is Required

GStreamer's macOS video sinks (like `osxvideosink`) use Cocoa APIs internally that require:
1. An NSApplication instance
2. An active NSRunLoop on the main thread
3. Proper Cocoa event processing

The `gst_macos_main()` function (introduced in GStreamer 1.22) handles this by:
1. Starting NSApplication on the main thread
2. Running your application code on a secondary thread
3. Ensuring proper event loop integration between GLib and Cocoa

### Thread Model After Fix

```
Main Thread (NSApplication/NSRunLoop):
├─ Cocoa event processing
├─ NSWindow management (dispatched via GCD)
└─ Video rendering (osxvideosink)

Secondary Thread (GLib main loop):
├─ GStreamer pipeline management
├─ Camera capture
├─ Application logic
└─ User input handling
```

## Related Files

- `src/main.c` - Application entry point
- `src/osx/window.m` - Window and video view management
- `src/osx/window.c` - Window API wrapper

## References

- GStreamer macOS Integration: `/opt/homebrew/Cellar/gstreamer/1.26.10/include/gstreamer-1.0/gst/gstmacos.h`
- Apple NSWindow Documentation: Must be created on main thread
- Apple Grand Central Dispatch (GCD): `dispatch_sync()` for main thread dispatch
