/**
 * @file window.m
 * @brief OS X window management (Objective-C implementation)
 *
 * Implements Cocoa NSWindow creation, GStreamer osxvideosink integration,
 * and Metal/OpenGL rendering context coordination.
 */

#import <Cocoa/Cocoa.h>
#include "window.h"
#include "../utils/logging.h"
#include "../input/keyboard_handler.h"
#include <stdlib.h>
#include <string.h>
#include <gst/video/video.h>

/* ============================================================================
 * Forward declarations for Objective-C window delegate
 * ========================================================================= */

/**
 * Custom NSWindow delegate for handling window events (close, resize, etc.)
 */
@interface VideoLooperWindowDelegate : NSObject<NSWindowDelegate>
{
  OSXWindow *window_context;
}
- (instancetype)initWithWindowContext:(OSXWindow *)context;
- (void)windowWillClose:(NSNotification *)notification;
@end

/* Forward declaration for internal window creation function */
static gboolean window_create_nswindow_internal(OSXWindow *win, CGFloat width, CGFloat height);

/**
 * Custom NSView for Metal/OpenGL rendering integration
 */
@interface VideoLooperView : NSView
{
  OSXWindow *window_context;
}
- (instancetype)initWithFrame:(NSRect)frame context:(OSXWindow *)context;
@end

/* ============================================================================
 * NSView implementation
 * ========================================================================= */

@implementation VideoLooperView

- (instancetype)initWithFrame:(NSRect)frame context:(OSXWindow *)context {
  self = [super initWithFrame:frame];
  if (self) {
    window_context = context;
    // DO NOT set wantsLayer - osxvideosink manages its own rendering
    // DO NOT implement drawRect - osxvideosink renders directly via GstVideoOverlay
  }
  return self;
}

- (BOOL)isOpaque {
  // View is opaque - no transparency needed
  return YES;
}

- (BOOL)acceptsFirstResponder {
  // Accept keyboard events
  return YES;
}

- (void)keyDown:(NSEvent *)event {
  // Forward key press to keyboard handler
  unsigned short keyCode = [event keyCode];
  BOOL isShifted = (event.modifierFlags & NSEventModifierFlagShift) != 0;
  LOG_DEBUG("keyDown: keyCode=%u", keyCode);
  keyboard_on_event((int)keyCode, isShifted, TRUE);
}

- (void)keyUp:(NSEvent *)event {
  // Forward key release to keyboard handler
  unsigned short keyCode = [event keyCode];
  BOOL isShifted = (event.modifierFlags & NSEventModifierFlagShift) != 0;
  LOG_DEBUG("keyUp: keyCode=%u", keyCode);
  keyboard_on_event((int)keyCode, isShifted, FALSE);
}

@end

/* ============================================================================
 * NSWindow delegate implementation
 * ========================================================================= */

@implementation VideoLooperWindowDelegate

- (instancetype)initWithWindowContext:(OSXWindow *)context {
  self = [super init];
  if (self) {
    window_context = context;
  }
  return self;
}

- (void)windowWillClose:(NSNotification *)notification {
  LOG_INFO("Window close event detected");
  // Signal the application to shut down gracefully
  // This is handled by the main event loop which can check window_is_visible()
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
  LOG_DEBUG("Window close requested by user");
  return YES;
}

@end

/* ============================================================================
 * Window module C wrapper functions (exported from window.c)
 * ========================================================================= */

/**
 * Calculate window dimensions based on aspect ratio
 *
 * @param grid_cols Number of grid columns (typically 11)
 * @param grid_rows Number of grid rows (typically 2)
 * @param aspect_ratio Video aspect ratio (width / height)
 * @param out_width Pointer to receive calculated width
 * @param out_height Pointer to receive calculated height
 */
void window_calculate_dimensions(
    guint grid_cols,
    guint grid_rows,
    CGFloat aspect_ratio,
    CGFloat *out_width,
    CGFloat *out_height) {
  const CGFloat CELL_WIDTH = 320.0;  // pixels per cell

  // Use 16:9 aspect ratio as default if not set
  if (aspect_ratio <= 0) {
    aspect_ratio = 16.0 / 9.0;  // 1.777...
  }

  *out_width = CELL_WIDTH * grid_cols;
  *out_height = (CELL_WIDTH / aspect_ratio) * grid_rows;
}

/**
 * Create and configure osxvideosink element
 *
 * @return Newly allocated GStreamer osxvideosink element
 */
GstElement *window_create_osxvideosink(void) {
  GstElement *sink = gst_element_factory_make("osxvideosink", "videosink");
  if (!sink) {
    LOG_ERROR("Failed to create osxvideosink element");
    return NULL;
  }

  // Configure osxvideosink properties
  g_object_set(G_OBJECT(sink),
               "sync", TRUE,                // Synchronize to clock for proper framerate
               "force-aspect-ratio", FALSE, // Use exact dimensions (window handles aspect ratio)
               NULL);

  LOG_DEBUG("osxvideosink element created and configured");
  return sink;
}

/**
 * Internal: Create NSWindow and VideoLooperView
 *
 * @param win Window context to initialize
 * @param width Window width in pixels
 * @param height Window height in pixels
 * @return TRUE on success, FALSE on failure
 */
gboolean window_create_nswindow(OSXWindow *win, CGFloat width, CGFloat height) {
  // NSWindow MUST be created on the main thread
  // If called from secondary thread (via gst_macos_main), dispatch to main
  __block gboolean result = FALSE;

  if ([NSThread isMainThread]) {
    result = window_create_nswindow_internal(win, width, height);
  } else {
    dispatch_sync(dispatch_get_main_queue(), ^{
      result = window_create_nswindow_internal(win, width, height);
    });
  }

  return result;
}

/**
 * Internal: Create NSWindow (must be called on main thread)
 */
static gboolean window_create_nswindow_internal(OSXWindow *win, CGFloat width, CGFloat height) {
  @autoreleasepool {
    // Initialize NSApplication (required for macOS GUI applications)
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
    LOG_DEBUG("NSApplication initialized");

    // Create window style: titled, closable, miniaturizable, resizable
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                       NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;

    // Create window with calculated dimensions
    NSRect frame = NSMakeRect(0, 0, width, height);
    NSWindow *nswindow = [[NSWindow alloc] initWithContentRect:frame
                                                     styleMask:style
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO];

    if (!nswindow) {
      LOG_ERROR("Failed to create NSWindow");
      return FALSE;
    }

    // Configure window properties
    [nswindow setTitle:@"Video Looper"];
    [nswindow setAcceptsMouseMovedEvents:NO];  // We only care about keyboard
    [nswindow setReleasedWhenClosed:NO];       // Manual lifecycle management

    // Center window on screen
    [nswindow center];

    // Create video view for rendering
    NSView *video_view = [[VideoLooperView alloc] initWithFrame:frame context:win];
    if (!video_view) {
      LOG_ERROR("Failed to create VideoLooperView");
      [nswindow release];
      return FALSE;
    }

    // Set view as window's content
    [nswindow setContentView:video_view];
    [video_view release];  // Window retains the view

    // Create and set window delegate for event handling
    VideoLooperWindowDelegate *delegate = [[VideoLooperWindowDelegate alloc] initWithWindowContext:win];
    if (!delegate) {
      LOG_ERROR("Failed to create window delegate");
      [nswindow release];
      return FALSE;
    }
    [nswindow setDelegate:delegate];
    [delegate release];  // Window retains the delegate

    // Make window visible and set first responder for keyboard events
    [nswindow makeKeyAndOrderFront:nil];
    [nswindow makeFirstResponder:video_view];

    // Explicitly process pending events to ensure window appears
    // This is necessary when NSRunLoop is not yet running
    NSEvent *event;
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES])) {
      [NSApp sendEvent:event];
      [NSApp updateWindows];
    }

    LOG_DEBUG("NSWindow created: %p", nswindow);
    LOG_DEBUG("Video view created: %p", video_view);

    // Store opaque pointers
    win->nswindow = nswindow;
    win->video_view = video_view;
  }  // @autoreleasepool

  return TRUE;
}

/**
 * Internal: Update NSWindow frame to new dimensions
 *
 * @param win Window context
 * @param width New width
 * @param height New height
 */
void window_update_nswindow_frame(OSXWindow *win, CGFloat width, CGFloat height) {
  if ([NSThread isMainThread]) {
    @autoreleasepool {
      NSWindow *nswindow = (NSWindow *)win->nswindow;
      if (nswindow) {
        // Use setContentSize to set the content area size (excluding title bar)
        NSSize content_size = NSMakeSize(width, height);
        [nswindow setContentSize:content_size];
        LOG_DEBUG("Window content resized to %.0f x %.0f", width, height);
      }
    }
  } else {
    dispatch_async(dispatch_get_main_queue(), ^{
      @autoreleasepool {
        NSWindow *nswindow = (NSWindow *)win->nswindow;
        if (nswindow) {
          // Use setContentSize to set the content area size (excluding title bar)
          NSSize content_size = NSMakeSize(width, height);
          [nswindow setContentSize:content_size];
          LOG_DEBUG("Window content resized to %.0f x %.0f", width, height);
        }
      }
    });
  }
}

/**
 * Internal: Check if NSWindow is visible
 *
 * @param win Window context
 * @return TRUE if window is visible, FALSE otherwise
 */
gboolean window_check_nswindow_visible(OSXWindow *win) {
  gboolean visible = FALSE;
  @autoreleasepool {
    NSWindow *nswindow = (NSWindow *)win->nswindow;
    if (nswindow) {
      visible = [nswindow isVisible] ? TRUE : FALSE;
    }
  }
  return visible;
}

/**
 * Internal: Request redraw of the video view
 *
 * @param win Window context
 */
void window_request_nsview_redraw(OSXWindow *win) {
  @autoreleasepool {
    NSView *view = (NSView *)win->video_view;
    if (view) {
      [view setNeedsDisplay:YES];
    }
  }
}

/**
 * Internal: Release NSWindow and related resources
 *
 * @param win Window context
 */
void window_release_nswindow(OSXWindow *win) {
  if ([NSThread isMainThread]) {
    @autoreleasepool {
      NSWindow *nswindow = (NSWindow *)win->nswindow;
      if (nswindow) {
        [nswindow setDelegate:nil];
        [nswindow close];
        [nswindow release];
        LOG_DEBUG("NSWindow released");
        win->nswindow = NULL;
        win->video_view = NULL;
      }
    }
  } else {
    dispatch_sync(dispatch_get_main_queue(), ^{
      @autoreleasepool {
        NSWindow *nswindow = (NSWindow *)win->nswindow;
        if (nswindow) {
          [nswindow setDelegate:nil];
          [nswindow close];
          [nswindow release];
          LOG_DEBUG("NSWindow released");
          win->nswindow = NULL;
          win->video_view = NULL;
        }
      }
    });
  }
}
