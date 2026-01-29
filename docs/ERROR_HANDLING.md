# Error Handling Implementation Guide

This document describes the comprehensive error handling implementation for the Video Looper macOS application, as specified in SDD §7.

## Overview

The Video Looper implements a multi-layered error handling strategy:

1. **Low-level component errors** (camera, pipeline) → logged and propagated
2. **Application-level error dispatch** → centralized error handler
3. **User-facing error dialogs** → NSAlert for fatal errors
4. **Graceful shutdown** → cleanup in reverse initialization order

## Error Categories and Handling

### 1. Initialization Errors (Fatal)

These errors occur during application startup and prevent normal operation.

#### Camera Not Found
- **Trigger**: No built-in camera detected on the system
- **Location**: `src/camera/camera_source.c` - `camera_negotiate_format()`
- **Error Code**: `APP_ERROR_CAMERA_NOT_FOUND`
- **Logging**: ERROR level
- **User Dialog**: "No Camera Found" with system settings guidance
- **Recovery**: None - application exits

```c
// In camera_source.c
if (!camera_negotiate_format(cam)) {
    app_log_error(APP_ERROR_CAMERA_NOT_FOUND,
                  "Unable to negotiate compatible camera format");
    return NULL;  // Propagate to main
}
```

#### Camera Permission Denied
- **Trigger**: User denies camera access in system dialog
- **Location**: `src/camera/camera_source.c` - `camera_source_init()`
- **Error Code**: `APP_ERROR_CAMERA_PERMISSION_DENIED`
- **Logging**: ERROR level
- **User Dialog**: "Camera Access Denied" with System Settings instructions
- **Recovery**: None - application exits

```c
// In camera_source.c
if (perm_status == CAMERA_PERMISSION_DENIED) {
    app_log_error(APP_ERROR_CAMERA_PERMISSION_DENIED,
                  "Camera permission denied by user");
    return NULL;
}
```

#### GStreamer Initialization Failed
- **Trigger**: GStreamer library fails to initialize
- **Location**: `src/main.c` - `initialize_gstreamer()`
- **Error Code**: `APP_ERROR_GSTREAMER_INIT_FAILED`
- **Logging**: ERROR level
- **User Dialog**: "GStreamer Initialization Failed" with GStreamer details
- **Recovery**: None - application exits

```c
// In main.c
if (!gst_init_check(&argc, &argv, &gst_error)) {
    app_log_error(APP_ERROR_GSTREAMER_INIT_FAILED,
                  "Failed to initialize GStreamer library");
    return 1;
}
```

#### Window Creation Failed
- **Trigger**: Cocoa NSWindow creation fails
- **Location**: `src/main.c` - `initialize_window()`
- **Error Code**: `APP_ERROR_WINDOW_CREATE_FAILED`
- **Logging**: ERROR level
- **User Dialog**: "Window Creation Failed" with display settings guidance
- **Recovery**: None - application exits

```c
// In main.c
if (!window) {
    app_log_error(APP_ERROR_WINDOW_CREATE_FAILED,
                  "Could not create application window");
    return FALSE;
}
```

#### Pipeline Build Failed
- **Trigger**: GStreamer pipeline construction fails
- **Location**: `src/main.c` - `initialize_pipeline()`
- **Error Code**: `APP_ERROR_PIPELINE_BUILD_FAILED`
- **Logging**: ERROR level
- **User Dialog**: "Pipeline Build Failed" with GStreamer guidance
- **Recovery**: None - application exits

```c
// In main.c
if (!pipeline) {
    app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED,
                  "Could not construct main video pipeline");
    return FALSE;
}
```

### 2. State Transition Errors

These errors occur when the GStreamer pipeline fails to transition states.

#### Pipeline State Change Failed
- **Trigger**: `gst_element_set_state()` returns `GST_STATE_CHANGE_FAILURE`
- **Location**: `src/gstreamer/pipeline_builder.c` - `pipeline_set_state()`
- **Error Code**: `APP_ERROR_PIPELINE_STATE_CHANGE_FAILED`
- **Logging**: ERROR level
- **User Dialog**: Generic "Pipeline Error" (during runtime)
- **Recovery**: Attempt to return to READY state (TBD in runtime error handling)

```c
// In pipeline_builder.c
if (ret == GST_STATE_CHANGE_FAILURE) {
    LOG_ERROR("State change to %s failed",
              gst_element_state_get_name(state));
    app_log_error(APP_ERROR_PIPELINE_STATE_CHANGE_FAILED,
                  "Failed to transition pipeline to %s state",
                  gst_element_state_get_name(state));
    return FALSE;
}
```

### 3. Runtime Errors (Non-Fatal)

These errors occur during normal operation and may be recoverable.

#### Camera Disconnected
- **Trigger**: Camera device is unplugged or loses connection
- **Location**: Pipeline bus message handler in `src/gstreamer/pipeline_builder.c`
- **Error Code**: `APP_ERROR_CAMERA_DISCONNECTED`
- **Logging**: WARNING level
- **Recovery**: Freeze live feed, disable recording, wait for reconnection
- **Status**: Partial implementation - bus handler detects errors

#### Recording Buffer Full
- **Trigger**: Ring buffer for recording reaches capacity
- **Location**: `src/recording/buffer_manager.c`
- **Error Code**: `APP_WARNING_RECORDING_BUFFER_FULL`
- **Logging**: WARNING level
- **Recovery**: Drop oldest frame, continue recording with next frame
- **Status**: Implemented in ring buffer logic

#### Memory Allocation Failed
- **Trigger**: `malloc()`, `g_malloc()` or similar allocation fails
- **Location**: Any component
- **Error Code**: `APP_ERROR_MEMORY_ALLOCATION_FAILED`
- **Logging**: ERROR level
- **Recovery**: Attempt to free resources and exit gracefully
- **Status**: Checked in critical allocation points

#### Keyboard Handler Failed
- **Trigger**: Keyboard input initialization fails
- **Location**: `src/input/keyboard_handler.c`
- **Error Code**: `APP_ERROR_KEYBOARD_HANDLER_FAILED`
- **Logging**: WARNING level (non-fatal)
- **Recovery**: Continue without keyboard input
- **Status**: Logged but non-blocking

### 4. Warnings (Non-Fatal)

#### Frame Drop Detected
- **Code**: `APP_WARNING_FRAME_DROP_DETECTED`
- **Logging**: WARNING level
- **Recovery**: Continue operation, log for debugging
- **Status**: Can be added to frame delivery monitoring

#### Memory Usage High
- **Code**: `APP_WARNING_MEMORY_USAGE_HIGH`
- **Logging**: WARNING level
- **Recovery**: Continue operation, monitor growth
- **Status**: Can be added to resource monitoring

## Error Handler Architecture

### Application Error Handler (`src/app/app_error.h|c`)

Centralized error handling using GLib-style error model:

```c
typedef struct {
    AppErrorCode code;      // Error code enum
    gchar *message;         // Formatted error message
    const gchar *function;  // Function name (optional)
    const gchar *file;      // Source file (optional)
    guint line;             // Source line (optional)
    gpointer user_data;     // User callback data
} AppError;

void app_log_error(AppErrorCode code, const gchar *format, ...);
void app_log_warning(AppErrorCode code, const gchar *format, ...);
AppError *app_get_last_error(void);
```

Features:
- Stores last error for retrieval
- Calls registered error handler callback
- Formats error messages with varargs
- Centralized error code definitions

### Error Dialog Module (`src/app/error_dialog.h|m`)

User-facing NSAlert dialogs for fatal errors:

```c
gboolean error_dialog_show(ErrorDialogType type, const gchar *title,
                           const gchar *message);
gboolean error_dialog_show_camera_permission_denied(void);
gboolean error_dialog_show_camera_not_found(void);
gboolean error_dialog_show_gstreamer_init_failed(const gchar *reason);
gboolean error_dialog_show_generic(const gchar *title, const gchar *message);
```

Features:
- Native Cocoa NSAlert implementation
- Specialized dialogs for common errors
- Main thread dispatch for GUI operations
- Graceful fallback if display fails

### Error Dispatch in `main.c`

The `on_app_error()` callback is registered early:

```c
app_register_error_handler(on_app_error, NULL);
```

This callback:
1. Logs the error with context
2. Dispatches to appropriate error dialog based on error code
3. Provides user-friendly messages with actionable guidance
4. Logs for debugging purposes

## Error Flow Examples

### Scenario 1: Camera Permission Denied

```
User launches app
  ↓
main() initializes core systems
  ↓
initialize_camera() called
  ↓
camera_source_init() checks permission
  ↓
camera_request_permission() → DENIED
  ↓
app_log_error(APP_ERROR_CAMERA_PERMISSION_DENIED, ...)
  ↓
on_app_error() callback invoked
  ↓
error_dialog_show_camera_permission_denied()
  ↓
NSAlert displayed to user
  ↓
User clicks "OK"
  ↓
initialize_camera() returns FALSE
  ↓
initialize_components() returns FALSE
  ↓
main() cleans up and exits with code 1
```

### Scenario 2: GStreamer Pipeline Error at Runtime

```
User presses key to record
  ↓
Recording → buffer write → playback bin creation
  ↓
GStreamer pipeline processes frames
  ↓
Bus message: GST_MESSAGE_ERROR
  ↓
pipeline_bus_watch_handler() receives error
  ↓
app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED, ...)
  ↓
on_app_error() callback invoked
  ↓
error_dialog_show_generic("Pipeline Error", ...)
  ↓
NSAlert displayed to user
  ↓
User clicks "OK"
  ↓
Application may continue or shut down based on recovery logic
```

## Cleanup on Error

All error paths ensure proper cleanup:

1. **Resource cleanup** - memory, GStreamer objects freed in reverse order
2. **Logging** - all errors logged for debugging
3. **User notification** - error dialogs for fatal errors
4. **Graceful exit** - application exits cleanly with appropriate exit code

Exit codes:
- `0` - Normal exit
- `1` - Camera or permission error
- `2` - GStreamer init error
- `3` - Window creation error
- Other - Component initialization error

## Testing Error Handling

### Unit Tests
- Test error logging functions
- Test error callback dispatch
- Test error dialog creation (mock NSAlert)

### Integration Tests
- Test camera permission denial flow
- Test GStreamer init failure scenarios
- Test pipeline state change failures

### Manual Testing
- Deny camera permission and observe dialog
- Unplug camera during initialization
- Kill GStreamer process to simulate failure
- Monitor error logs for proper categorization

## Best Practices for Developers

### When Adding New Error Conditions

1. Define error code in `app_error.h`:
   ```c
   APP_ERROR_NEW_CONDITION = 205,
   ```

2. Log error in component:
   ```c
   app_log_error(APP_ERROR_NEW_CONDITION,
                 "Detailed error message with context");
   ```

3. Handle in `on_app_error()` callback if user-facing:
   ```c
   case APP_ERROR_NEW_CONDITION:
       error_dialog_show_generic("Title", "Message");
       break;
   ```

### Error Logging Rules

- **ERROR**: Fatal errors that prevent operation
- **WARNING**: Non-fatal issues that don't block operation
- **INFO**: Significant state changes (init success, shutdown)
- **DEBUG**: Detailed diagnostic information

### Propagation Pattern

```c
// Low-level component
component_do_work() {
    if (failure_condition) {
        LOG_ERROR("Specific error condition");
        app_log_error(APP_ERROR_CODE, "Detailed message");
        return NULL;  // Propagate to caller
    }
    return result;
}

// Mid-level handler
handler_init_component() {
    result = component_do_work();
    if (!result) {
        LOG_ERROR("Component initialization failed");
        return FALSE;  // Propagate to main
    }
    return TRUE;
}

// Main entry point
main() {
    if (!handler_init_component()) {
        LOG_ERROR("Initialization failed");
        cleanup_and_exit(1);
    }
}
```

## Future Enhancements

1. **Runtime Error Recovery**
   - Implement recovery mechanisms for non-fatal errors
   - Restart failed components without full shutdown
   - Notify user of automatic recovery attempts

2. **Error Statistics**
   - Track error frequency by type
   - Log performance impact of errors
   - Generate diagnostic reports

3. **User Feedback**
   - Error reporting dialog with copy/paste
   - System logs integration
   - Automatic bug report submission (opt-in)

4. **Advanced Logging**
   - Rotate error logs for long sessions
   - Compress old logs
   - Search and filter error history

## References

- **SDD §7**: Error Handling Strategy
- **SDD §7.1**: Exception/Error Hierarchy
- **SDD §7.2**: Error Propagation
- **SDD §7.3**: Error Handling by Category
- **SDD §7.4**: Logging Approach
- **SDD §7.5**: User Feedback and Recovery
- **PRD §4.1**: Application Launch Requirements
- **PRD §5.2**: Reliability Requirements
