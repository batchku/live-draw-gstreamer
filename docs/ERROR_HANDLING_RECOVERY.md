# Keyboard and Window Event Error Recovery

## Overview

Task T-9.4 implements comprehensive error recovery mechanisms for keyboard input and window event handling, with user-friendly error dialogs for fatal errors. This document describes the error recovery architecture, recovery strategies, and integration with the main application.

## Architecture

### Components

The error recovery system consists of four main modules:

#### 1. **Keyboard Event Error Recovery** (`src/input/keyboard_event_recovery.h/c`)

Handles errors in keyboard input processing:
- **Error Detection**: Tracks errors from keyboard event handler (not initialized, callback missing)
- **Error Counting**: Maintains error count within a 1-second window
- **Recovery Threshold**: Triggers recovery after 5 errors in 1 second
- **Recovery Strategies**:
  - Reinitialization: Reinitialize keyboard handler
  - Fallback: Use alternate event dispatch
  - Retry: Retry failed operations
- **Recovery Limits**: Maximum 3 recovery attempts before giving up

#### 2. **Window Event Error Recovery** (`src/osx/window_event_recovery.h/c`)

Handles errors in window management and rendering:
- **Error Detection**: Detects window creation failures, visibility loss, rendering errors
- **Visibility Monitoring**: Tracks if window becomes hidden/minimized
- **Rendering Health**: Monitors rendering context status
- **Recovery Strategies**:
  - Recreate: Recreate the window
  - Restore Visibility: Make window visible again
  - Reset Rendering: Reset rendering context
- **Recovery Limits**: Maximum 2 recovery attempts before giving up

#### 3. **Error Recovery Dialogs** (`src/app/error_recovery_dialog.h/c`)

Displays user-friendly error messages:
- **Keyboard Handler Failure**: Explains keyboard input issues
- **Window Failure**: Explains window creation/recovery issues
- **Rendering Failure**: Explains video rendering problems
- **Visibility Loss**: Notifies user window is hidden
- **Generic Fatal Error**: Custom error messages
- **Recovery Attempt Info**: Non-blocking progress messages

#### 4. **Error Handler Integration** (`src/app/error_handler_integration.h/c`)

Central coordination point:
- **Error Dispatch**: Routes errors to appropriate recovery systems
- **Recovery Orchestration**: Coordinates recovery attempts across components
- **Statistics Tracking**: Counts errors, warnings, and fatal errors
- **State Management**: Maintains overall error and recovery state
- **Cleanup Coordination**: Ensures proper shutdown

### Data Flow

```
Component Error (Keyboard/Window/Camera/GStreamer)
    ↓
ErrorHandlerIntegration::handle_error()
    ↓
    ├→ Log error
    ├→ Determine severity (warning/error/fatal)
    ├→ Update error counter
    │
    ├→ IF Recoverable:
    │   ├→ Component-specific recovery (Keyboard/Window recovery)
    │   ├→ Schedule retry or reinitialization
    │   └→ Return success
    │
    └→ IF Fatal:
        ├→ Display error dialog
        ├→ Prepare for shutdown
        └→ Return failure
```

## Usage Examples

### Keyboard Error Recovery

```c
// In keyboard event handler (main application code)
if (!keyboard_handler_initialized || callback == NULL) {
    // Dispatch error to recovery system
    ErrorHandlerContext ctx = {
        .component = ERROR_COMPONENT_KEYBOARD,
        .severity = ERROR_SEVERITY_ERROR,
        .error_code = APP_ERROR_KEYBOARD_HANDLER_FAILED,
        .error_message = "Keyboard handler not initialized"
    };

    if (!error_handler_integration_handle_error(&ctx)) {
        // Fatal error; prepare for shutdown
    }
}
```

### Window Error Recovery

```c
// In window creation code
OSXWindow *win = window_create(10);
if (!win) {
    ErrorHandlerContext ctx = {
        .component = ERROR_COMPONENT_WINDOW,
        .severity = ERROR_SEVERITY_FATAL,
        .error_code = APP_ERROR_WINDOW_CREATE_FAILED,
        .error_message = "NSWindow creation failed"
    };

    error_handler_integration_fatal_error(
        "Window Creation Failed",
        "Could not create application window",
        3);
}
```

### Window Visibility Recovery

```c
// Periodic check in main event loop
if (window_recovery_check_visibility(win)) {
    // Visibility lost; attempt recovery
    if (!window_recovery_restore_visibility(win)) {
        ErrorHandlerContext ctx = {
            .component = ERROR_COMPONENT_WINDOW,
            .severity = ERROR_SEVERITY_ERROR,
            .error_code = WINDOW_ERROR_VISIBILITY_LOST,
            .error_message = "Cannot restore window visibility"
        };
        error_handler_integration_handle_error(&ctx);
    }
}
```

## Recovery Thresholds

### Keyboard Recovery

| Threshold | Value | Meaning |
|-----------|-------|---------|
| Error threshold | 5 errors | Trigger recovery after this many errors |
| Time window | 1 second | Window for counting errors |
| Max attempts | 3 | Maximum recovery attempts |
| Recovery success | Reinitialization | Reinitialize handler and continue |

### Window Recovery

| Threshold | Value | Meaning |
|-----------|-------|---------|
| Error threshold | 3 errors | Trigger recovery after this many errors |
| Time window | 5 seconds | Window for counting errors |
| Max attempts | 2 | Maximum recovery attempts |
| Recovery success | Restore visibility or recreate | Continue operation |

## Error Codes

### Keyboard Error Codes

```c
APP_ERROR_KEYBOARD_HANDLER_FAILED = 204  // Handler initialization/operation failed
```

### Window Error Codes

```c
WINDOW_ERROR_WINDOW_NOT_FOUND = 1000           // Window pointer invalid
WINDOW_ERROR_VIDEOSINK_MISSING = 1001          // osxvideosink element missing
WINDOW_ERROR_VISIBILITY_LOST = 1002            // Window hidden/minimized
WINDOW_ERROR_RENDERING_FAILED = 1003           // Rendering context error
WINDOW_ERROR_RESIZE_FAILED = 1004              // Window resize failed
WINDOW_ERROR_FRAME_UPDATE_FAILED = 1005        // Frame update failed
```

## Integration with Main Application

### Initialization

Call during application startup:

```c
int main(int argc, char *argv[]) {
    // Initialize logging first
    logging_init();

    // Initialize error handling system
    error_handler_integration_init();

    // Initialize other components...
    keyboard_init(on_key_event);
    window_create(...);

    // Run main event loop...
}
```

### Periodic Health Checks

In the main event loop:

```c
// Check if recovery is needed
if (error_handler_integration_recovery_needed()) {
    if (!error_handler_integration_attempt_recovery()) {
        // Recovery failed; consider shutting down
        LOG_ERROR("Error recovery failed");
        break;  // Exit event loop
    }
}

// Check window visibility periodically
window_recovery_check_visibility(win);
```

### Shutdown

Call during application shutdown:

```c
// Cleanup error handling
error_handler_integration_cleanup();
```

## Error Dialog User Messages

### Keyboard Handler Failure

**Title**: "Keyboard Handler Error"

**Message**: "The keyboard input handler encountered repeated failures and could not recover. The application may not respond to keyboard input. You can continue using the application, but keyboard controls will be unavailable. Please try restarting the application if this persists."

**Action**: User clicks OK to continue or Exit to quit

### Window Visibility Loss

**Title**: "Window Not Visible"

**Message**: "The application window is no longer visible. It may have been minimized or hidden. The application will attempt to restore the window to the foreground."

**Action**: Automatic recovery attempt

### Window Creation Failed

**Title**: "Window Creation Failed"

**Message**: "The application window could not be created or recovered. [Optional: error details]. Please try restarting the application."

**Action**: User must exit; application cannot continue without window

### Rendering Failure

**Title**: "Rendering Error"

**Message**: "Video rendering has failed. This could be due to a GPU issue or incompatible display settings. The application will attempt to continue, but video may not display correctly. Please try restarting the application or checking your display settings."

**Action**: User clicks OK to continue or Exit to quit

## Testing

### Unit Tests

- `test/unit/test_keyboard_handler.c`: Keyboard event handling and error cases
- `test/unit/test_window_creation.c`: Window creation and error recovery

### Integration Tests

- `test/integration/test_pipeline_error_handling.c`: Full pipeline error scenarios
- `test/integration/test_app_lifecycle.c`: Application startup/shutdown with errors

### Manual Testing

1. **Keyboard Recovery**: Simulate keyboard handler not initialized, verify recovery
2. **Window Visibility**: Minimize window during execution, verify recovery attempt
3. **Fatal Errors**: Trigger fatal error (e.g., no window), verify error dialog and shutdown

## Performance Implications

- **Error Tracking**: O(1) per error; minimal overhead
- **Recovery State**: Mutex-protected; lock held briefly
- **Dialog Display**: Blocks until user responds; modal behavior
- **Error Counting**: 1-5 second windows per component
- **No Performance Degradation**: Recovery system transparent during normal operation

## Future Enhancements

1. **Error Logging to File**: Log errors and recovery attempts to persistent storage
2. **Recovery Metrics**: Track recovery success rates and error patterns
3. **Automatic Restart**: Optionally restart application on fatal errors
4. **Network Reporting**: Send error telemetry (optional, privacy-respecting)
5. **Graceful Degradation**: Run in reduced mode if some components fail
6. **Recovery Strategies**: Machine learning to predict needed recovery actions

## References

- SDD §3.7: Keyboard Input Handler
- SDD §3.8: OS X Window and Rendering
- SDD §7: Error Handling Strategy
- SDD §7.5: User Feedback and Recovery
