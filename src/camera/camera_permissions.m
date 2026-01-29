/**
 * @file camera_permissions.m
 * @brief Objective-C wrapper for macOS camera permissions using AVFoundation
 *
 * Implements AVFoundation-based camera permission requests for macOS.
 * This file uses Objective-C to interface with native macOS camera APIs.
 */

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#include "camera_source.h"

/**
 * Request camera permission from the user via AVFoundation
 *
 * Uses AVCaptureDevice authorization status APIs to check and request
 * camera permissions on macOS.
 *
 * @return Permission status (GRANTED, DENIED, or NOT_DETERMINED)
 */
CameraPermissionStatus camera_request_permission_objc(void) {
  /* Check if AVFoundation is available and get authorization status */
  AVAuthorizationStatus auth_status =
      [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];

  switch (auth_status) {
    case AVAuthorizationStatusAuthorized:
      /* User has previously granted permission */
      return CAMERA_PERMISSION_GRANTED;

    case AVAuthorizationStatusDenied:
      /* User has denied camera access */
      return CAMERA_PERMISSION_DENIED;

    case AVAuthorizationStatusRestricted:
      /* Camera access is restricted by parental controls or similar */
      return CAMERA_PERMISSION_DENIED;

    case AVAuthorizationStatusNotDetermined: {
      /* Permission not yet requested - request it now */
      __block CameraPermissionStatus result =
          CAMERA_PERMISSION_NOT_DETERMINED;

      /* Use dispatch_semaphore to wait for async request completion */
      dispatch_semaphore_t sem = dispatch_semaphore_create(0);

      [AVCaptureDevice
          requestAccessForMediaType:AVMediaTypeVideo
                  completionHandler:^(BOOL granted) {
                    result = granted ? CAMERA_PERMISSION_GRANTED
                                     : CAMERA_PERMISSION_DENIED;
                    dispatch_semaphore_signal(sem);
                  }];

      /* Wait for permission dialog to be answered (max 30 seconds) */
      dispatch_time_t timeout =
          dispatch_time(DISPATCH_TIME_NOW, 30 * NSEC_PER_SEC);
      if (dispatch_semaphore_wait(sem, timeout) == 0) {
        return result;
      } else {
        /* Timeout - assume denied */
        return CAMERA_PERMISSION_DENIED;
      }
    }

    default:
      return CAMERA_PERMISSION_NOT_DETERMINED;
  }
}

/**
 * Detect available camera devices on the system using modern AVFoundation APIs
 *
 * Uses AVCaptureDeviceDiscoverySession to find available cameras.
 * Currently returns the built-in front-facing camera, but could be extended
 * for multi-camera support.
 *
 * @return Reference to the built-in camera device, or nil if not found
 */
AVCaptureDevice *find_built_in_camera(void) {
  /* Use modern AVCaptureDeviceDiscoverySession API (macOS 10.15+) */
  AVCaptureDeviceDiscoverySession *discovery_session =
      [AVCaptureDeviceDiscoverySession
          discoverySessionWithDeviceTypes:@[ AVCaptureDeviceTypeBuiltInWideAngleCamera ]
                                mediaType:AVMediaTypeVideo
                                 position:AVCaptureDevicePositionFront];

  NSArray<AVCaptureDevice *> *devices = [discovery_session devices];

  if (devices && [devices count] > 0) {
    /* Return the first available built-in camera */
    return [devices objectAtIndex:0];
  }

  /* Fall back to searching all available devices */
  discovery_session =
      [AVCaptureDeviceDiscoverySession
          discoverySessionWithDeviceTypes:@[ AVCaptureDeviceTypeBuiltInWideAngleCamera ]
                                mediaType:AVMediaTypeVideo
                                 position:AVCaptureDevicePositionUnspecified];

  devices = [discovery_session devices];

  if (devices && [devices count] > 0) {
    return [devices objectAtIndex:0];
  }

  return nil;
}

/**
 * Query camera capabilities and supported formats
 *
 * Enumerates supported video formats from the camera device.
 * Returns information about supported resolutions and frame rates.
 *
 * @param device Camera device to query
 * @return Array of supported video formats (caller must release)
 */
NSArray *get_camera_formats(AVCaptureDevice *device) {
  if (!device) {
    return nil;
  }

  NSMutableArray *formats = [NSMutableArray array];

  for (AVCaptureDeviceFormat *format in [device formats]) {
    CMMediaType mediaType = CMFormatDescriptionGetMediaType(
        [format formatDescription]);

    if (mediaType == kCMMediaType_Video) {
      [formats addObject:format];
    }
  }

  return formats;
}

/**
 * Validate and negotiate camera format with device hardware
 *
 * Attempts to validate that the camera can support the requested format.
 * Tries formats in order of preference: 1920x1080 first, then 1280x720.
 *
 * @param out_width Pointer to store negotiated width (output parameter)
 * @param out_height Pointer to store negotiated height (output parameter)
 * @param out_framerate Pointer to store negotiated framerate (output parameter)
 * @return 0 on success (format negotiated), -1 on failure (no compatible format)
 */
int camera_negotiate_format_objc(int *out_width, int *out_height, int *out_framerate) {
  if (!out_width || !out_height || !out_framerate) {
    return -1;
  }

  /* Find the built-in camera device */
  AVCaptureDevice *device = find_built_in_camera();
  if (!device) {
    NSLog(@"ERROR: No built-in camera device found");
    return -1;
  }

  NSLog(@"INFO: Found camera device: %@", [device localizedName]);

  /* Query supported formats from the device */
  NSArray *formats = get_camera_formats(device);
  if (!formats || [formats count] == 0) {
    NSLog(@"ERROR: Camera device has no video formats available");
    return -1;
  }

  /* Format candidates in order of preference */
  int width_candidates[] = {1920, 1280, 0};
  int height_candidates[] = {1080, 720, 0};

  /* Try each candidate format */
  for (int i = 0; width_candidates[i] > 0; i++) {
    int req_width = width_candidates[i];
    int req_height = height_candidates[i];

    NSLog(@"DEBUG: Checking format: %dx%d @ 30fps", req_width, req_height);

    /* Search for a format matching the requested resolution */
    for (AVCaptureDeviceFormat *format in formats) {
      CMVideoDimensions dims = CMVideoFormatDescriptionGetDimensions(
          [format formatDescription]);

      /* Check if this format matches our width and height */
      if (dims.width == req_width && dims.height == req_height) {
        NSLog(@"INFO: Found compatible format: %dx%d", dims.width, dims.height);

        /* Return the negotiated values */
        *out_width = req_width;
        *out_height = req_height;
        *out_framerate = 30;  /* macOS built-in cameras typically support 30fps */

        return 0;  /* Success */
      }
    }

    NSLog(@"WARNING: Format %dx%d not available, trying fallback", req_width, req_height);
  }

  /* No compatible format found */
  NSLog(@"ERROR: No compatible camera format found");
  return -1;
}
