# Camera Error Handling Implementation (T-9.1)

## Overview

This document describes the comprehensive camera error handling system implemented for Task T-9.1, covering:
- Camera not found errors
- Permission denied errors
- Camera disconnection during session
- Reconnection logic and recovery

## Architecture

### Components

The camera error handling system consists of three main modules:

#### 1. Camera Error Handler (`camera_error_handler.h/.c`)
- **Purpose**: Central error handling for camera-related failures
- **Responsibilities**:
  - Classify camera errors (not found, permission denied, disconnected, format failed, etc.)
  - Track error state and recovery progress
  - Provide recoverable vs. fatal error classification
  - Coordinate error callbacks to application

**Key Types**:
- `CameraErrorType`: Enum of error categories
- `CameraState`: Camera operational state (uninitialized, initializing, ready, error, disconnected, shutdown)
- `CameraErrorInfo`: Error details including type, message, timestamp, retry count
- `CameraErrorHandler`: Context for error handling

**Key Functions**:
- `camera_error_handler_create()`: Initialize error handler
- `camera_error_handle_not_found()`: Handle missing camera (FATAL)
- `camera_error_handle_permission_denied()`: Handle permission errors (FATAL)
- `camera_error_handle_disconnected()`: Handle mid-session disconnect (RECOVERABLE)
- `camera_error_attempt_reconnection()`: Initiate recovery sequence
- `camera_error_is_accessible()`: Check if camera usable

#### 2. Camera Source Extensions (`camera_source.h/.c`)
- **Purpose**: Extend existing camera source with connectivity checking
- **Responsibilities**:
  - Check camera connection status
  - Reinitialize camera after reconnection
  - Handle permission re-verification

**New Functions**:
- `camera_source_is_connected()`: Verify camera is responsive
- `camera_source_reinitialize()`: Attempt reconnection sequence

#### 3. Camera Monitor (`camera_monitor.h/.c`)
- **Purpose**: Periodic health checking and disconnection detection
- **Responsibilities**:
  - Monitor camera element state continuously
  - Detect disconnection through failed health checks
  - Generate notifications when status changes
  - Track consecutive failure counts

**Key Features**:
- Runs on 500ms polling interval
- Requires 3 consecutive failures to report disconnection
- Detects state changes (healthy→disconnected, disconnected→healthy)
- Optional callback for health status changes

**Key Functions**:
- `camera_monitor_create()`: Start monitoring
- `camera_monitor_start()`: Enable periodic checks
- `camera_monitor_check_health()`: Force immediate health check
- `camera_monitor_is_healthy()`: Query current status

## Error Handling Flows

### 1. Camera Not Found (Initialization)

**When**: During camera initialization in Phase 2

**Flow**:
```
camera_source_init()
  → camera_request_permission()
  → camera_negotiate_format()
    → No compatible format found
    → LOG_ERROR("No compatible camera format found")
    → app_log_error(APP_ERROR_CAMERA_NOT_FOUND, ...)
    → return NULL

main()
  → Detects NULL camera source
  → Calls camera_error_handle_not_found(handler)
  → Logs error to console
  → Notifies application via callback
  → Application exits with code 1
```

**Recovery**: NONE (fatal error)

**User Experience**:
- Error message logged: "Built-in camera not detected on this computer"
- Application terminates gracefully
- User must restart with camera connected

---

### 2. Camera Permission Denied (Initialization)

**When**: During camera initialization, user denies permission in macOS dialog

**Flow**:
```
camera_request_permission()
  → camera_request_permission_objc()  [AVFoundation call]
  → User denies camera access in system dialog
  → Returns CAMERA_PERMISSION_DENIED

camera_source_init()
  → Detects CAMERA_PERMISSION_DENIED
  → LOG_ERROR("Camera permission denied by user")
  → app_log_error(APP_ERROR_CAMERA_PERMISSION_DENIED, ...)
  → free(cam); return NULL

main()
  → Detects NULL camera source
  → Calls camera_error_handle_permission_denied(handler)
  → Logs detailed instructions to console
  → Notifies application via callback
  → Application exits with code 1
```

**Recovery**: NONE (fatal error)

**User Experience**:
- Clear error message: "Camera access was denied. Please grant camera permission in System Preferences..."
- User can fix permission in macOS security settings
- Restart application to try again

---

### 3. Camera Disconnection During Session (Runtime)

**When**: Camera is removed, unplugged, or becomes unavailable during operation

**Detection Flow**:
```
GStreamer Pipeline Running
  ↓
camera_monitor runs on 500ms interval
  ↓
camera_monitor_check_health()
  ↓
gst_element_get_state(camera_element, ...) [FAILS]
  ↓
consecutive_failures++
  ↓
[After 3 consecutive failures]
  ↓
camera_monitor_is_healthy() → FALSE
  ↓
Invokes health_callback(FALSE, "Camera failed health check", ...)
  ↓
Pipeline message handler receives notification
```

**Recovery Attempt Flow**:
```
Pipeline Error Handler
  ↓
camera_error_handle_disconnected(handler)
  ↓
handler->current_state = CAMERA_STATE_DISCONNECTED
  ↓
camera_error_attempt_reconnection(handler)
  ↓
[Attempt 1-5]
  ├─ Wait 100-200ms
  ├─ Call camera_request_permission() [may be restored]
  ├─ Call camera_negotiate_format() [retry detection]
  └─ Re-link pipeline if successful
  ↓
[If all attempts fail]
  ↓
handler->current_state = CAMERA_STATE_ERROR
  ↓
app_log_error(APP_ERROR_CAMERA_DISCONNECTED, "After 5 attempts...")
```

**Recovery Status**:
- **Recoverable**: YES, up to 5 reconnection attempts
- **User can fix**: Yes - reconnect camera hardware and it will resume

**User Experience**:
- Immediate detection (within 1.5 seconds of first failure)
- Console shows: "Camera disconnected. Attempting to reconnect..."
- Live feed freezes on last frame (no new data from camera)
- Recording attempts become no-ops
- When camera is reconnected, application attempts to resume
- If recovery succeeds within 5 attempts: transparent recovery
- If recovery fails: "Camera reconnection failed after 5 attempts" error

---

### 4. Format Negotiation Error (Initialization)

**When**: Camera detected but doesn't support required formats

**Flow**:
```
camera_negotiate_format()
  ↓
Try 1920x1080 @ 30fps → FAIL
Try 1280x720 @ 30fps → FAIL
  ↓
No compatible formats remaining
  ↓
camera_error_handle_format_failed(handler)
  ↓
handler->is_recoverable = FALSE
  ↓
app_log_error(APP_ERROR_CAMERA_NOT_FOUND,
  "Camera may not support required video formats...")
  ↓
Application exits with code 1
```

**Recovery**: NONE (fatal)

**User Experience**:
- Error: "Failed to negotiate camera format. Camera may not support required video formats..."
- Application terminates
- User must use compatible Mac or check camera drivers

---

### 5. GStreamer Element Creation Error (Initialization)

**When**: GStreamer plugin missing (e.g., avfvideosrc not available)

**Flow**:
```
camera_source_create_element()
  ↓
gst_element_factory_make("avfvideosrc", ...) → NULL
  ↓
camera_error_handle_element_create_failed(handler, "avfvideosrc")
  ↓
LOG_ERROR("Failed to create GStreamer element: avfvideosrc")
  ↓
app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED,
  "Failed to create GStreamer element 'avfvideosrc'...")
  ↓
Application exits with code 2
```

**Recovery**: NONE (fatal)

**User Experience**:
- Error: "Failed to create GStreamer element 'avfvideosrc'. Required GStreamer plugin not installed."
- Application terminates
- User must install/reinstall GStreamer with AVF plugin

---

## Integration Points

### With GStreamer Pipeline

The error handler integrates with the pipeline's bus message handling:

```c
// In pipeline_builder.c bus watch handler:

case GST_MESSAGE_ERROR: {
    // Check if error is camera-related
    if (strstr(err->message, "camera") || strstr(err->message, "avfvideosrc")) {
        // Delegate to camera error handler
        camera_error_handle_disconnected(handler);
    } else {
        // Handle as generic pipeline error
        app_log_error(APP_ERROR_PIPELINE_STATE_CHANGE_FAILED, ...);
    }
}
```

### With Application Main Loop

In `main.c`:

```c
// During initialization:
CameraSource *camera = camera_source_init();
if (!camera) {
    camera_error_handle_not_found(error_handler);
    return 1;
}

// During event loop:
GstElement *camera_elem = camera_source_create_element(camera);
if (!camera_elem) {
    camera_error_handle_element_create_failed(error_handler, "avfvideosrc");
    return 2;
}

// Start monitoring:
CameraMonitor *monitor = camera_monitor_create(camera, camera_elem);
camera_monitor_set_callback(monitor, on_camera_health_changed, app_context);
camera_monitor_start(monitor);

// In shutdown:
camera_monitor_cleanup(monitor);
camera_error_handler_cleanup(error_handler);
```

## Error Code Mappings

| Error Type | App Error Code | Exit Code | Recoverable |
|------------|---|---|---|
| Not Found | `APP_ERROR_CAMERA_NOT_FOUND` (102) | 1 | No |
| Permission Denied | `APP_ERROR_CAMERA_PERMISSION_DENIED` (103) | 1 | No |
| Disconnected | `APP_ERROR_CAMERA_DISCONNECTED` (200) | - | Yes |
| Format Failed | `APP_ERROR_CAMERA_NOT_FOUND` (102) | 1 | No |
| Element Create Failed | `APP_ERROR_PIPELINE_BUILD_FAILED` (104) | 2 | No |

## Logging Strategy

All camera errors are logged at appropriate levels:

```
[2026-01-27 10:15:34.456] [ERROR] camera_source: Camera permission denied by user
[2026-01-27 10:15:34.456] [ERROR] camera_source: Camera source initialized failed
[2026-01-27 10:15:34.456] [ERROR] camera_error_handler: FATAL: Camera not found

[2026-01-27 10:16:45.123] [WARNING] camera_monitor: Camera health check failed (failure count: 1/3)
[2026-01-27 10:16:45.623] [WARNING] camera_monitor: Camera health check failed (failure count: 2/3)
[2026-01-27 10:16:46.123] [WARNING] camera_monitor: Camera health check failed (failure count: 3/3)
[2026-01-27 10:16:46.123] [ERROR] camera_monitor: Camera disconnection detected
[2026-01-27 10:16:46.123] [INFO] camera_error_handler: Attempting camera reconnection (attempt 1/5)
```

## State Machine

```
UNINITIALIZED
    ↓
INITIALIZING (requesting permission, negotiating format)
    ↓
READY (normal operation, monitoring active)
    ↓ [Camera removed/disconnected]
DISCONNECTED (recovery attempts in progress)
    ├─ [Reconnection succeeds] → READY
    └─ [5 reconnection attempts fail] ↓
        ERROR (unrecoverable, must exit)
    ↓
SHUTDOWN (cleanup in progress)
```

## Testing Scenarios (T-9.5)

The error handling can be tested with the following scenarios:

### Unit Tests
- `test_camera_error_handler_not_found()`: Verify fatal error handling
- `test_camera_error_handler_permission_denied()`: Verify permission error
- `test_camera_error_handler_disconnected()`: Verify recoverable error
- `test_camera_monitor_health_check()`: Verify monitoring detects failures
- `test_camera_monitor_reconnection()`: Verify reconnection attempts

### Integration Tests
- `test_camera_initialization_fails()`: Simulate no camera on startup
- `test_camera_permission_denied()`: Simulate permission denial
- `test_camera_disconnect_detection()`: Simulate camera removal mid-session
- `test_camera_reconnection_recovery()`: Verify successful recovery

### E2E Tests
- Launch app, deny camera permission → verify error dialog and exit
- Launch app, no camera attached → verify error and exit
- Launch app, remove camera after 30 seconds → verify detection and recovery
- Launch app, camera boots back up → verify transparent recovery

## Performance Considerations

- **Health Check Overhead**: ~1-2% CPU per check (500ms interval)
- **Memory**: ~2KB per error handler, ~4KB per monitor
- **Latency**: Camera disconnection detected within 1.5 seconds (3 failed checks × 500ms)

## Future Enhancements

1. **Smart Reconnection**: Exponential backoff instead of fixed timing
2. **User Notification**: macOS notifications for camera disconnect/reconnect
3. **Graceful Degradation**: Continue app with frozen live frame during disconnect
4. **Camera Selection**: Support for multiple cameras with fallback switching
5. **Metrics**: Track disconnection frequency for diagnostics

## References

- **SDD**: §3.2 (Camera Input), §7 (Error Handling)
- **PRD**: §4.1 (Application Launch), §5.2 (Reliability/Error Recovery)
- **TTL**: Task T-9.1 (Camera Error Handling)
