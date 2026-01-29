/**
 * @file error_dialog.m
 * @brief macOS native error dialog implementation using NSAlert
 *
 * Implements error dialog display using native Cocoa NSAlert framework.
 * This provides user-friendly error messages for fatal errors during
 * application initialization.
 */

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#include "error_dialog.h"
#include "../utils/logging.h"

/**
 * Display a native macOS error dialog using NSAlert
 *
 * Creates and displays a modal NSAlert with the specified error information.
 *
 * @param dialog_type Type of error (used for logging)
 * @param title Alert title/heading
 * @param message Alert message body
 * @return TRUE if dialog was displayed
 */
gboolean error_dialog_show(ErrorDialogType dialog_type, const gchar *title,
                           const gchar *message) {
  if (!title || !message) {
    LOG_ERROR("error_dialog_show: Invalid title or message");
    return FALSE;
  }

  LOG_DEBUG("Displaying error dialog: type=%d, title=%s", dialog_type, title);

  /* Convert C strings to NSStrings */
  NSString *ns_title = [NSString stringWithUTF8String:title];
  NSString *ns_message = [NSString stringWithUTF8String:message];

  if (!ns_title || !ns_message) {
    LOG_ERROR("error_dialog_show: Failed to convert strings to NSString");
    return FALSE;
  }

  /* Ensure we're on the main thread (required for GUI operations) */
  if (![NSThread isMainThread]) {
    LOG_DEBUG("error_dialog_show: Dispatching to main thread");
    __block gboolean result = FALSE;
    dispatch_sync(dispatch_get_main_queue(), ^{
      NSAlert *alert = [[NSAlert alloc] init];
      [alert setMessageText:ns_title];
      [alert setInformativeText:ns_message];
      [alert setAlertStyle:NSAlertStyleCritical];
      [alert addButtonWithTitle:@"OK"];

      [alert runModal];
      [alert release];
      result = TRUE;
    });
    return result;
  }

  /* Create and display the alert on the main thread */
  NSAlert *alert = [[NSAlert alloc] init];
  [alert setMessageText:ns_title];
  [alert setInformativeText:ns_message];
  [alert setAlertStyle:NSAlertStyleCritical];
  [alert addButtonWithTitle:@"OK"];

  NSInteger result = [alert runModal];
  [alert release];

  LOG_DEBUG("error_dialog_show: Dialog dismissed with result %ld", (long)result);
  return TRUE;
}

/**
 * Display camera permission denied error
 *
 * Shows a user-friendly error message explaining that camera access
 * was denied and how to enable it in System Settings.
 */
gboolean error_dialog_show_camera_permission_denied(void) {
  LOG_INFO("Displaying camera permission denied dialog");

  NSString *title = @"Camera Access Denied";
  NSString *message = @"Video Looper was denied access to your camera.\n\n"
                      @"To use Video Looper, you must grant camera permission:\n\n"
                      @"1. Open System Settings\n"
                      @"2. Go to Privacy & Security → Camera\n"
                      @"3. Enable Video Looper in the camera access list\n"
                      @"4. Restart Video Looper\n\n"
                      @"Application will now exit.";

  /* Ensure we're on the main thread (required for GUI operations) */
  if (![NSThread isMainThread]) {
    LOG_DEBUG("error_dialog_show_camera_permission_denied: Dispatching to main thread");
    dispatch_sync(dispatch_get_main_queue(), ^{
      NSAlert *alert = [[NSAlert alloc] init];
      [alert setMessageText:title];
      [alert setInformativeText:message];
      [alert setAlertStyle:NSAlertStyleCritical];
      [alert addButtonWithTitle:@"OK"];
      [alert runModal];
      [alert release];
    });
    return TRUE;
  }

  NSAlert *alert = [[NSAlert alloc] init];
  [alert setMessageText:title];
  [alert setInformativeText:message];
  [alert setAlertStyle:NSAlertStyleCritical];
  [alert addButtonWithTitle:@"OK"];

  [alert runModal];
  [alert release];

  return TRUE;
}

/**
 * Display camera not found error
 *
 * Shows an error message when no built-in camera is detected on the system.
 */
gboolean error_dialog_show_camera_not_found(void) {
  LOG_INFO("Displaying camera not found dialog");

  NSString *title = @"No Camera Found";
  NSString *message =
      @"Video Looper could not detect a built-in camera on this Mac.\n\n"
      @"Your Mac may not have a built-in camera, or the camera may be "
      @"unavailable.\n\n"
      @"Please check:\n"
      @"1. Your Mac has a functional camera\n"
      @"2. The camera is not in use by another application\n"
      @"3. Try restarting your computer\n\n"
      @"Application will now exit.";

  /* Ensure we're on the main thread (required for GUI operations) */
  if (![NSThread isMainThread]) {
    LOG_DEBUG("error_dialog_show_camera_not_found: Dispatching to main thread");
    dispatch_sync(dispatch_get_main_queue(), ^{
      NSAlert *alert = [[NSAlert alloc] init];
      [alert setMessageText:title];
      [alert setInformativeText:message];
      [alert setAlertStyle:NSAlertStyleCritical];
      [alert addButtonWithTitle:@"OK"];
      [alert runModal];
      [alert release];
    });
    return TRUE;
  }

  NSAlert *alert = [[NSAlert alloc] init];
  [alert setMessageText:title];
  [alert setInformativeText:message];
  [alert setAlertStyle:NSAlertStyleCritical];
  [alert addButtonWithTitle:@"OK"];

  [alert runModal];
  [alert release];

  return TRUE;
}

/**
 * Display GStreamer initialization failed error
 *
 * Shows an error message when GStreamer fails to initialize.
 * Optionally includes a reason message for debugging.
 */
gboolean error_dialog_show_gstreamer_init_failed(const gchar *reason) {
  LOG_INFO("Displaying GStreamer init failed dialog");

  NSString *title = @"GStreamer Initialization Failed";
  NSString *message;

  if (reason) {
    message = [NSString
        stringWithFormat:
            @"Video Looper failed to initialize the GStreamer media library.\n\n"
            @"Reason: %s\n\n"
            @"This may indicate:\n"
            @"1. GStreamer is not properly installed\n"
            @"2. Required GStreamer plugins are missing\n"
            @"3. System configuration issue\n\n"
            @"Please reinstall GStreamer or contact support.\n\n"
            @"Application will now exit.",
            reason];
  } else {
    message = @"Video Looper failed to initialize the GStreamer media library.\n\n"
              @"This may indicate:\n"
              @"1. GStreamer is not properly installed\n"
              @"2. Required GStreamer plugins are missing\n"
              @"3. System configuration issue\n\n"
              @"Please reinstall GStreamer or contact support.\n\n"
              @"Application will now exit.";
  }

  /* Ensure we're on the main thread (required for GUI operations) */
  if (![NSThread isMainThread]) {
    LOG_DEBUG("error_dialog_show_gstreamer_init_failed: Dispatching to main thread");
    dispatch_sync(dispatch_get_main_queue(), ^{
      NSAlert *alert = [[NSAlert alloc] init];
      [alert setMessageText:title];
      [alert setInformativeText:message];
      [alert setAlertStyle:NSAlertStyleCritical];
      [alert addButtonWithTitle:@"OK"];
      [alert runModal];
      [alert release];
    });
    return TRUE;
  }

  NSAlert *alert = [[NSAlert alloc] init];
  [alert setMessageText:title];
  [alert setInformativeText:message];
  [alert setAlertStyle:NSAlertStyleCritical];
  [alert addButtonWithTitle:@"OK"];

  [alert runModal];
  [alert release];

  return TRUE;
}

/**
 * Display a generic fatal error dialog
 *
 * Shows a generic error dialog with custom title and message.
 */
gboolean error_dialog_show_generic(const gchar *title, const gchar *message) {
  return error_dialog_show(ERROR_DIALOG_GENERIC_ERROR, title, message);
}
