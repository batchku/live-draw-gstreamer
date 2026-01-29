# Software Design Document: Video Looper for macOS

## 1. System Overview

### Architecture Style and System Boundaries

Video Looper is a **GPU-accelerated real-time video looping application** built on GStreamer with native macOS integration. The system follows a **pipeline-based architecture** where all video processing occurs on the GPU, with a minimal CPU layer handling keyboard input and window management.

**System Boundaries:**
- **Input**: Built-in macOS camera feed, keyboard input (keys 1-9)
- **Processing**: GStreamer pipeline for GPU-accelerated video capture, recording, and playback
- **Output**: OS X window with 10-cell grid display at 120 fps
- **Storage**: In-memory video loop storage (no persistence)

### Architecture Style

The application uses a **GStreamer Pipeline Architecture** where:
1. A master GStreamer pipeline captures the live camera feed
2. Nine branching record pipelines capture keyboard-triggered recordings
3. Nine playback pipelines implement palindrome looping
4. A compositor pipeline merges all streams into a 10-cell grid
5. An OS X videosink renders the composite to the application window

This architecture achieves complete GPU acceleration by keeping all video data on the GPU throughout its lifecycle.

### Key Architectural Decisions and Rationale

| Decision | Rationale |
|----------|-----------|
| **GStreamer Pipeline-Based** | Provides native GPU acceleration, minimal CPU overhead, and standard components for video processing |
| **GPU-Only Processing** | Meets performance requirement of 120 fps across 10 cells; avoids CPU bottlenecks from frame transfer |
| **In-Memory Storage** | Simplifies implementation per PRD scope (no persistence required); focuses on real-time performance |
| **Single Monolithic Application** | Aligns with "single terminal command" requirement; simpler deployment and lower latency |
| **Keyboard-Driven Interface** | Meets accessibility requirement and performance (no GUI event loop overhead) |
| **Minimal External Dependencies** | Relies on vanilla GStreamer and macOS system libraries only |
| **Direct OS X Integration** | Uses native APIs for camera access (AVFoundation) and window management (Cocoa) |

### Design Principles

- **Single Responsibility Principle (SRP)**: Each component handles one aspect (capture, record, playback, compose, render)
- **Separation of Concerns**: Video pipeline logic separated from input handling and window management
- **GPU Affinity**: All video data remains on GPU; CPU handles only control flow
- **Fail-Fast**: Errors in camera initialization cause immediate graceful shutdown
- **Minimal Abstraction**: Direct GStreamer API usage reduces indirection and improves performance

---

## 2. Architecture Diagram

### System Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          Video Looper Application                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                               │
│  ┌──────────────────┐                                                       │
│  │  Keyboard Input  │                                                       │
│  │  Handler (CPU)   │◄──────┐                                               │
│  └──────────────────┘       │                                               │
│          │                  │ Key Events                                    │
│          ▼                  │ (1-9, quit)                                   │
│  ┌──────────────────┐       │                                               │
│  │ Recording State  │◄──────┘                                               │
│  │ Manager (CPU)    │                                                       │
│  └──────────────────┘                                                       │
│          │                                                                   │
│          │ Record Start/Stop Signals                                        │
│          ▼                                                                   │
│  ┌────────────────────────────────────────────────────────────────────┐     │
│  │              GStreamer Pipeline (GPU)                              │     │
│  ├────────────────────────────────────────────────────────────────────┤     │
│  │                                                                    │     │
│  │  ┌────────────────┐                                               │     │
│  │  │  Camera Source │ (AVFoundation)                               │     │
│  │  │  1920x1080     │                                               │     │
│  │  └────────┬───────┘                                               │     │
│  │           │                                                       │     │
│  │           ▼                                                       │     │
│  │  ┌────────────────┐                                               │     │
│  │  │ Live Tee       │                                               │     │
│  │  │ (Split Stream) │                                               │     │
│  │  └────┬─────┬─────┘                                               │     │
│  │       │     │                                                     │     │
│  │  ┌────▼──┐ │  Cell 1: Live Feed Queue (GPU Memory)               │     │
│  │  │Queue  │ │  Display in grid position 1                         │     │
│  │  └────┬──┘ │                                                     │     │
│  │       │    │                                                     │     │
│  │       │    ├──────────────┐                                      │     │
│  │       │    │              │                                      │     │
│  │       │    │   Record Pipelines (Keys 1-9)                       │     │
│  │       │    │   ┌────────────────────────┐                       │     │
│  │       │    ├──►│ Record Bin #1 (Key 1)  │◄──Record Signal        │     │
│  │       │    │   │ Queue → Capsfilter     │                       │     │
│  │       │    │   │ → FakeSink (GPU Mem)   │                       │     │
│  │       │    │   └────────────────────────┘                       │     │
│  │       │    │   ┌────────────────────────┐                       │     │
│  │       │    ├──►│ Record Bin #2 (Key 2)  │◄──Record Signal        │     │
│  │       │    │   │ Queue → Capsfilter     │                       │     │
│  │       │    │   │ → FakeSink (GPU Mem)   │                       │     │
│  │       │    │   └────────────────────────┘                       │     │
│  │       │    │   ┌────────────────────────┐                       │     │
│  │       │    └──►│ Record Bin #9 (Key 9)  │◄──Record Signal        │     │
│  │       │        │ Queue → Capsfilter     │                       │     │
│  │       │        │ → FakeSink (GPU Mem)   │                       │     │
│  │       │        └────────────────────────┘                       │     │
│  │       │                                                         │     │
│  │       └──────────────────────┐                                  │     │
│  │                              │                                  │     │
│  │                              ▼                                  │     │
│  │                    ┌──────────────────┐                         │     │
│  │                    │  Playback Loop   │                         │     │
│  │                    │  Manager (CPU)   │                         │     │
│  │                    │ (Palindrome)     │                         │     │
│  │                    └──────┬───────────┘                         │     │
│  │                           │                                     │     │
│  │          ┌────────────────┼────────────────┐                   │     │
│  │          │                │                │                   │     │
│  │          ▼                ▼                ▼                    │     │
│  │    ┌──────────┐  ┌──────────────────┐  ┌──────────┐            │     │
│  │    │Playback  │  │ Playback Bins    │  │Playback  │            │     │
│  │    │Bin #1    │  │ (Keys 2-9)       │  │Bin #9    │            │     │
│  │    │(Cell 2)  │  │ (Cells 3-10)     │  │(Cell 10) │            │     │
│  │    └────┬─────┘  └────────┬─────────┘  └────┬─────┘            │     │
│  │         │                 │                 │                  │     │
│  │         └─────────────────┼─────────────────┘                  │     │
│  │                           │                                     │     │
│  │                           ▼                                     │     │
│  │                    ┌──────────────┐                             │     │
│  │                    │ Videomixer   │                             │     │
│  │                    │ Compositor   │ (10-cell grid)              │     │
│  │                    │ (GL/Metal)   │                             │     │
│  │                    └──────┬───────┘                             │     │
│  │                           │                                     │     │
│  │                           ▼                                     │     │
│  │                    ┌──────────────┐                             │     │
│  │                    │ VideoCapsfilter                             │     │
│  │                    │ (Format Conv) │                             │     │
│  │                    └──────┬───────┘                             │     │
│  │                           │                                     │     │
│  │                           ▼                                     │     │
│  │                    ┌──────────────┐                             │     │
│  │                    │ osxvideosink │                             │     │
│  │                    │ (240 fps cap)│                             │     │
│  │                    └──────────────┘                             │     │
│  │                           │                                     │     │
│  └───────────────────────────┼─────────────────────────────────────┘     │
│                              │                                            │
│                              ▼                                            │
│                    ┌──────────────────────┐                              │
│                    │   OS X Window        │                              │
│                    │   (Cocoa/Metal)      │                              │
│                    │   10-Cell Grid View  │                              │
│                    │   320px × 10 cells   │                              │
│                    └──────────────────────┘                              │
│                                                                           │
└─────────────────────────────────────────────────────────────────────────┘
```

### Data Flow Diagram

```
Camera Input (1920×1080 @ 30fps, GPU)
    │
    ▼
Live Feed Tee (GPU)
    │
    ├─────────────────┬──────────────────┬──────────────────┐
    │                 │                  │                  │
    ▼                 ▼                  ▼                  ▼
Cell 1 Queue     Record Bin 1      Record Bin 2      Record Bin 9
(Live Feed)      (Key 1)           (Key 2)           (Key 9)
    │                 │                  │                  │
    │                 ▼                  ▼                  ▼
    │            GPU Buffer          GPU Buffer         GPU Buffer
    │            (Ring Buffer)        (Ring Buffer)      (Ring Buffer)
    │                 │                  │                  │
    │                 └──────────────────┼──────────────────┘
    │                                    │
    │                 ┌──────────────────┘
    │                 │
    │                 ▼
    │          Playback Loop Mgr (CPU)
    │          - Detects record end
    │          - Reads buffers
    │          - Creates playback bins
    │          - Manages palindrome state
    │                 │
    ├─────────────────┼──────────────────┐
    │                 │                  │
    ▼                 ▼                  ▼
Videomixer Compositor (GPU)
    │
    ▼
VideoCapsfilter (Format conversion to window format)
    │
    ▼
osxvideosink (Cocoa integration, Metal rendering)
    │
    ▼
OS X Window (Cocoa NSWindow)
```

---

## 3. Component Specifications

### 3.1 Application Entry Point

**File**: `src/main.c`

**Purpose**: Initialize application, set up GStreamer, create window, and run main event loop.

**Public Interface**:

```c
// Main entry point
int main(int argc, char *argv[]);

// Called once during initialization
typedef struct {
    GstElement *pipeline;
    GstElement *live_queue;
    GstElement *record_bins[9];        // One for each key (1-9)
    GstElement *playback_bins[9];      // One for each loop (cells 2-10)
    GstElement *videomixer;
    GstElement *osxvideosink;
    NSWindow *window;
} AppState;

// Error handling
typedef enum {
    APP_ERROR_CAMERA_NOT_FOUND,
    APP_ERROR_CAMERA_PERMISSION_DENIED,
    APP_ERROR_GSTREAMER_INIT_FAILED,
    APP_ERROR_WINDOW_CREATE_FAILED,
    APP_ERROR_PIPELINE_STATE_FAILED
} AppErrorCode;

void handle_app_error(AppErrorCode code, const char *message);
```

**Responsibilities**:
- Initialize GStreamer library
- Create and configure OS X window (Cocoa)
- Request camera permissions from macOS
- Build the main GStreamer pipeline
- Initialize keyboard input handler
- Run the GTK+ main event loop
- Handle shutdown and cleanup

**Dependencies**:
- GStreamer 1.20+
- Cocoa framework (OS X window management)
- AVFoundation (camera access)
- Core Foundation

**Error Handling**:
- If camera permission denied, display error dialog and exit with code 1
- If GStreamer initialization fails, log error and exit with code 2
- If window creation fails, exit with code 3
- All cleanup performed in atexit handlers

### 3.2 Camera Input Component

**File**: `src/camera/camera_source.c`

**Purpose**: Initialize and manage connection to built-in macOS camera, negotiate format, and expose raw video stream.

**Public Interface**:

```c
// Camera initialization
typedef struct {
    GstElement *source_element;
    char device_id[256];
    int width;
    int height;
    int framerate;
    char caps_string[512];
} CameraSource;

CameraSource* camera_source_init(void);

// Request camera permission and verify access
typedef enum {
    CAMERA_PERMISSION_GRANTED,
    CAMERA_PERMISSION_DENIED,
    CAMERA_PERMISSION_NOT_DETERMINED
} CameraPermissionStatus;

CameraPermissionStatus camera_request_permission(void);

// Query camera capabilities
typedef struct {
    int *supported_widths;      // NULL-terminated array
    int *supported_heights;
    int *supported_framerates;
    int count;
} CameraCapabilities;

CameraCapabilities* camera_get_capabilities(CameraSource *cam);

// Create GStreamer source element
GstElement* camera_source_create_element(CameraSource *cam);

// Cleanup
void camera_source_cleanup(CameraSource *cam);
void camera_capabilities_free(CameraCapabilities *caps);
```

**Responsibilities**:
- Request AVFoundation camera permission using macOS APIs
- Detect and enumerate built-in camera hardware
- Negotiate optimal resolution (1920×1080 preferred, fallback to 1280×720)
- Negotiate frame rate (30 fps native camera output, will be interpolated to 120 fps in playback)
- Create avfvideosrc GStreamer element configured for raw output
- Provide capsfilter with correct caps string for downstream elements

**Dependencies**:
- AVFoundation framework
- GStreamer avf plugin
- Core Media framework

**Error Handling**:
- Permission denied: Return `CAMERA_PERMISSION_DENIED` and log "Camera permission denied by user"
- Camera not found: Return NULL and log "No built-in camera detected"
- Format negotiation fails: Log warning, use fallback resolution

**Data Structures**:

```c
// Used internally
typedef struct {
    AVCaptureDevice *device;
    AVCaptureSession *session;
    AVCaptureVideoDataOutput *output;
    dispatch_queue_t queue;
} AVFoundationContext;
```

### 3.3 Recording State Manager

**File**: `src/recording/recording_state.c`

**Purpose**: Track keyboard input and manage which cells are currently recording. Translate key press/release events to record start/stop signals sent to GStreamer bins.

**Public Interface**:

```c
// Recording state tracker
typedef struct {
    gboolean is_recording[9];          // One per key (1-9)
    guint64 record_start_time[9];      // Timestamp when recording started
    guint64 record_duration_us[9];     // Duration in microseconds
    gint current_cell_index;           // Next cell to fill (0-8, maps to cells 2-10)
} RecordingState;

// Initialize state manager
RecordingState* recording_state_init(void);

// Handle keyboard input
void recording_on_key_press(RecordingState *state, int key_number);
void recording_on_key_release(RecordingState *state, int key_number);

// Query recording state
gboolean recording_is_recording(RecordingState *state, int key_number);
guint64 recording_get_duration(RecordingState *state, int key_number);

// Request GStreamer bins start/stop recording
void recording_start_capture(GstElement *record_bin, guint64 start_time);
void recording_stop_capture(GstElement *record_bin, guint64 duration_us);

// Cleanup
void recording_state_cleanup(RecordingState *state);
```

**Responsibilities**:
- Track which keys (1-9) are currently pressed
- Measure duration of each key press in microseconds
- Provide immediate feedback (visual indicator) when recording starts
- Translate key events to GStreamer bin control signals
- Manage circular buffer indexing for cell assignment (cell 2 → cell 3 → ... → cell 10 → cell 2)
- Coordinate with playback manager when recording ends

**Dependencies**:
- GStreamer GLib integration

**Error Handling**:
- Multiple simultaneous key presses: Each handled independently
- Key press duration < 1 frame (33ms at 30fps): Still records (minimum 1 frame capture)
- All keys released before playback setup: No recording occurs

**State Machine**:

```
Key Press (Not Recording)
    │
    ├─► [RECORDING] ────────────────► Key Release
    │                                      │
    ├─────────────────────────────────────┘
    │
    ▼
Signal Recording Manager
(Notify playback component of new recording)
```

### 3.4 GStreamer Pipeline Builder

**File**: `src/gstreamer/pipeline_builder.c`

**Purpose**: Construct the main GStreamer pipeline with all source, recording, playback, and rendering elements. Manage element lifecycle and state transitions.

**Public Interface**:

```c
// Pipeline construction
typedef struct {
    GstElement *pipeline;
    GstElement *camera_source;
    GstElement *live_queue;
    GstElement *live_caps;
    GstElement *record_bins[9];
    GstElement *playback_bins[9];      // Created dynamically
    GstElement *playback_queues[9];
    GstElement *videomixer;
    GstElement *composite_caps;
    GstElement *osxvideosink;
    GstBus *bus;
} Pipeline;

// Create pipeline
Pipeline* pipeline_create(CameraSource *camera);

// Add/remove recording and playback bins
void pipeline_add_record_bin(Pipeline *p, int key_num);
void pipeline_remove_record_bin(Pipeline *p, int key_num);

void pipeline_add_playback_bin(Pipeline *p, int cell_num, guint64 duration_us);
void pipeline_remove_playback_bin(Pipeline *p, int cell_num);

// State management
gboolean pipeline_set_state(Pipeline *p, GstState state);
GstState pipeline_get_state(Pipeline *p);

// Cleanup
void pipeline_cleanup(Pipeline *p);

// Bus message handling
typedef enum {
    PIPELINE_MSG_ERROR,
    PIPELINE_MSG_WARNING,
    PIPELINE_MSG_INFO,
    PIPELINE_MSG_STATE_CHANGED
} PipelineMessageType;

typedef void (*PipelineMessageCallback)(PipelineMessageType type, const char *message);
void pipeline_set_message_callback(Pipeline *p, PipelineMessageCallback cb);
```

**Responsibilities**:
- Create and link camera source element
- Create live queue (GPU memory buffer) for cell 1
- Create nine record bins (one per key), initially idle
- Create nine playback bins (created dynamically when recordings finish)
- Create videomixer compositor for 10-cell grid layout
- Create osxvideosink for window rendering
- Manage dynamic addition/removal of bins during runtime
- Implement proper queue management to prevent deadlocks
- Handle GStreamer bus messages (errors, state changes)

**Dependencies**:
- GStreamer 1.20+
- GStreamer GL plugin (for GPU acceleration)
- GStreamer videomixer plugin

**Error Handling**:
- Element creation fails: Log error, propagate to main
- Pipeline state change fails: Revert to previous state, log error
- Bus error message: Log error details, attempt recovery or exit
- Deadlock detection: Timeout on state changes, force READY state

**Element Configuration**:

```c
// Example element properties
// osxvideosink:
//   sync=true (synchronize to clock for 120fps)
//   fullscreen=false
//   force-aspect-ratio=true
//
// videomixer:
//   background=black
//   latency=0 (minimize latency)
//   pad-properties: alpha=1.0, zorder=0-9
//
// queues:
//   max-size-buffers=30 (1 second at 30fps input)
//   max-size-bytes=0 (unlimited)
//   max-size-time=0 (unlimited)
```

### 3.5 Recording Buffer Manager

**File**: `src/recording/buffer_manager.c`

**Purpose**: Manage GPU memory buffers for recorded video, implement ring buffer storage for each recording, and provide read access to playback pipeline.

**Public Interface**:

```c
// Ring buffer for one recording
typedef struct {
    GstBuffer **frames;                // Ring of GStreamer buffers
    guint capacity;                    // Max number of frames
    guint write_pos;                   // Current write position
    guint read_pos;                    // Current read position
    guint frame_count;                 // Total frames stored
    guint64 duration_us;               // Total duration
    GstCaps *caps;                     // Frame format (resolution, etc)
} RingBuffer;

// Create buffer for recording
RingBuffer* buffer_create(guint max_frames, GstCaps *caps);

// Write frame to buffer (called during recording)
void buffer_write_frame(RingBuffer *buf, GstBuffer *frame);

// Read frame from buffer (called during playback)
GstBuffer* buffer_read_frame(RingBuffer *buf, guint frame_index);
GstBuffer* buffer_read_frame_palindrome(RingBuffer *buf, guint *frame_index,
                                        gboolean *is_reverse);

// Query buffer state
guint buffer_get_frame_count(RingBuffer *buf);
guint64 buffer_get_duration(RingBuffer *buf);
GstCaps* buffer_get_caps(RingBuffer *buf);

// Cleanup
void buffer_cleanup(RingBuffer *buf);
```

**Responsibilities**:
- Allocate GPU memory for video frames during recording
- Implement ring buffer to allow continuous recording and reading
- Store raw GStreamer buffers (no CPU copy)
- Provide read interface for playback
- Track frame count and duration for each recording
- Handle memory exhaustion (oldest frames discarded if buffer fills)

**Dependencies**:
- GStreamer memory management
- GPU memory (via GStreamer memory allocators)

**Error Handling**:
- Buffer exhaustion: Drop oldest frame, continue recording
- Invalid frame indices: Return NULL
- Memory allocation fails: Return NULL, log error

**Memory Model**:
- Each frame stored as GStreamer GstBuffer on GPU
- Ring buffer uses fixed-size array of buffer pointers
- Capacity = ~60 frames per recording (2 seconds @ 30fps input)
- Total GPU memory per recording ≈ 1920×1080×3×60 ≈ 373 MB (assuming RGB)
- 9 recordings max ≈ 3.4 GB (acceptable for modern Macs)

### 3.6 Playback Loop Manager

**File**: `src/playback/playback_manager.c`

**Purpose**: Implement palindrome looping logic, create playback GStreamer bins on demand, manage playback lifecycle, and coordinate between recording and playback.

**Public Interface**:

```c
// Palindrome playback state machine
typedef enum {
    PLAYBACK_STATE_FORWARD,
    PLAYBACK_STATE_REVERSE,
} PlaybackDirection;

typedef struct {
    RingBuffer *buffer;
    guint current_frame;
    PlaybackDirection direction;
    guint total_frames;
    gboolean is_playing;
} PlaybackLoop;

// Create playback loop from recorded buffer
PlaybackLoop* playback_loop_create(RingBuffer *recorded_buffer);

// Advance playback by one frame (called by playback bin on each frame)
void playback_advance_frame(PlaybackLoop *loop);

// Get next frame for rendering
GstBuffer* playback_get_next_frame(PlaybackLoop *loop);

// Create GStreamer bin for playback
GstElement* playback_create_bin(PlaybackLoop *loop, guint output_pad_index);

// Query playback state
PlaybackDirection playback_get_direction(PlaybackLoop *loop);
gboolean playback_is_playing(PlaybackLoop *loop);

// Cleanup
void playback_loop_cleanup(PlaybackLoop *loop);
```

**Responsibilities**:
- Implement palindrome playback algorithm (forward → reverse → repeat)
- Create playback GStreamer bins dynamically when recording ends
- Manage frame index and direction state
- Provide next frame to videomixer for rendering
- Handle loop restart when palindrome cycle completes
- Coordinate timing to achieve 120 fps rendering

**Dependencies**:
- RingBuffer from buffer_manager
- GStreamer videomixer

**Algorithm: Palindrome Playback**:

```
Frame sequence for buffer of N frames:
Frames:    0  1  2  3  4  5  6  7  8  9
Forward:   ►──────────────────────────►
Reverse:                          ◄──────
Output:    0  1  2  3  4  5  6  7  8  9  8  7  6  5  4  3  2  1  0  1  2  3...

State machine:
current_frame = 0, direction = FORWARD
  Loop:
    Output frame[current_frame]
    If direction == FORWARD:
      current_frame++
      If current_frame >= total_frames:
        direction = REVERSE
        current_frame = total_frames - 2  // Don't repeat last frame
    Else (direction == REVERSE):
      current_frame--
      If current_frame < 0:
        direction = FORWARD
        current_frame = 1  // Don't repeat frame 0
```

**Error Handling**:
- Empty buffer: Playback loop not created, cell remains empty
- Playback bin creation fails: Log error, cell shows black
- Frame timing: Use GStreamer's internal clock for synchronization

### 3.7 Keyboard Input Handler

**File**: `src/input/keyboard_handler.c`

**Purpose**: Capture keyboard input (keys 1-9 and Ctrl+C for quit), translate to key events, and dispatch to recording state manager.

**Public Interface**:

```c
// Callback for key events
typedef void (*KeyEventCallback)(int key_number, gboolean is_pressed);

// Initialize keyboard handler
void keyboard_init(KeyEventCallback on_key_event);

// Process key event (called from X11/Wayland or OS X event handler)
void keyboard_on_event(int key_code, gboolean is_pressed);

// Key mappings
typedef struct {
    int key_1;      // GDK_KEY_1
    int key_2;      // GDK_KEY_2
    // ... through key_9
    int key_quit;   // GDK_KEY_Escape or GDK_KEY_c (for Ctrl+C)
} KeyMapping;

KeyMapping* keyboard_get_key_mapping(void);

// Cleanup
void keyboard_cleanup(void);
```

**Responsibilities**:
- Capture keyboard events from GTK+ event handling (or native Cocoa events)
- Detect press/release for keys 1-9
- Detect Ctrl+C or Escape for application quit
- Translate X11/Cocoa keycodes to logical key numbers
- Dispatch press/release events to recording state manager
- Provide low-latency event delivery (<50ms)

**Dependencies**:
- GTK+ (or Cocoa for native macOS event handling)
- GDK keyboard abstraction

**Error Handling**:
- Unknown key code: Ignore silently
- Keyboard event handler not installed: Warning log, continue
- Event dispatch fails: Log error, continue

**Key Mapping**:
```
Physical Key      → Logical Key Number → Maps to Cell
1                 → 1                  → Cell 2
2                 → 2                  → Cell 3
3                 → 3                  → Cell 4
4                 → 4                  → Cell 5
5                 → 5                  → Cell 6
6                 → 6                  → Cell 7
7                 → 7                  → Cell 8
8                 → 8                  → Cell 9
9                 → 9                  → Cell 10
Escape            → Quit               → Exit application
Ctrl+C (terminal) → Quit               → Exit application
```

### 3.8 OS X Window and Rendering

**File**: `src/osx/window.c`

**Purpose**: Create and manage native Cocoa window, integrate with GStreamer osxvideosink, and coordinate Metal/OpenGL rendering.

**Public Interface**:

```c
// OS X window context
typedef struct {
    NSWindow *window;
    NSView *video_view;
    CGFloat cell_width;         // 320 pixels per cell
    CGFloat cell_height;        // Calculated from aspect ratio
    guint grid_cols;            // 10 cells
    guint grid_rows;            // 1 row
} OSXWindow;

// Create window
OSXWindow* window_create(CameraSource *camera, guint num_cells);

// Get GStreamer sink element (osxvideosink)
GstElement* window_get_videosink(OSXWindow *win);

// Resize handling
void window_on_resize(OSXWindow *win, CGFloat width, CGFloat height);

// Aspect ratio (from camera)
void window_set_aspect_ratio(OSXWindow *win, gdouble aspect_ratio);

// Rendering loop integration
void window_request_render(OSXWindow *win);
void window_swap_buffers(OSXWindow *win);

// Cleanup
void window_cleanup(OSXWindow *win);
```

**Responsibilities**:
- Create NSWindow with video view for rendering
- Request and verify camera permission through macOS dialogs
- Configure osxvideosink element for window integration
- Set window title to "Video Looper"
- Position window on screen (use default positioning)
- Handle window closing (triggers app shutdown)
- Coordinate Metal/OpenGL rendering context
- Maintain 120 fps rendering with display link synchronization

**Dependencies**:
- Cocoa framework (NSWindow, NSView)
- Metal or OpenGL for rendering (via osxvideosink)
- GStreamer osxvideosink plugin

**Window Configuration**:
```
- Size: 320px × 10 cells × (aspect_ratio height) = 320×height pixels
- Default aspect ratio: 16:9 (from typical camera), height = 180 pixels
- Window size: 3200 × 180 pixels
- Title: "Video Looper"
- Fullscreen: No
- Resizable: Yes (but grid layout adapts)
- Always on top: No
```

**Error Handling**:
- Camera permission denied: Display system dialog, exit if denied
- Window creation fails: Log error, exit with code
- Rendering context failure: Attempt fallback, or exit

---

## 4. Data Models and Storage

### 4.1 Video Frame Data Model

**Storage Location**: GPU memory (GStreamer GstBuffer)

**Data Structure**:

```c
// Defined in GStreamer
typedef struct {
    // Memory information
    GstMemory **memories;              // GPU memory blocks
    gsize num_memories;

    // Timing information
    GstClockTime pts;                  // Presentation timestamp
    GstClockTime dts;                  // Decode timestamp
    GstClockTime duration;             // Frame duration

    // Format information (via GstCaps)
    gint width;                        // 1920 or negotiated width
    gint height;                       // 1080 or negotiated height
    const gchar *format;               // "BGRx", "RGB", etc (GPU format)
    gint framerate_num;                // 30 (numerator)
    gint framerate_den;                // 1 (denominator) → 30 fps
} GstBuffer;

// Camera format caps
// video/x-raw, width=1920, height=1080, framerate=30/1, format=BGRx
```

**Storage Approach**: In-memory GPU buffers with ring buffer management

- **Live Feed**: Single continuous buffer, replaced every frame
- **Recording Buffers**: Ring buffer of ~60 frames per recording
- **Playback Queues**: Dynamic pools created per playback bin
- **Videomixer Input**: Direct buffer references (no copy)

**Validation Rules**:
- Frame dimensions must match negotiated caps
- Timestamps must be monotonically increasing
- Format must be GPU-compatible (not RGB888, use BGRx or YUV)

### 4.2 Recording State Data Model

**Storage Location**: CPU memory (struct in recorder component)

**Data Structure**:

```c
typedef struct {
    // Per-recording state
    gboolean is_recording[9];          // Is key 1-9 currently recording?
    RingBuffer *buffer[9];             // Recorded frame buffers
    guint64 start_time_us[9];          // Start timestamp (microseconds)
    guint64 duration_us[9];            // Duration of recording
    guint frame_count[9];              // Total frames recorded
    gint cell_index[9];                // Which cell displays this? (0-8)
} RecordingState;

typedef struct {
    // Grid state
    gint next_cell_to_fill;            // 0-8 (maps to cells 2-10)
    PlaybackLoop *playback_loops[9];   // One per potential cell
    gboolean cell_has_content[10];     // True if cell 1-10 has video
} GridState;
```

**Persistence**: None (in-memory only, per PRD)

**Validation Rules**:
- Only keys 1-9 can record
- Only one recording per key at a time
- Cell index must be 0-8 (for cells 2-10)
- Duration must be > 0 microseconds

### 4.3 Application State Data Model

**Storage Location**: CPU memory (application context)

**Data Structure**:

```c
typedef struct {
    // GStreamer context
    GstElement *pipeline;
    GstBus *bus;
    GMainLoop *main_loop;

    // Component state
    CameraSource *camera;
    Pipeline *gst_pipeline;
    RecordingState *recording_state;
    OSXWindow *window;
    PlaybackManager *playback_mgr;

    // Configuration
    guint target_fps;                  // 120
    guint grid_cells;                  // 10
    guint cell_width_px;               // 320
    guint camera_width;                // Negotiated camera width
    guint camera_height;               // Negotiated camera height
    gdouble aspect_ratio;              // width / height

    // Timing
    guint64 startup_time_us;           // When app started
    guint64 last_frame_time_us;        // Timestamp of last rendered frame
} AppContext;
```

**Initialization**: Performed in main() during startup

**Cleanup**: Performed in main() before exit or in atexit handlers

---

## 5. Technology Stack

| Layer | Technology | Version | Purpose | Justification |
|-------|-----------|---------|---------|---------------|
| **Runtime** | GStreamer | 1.20+ | Video pipeline orchestration | Industry standard, GPU-accelerated, macOS support |
| **Camera API** | AVFoundation | 15.7+ | Camera access and permissions | Native macOS API, hardware integration |
| **Window Management** | Cocoa/NSWindow | 15.7+ | OS X window creation and rendering | Native macOS framework, integrated Metal support |
| **GPU Rendering** | Metal or OpenGL | Via GStreamer | Hardware acceleration for video | Available on all modern Macs; GStreamer handles abstraction |
| **Video Sink** | osxvideosink | Via GStreamer | Rendering to Cocoa window | Native GStreamer sink for macOS |
| **Videomixer** | videomixer | Via GStreamer | Grid composition | Standard GStreamer element, GPU acceleration available |
| **Programming Language** | C | C99 | Core application logic | Maximum performance, direct GStreamer API access, minimal overhead |
| **Build System** | Meson | 1.0+ | Compilation and linking | Modern, portable, GStreamer integration built-in |
| **Testing Framework** | GTest | Via GStreamer | Unit testing | GStreamer native, C-compatible |
| **Dependency Manager** | pkg-config | System | Library discovery | Standard for GStreamer and Unix libraries |

**Rationale for Technology Choices**:

- **GStreamer over custom code**: Eliminates need to write GPU pipeline code; provides mature, tested components
- **AVFoundation over QTCapture**: AVFoundation is current macOS standard; QTCapture deprecated
- **Cocoa over GTK+**: Native integration with macOS; GTK+ creates X11 window which is not native on macOS
- **Metal/OpenGL via GStreamer**: GStreamer handles platform abstraction; no need for direct Metal code
- **C over C++**: Lower overhead, direct GStreamer API compatibility, simpler build dependencies
- **Meson over CMake**: Better GStreamer integration, faster builds, simpler syntax

---

## 6. Directory Structure

```
video-looper-osx-5/
├── docs/
│   ├── planning/
│   │   ├── PRD.md                    (Product Requirements Document)
│   │   ├── SDD.md                    (This Software Design Document)
│   │   └── TTL.md                    (Technical Task List - generated next)
│   └── architecture/
│       ├── ARCHITECTURE.md           (Architecture overview and rationale)
│       └── GSTREAMER_PIPELINE.md     (Detailed GStreamer pipeline configuration)
│
├── src/
│   ├── main.c                        (Application entry point, event loop)
│   │
│   ├── app/
│   │   ├── app_context.c             (Application context initialization/cleanup)
│   │   ├── app_context.h
│   │   └── app_error.h               (Error codes and handling)
│   │
│   ├── camera/
│   │   ├── camera_source.c           (Camera initialization, AVFoundation wrapper)
│   │   ├── camera_source.h
│   │   └── camera_permissions.m      (Objective-C wrapper for camera permissions)
│   │
│   ├── gstreamer/
│   │   ├── pipeline_builder.c        (Main pipeline construction)
│   │   ├── pipeline_builder.h
│   │   ├── gst_elements.c            (GStreamer element creation helpers)
│   │   └── gst_elements.h
│   │
│   ├── recording/
│   │   ├── recording_state.c         (Keyboard → recording state machine)
│   │   ├── recording_state.h
│   │   ├── buffer_manager.c          (Ring buffer management for GPU frames)
│   │   └── buffer_manager.h
│   │
│   ├── playback/
│   │   ├── playback_manager.c        (Palindrome playback logic)
│   │   ├── playback_manager.h
│   │   ├── playback_bin.c            (GStreamer bin for playback)
│   │   └── playback_bin.h
│   │
│   ├── input/
│   │   ├── keyboard_handler.c        (Keyboard input capture and routing)
│   │   ├── keyboard_handler.h
│   │   └── key_codes.h               (Key mapping constants)
│   │
│   ├── osx/
│   │   ├── window.c                  (Cocoa window creation and management)
│   │   ├── window.h
│   │   └── window.mm                 (Objective-C++ Cocoa integration)
│   │
│   └── utils/
│       ├── logging.c                 (Centralized logging)
│       ├── logging.h
│       ├── memory.c                  (Memory allocation tracking)
│       ├── memory.h
│       ├── timing.c                  (High-resolution timing utilities)
│       └── timing.h
│
├── test/
│   ├── CMakeLists.txt                (Test build configuration)
│   ├── unit/
│   │   ├── test_recording_state.c    (Unit tests for recording state machine)
│   │   ├── test_buffer_manager.c     (Unit tests for ring buffer)
│   │   ├── test_playback_manager.c   (Unit tests for palindrome playback)
│   │   └── test_keyboard_handler.c   (Unit tests for key input)
│   │
│   ├── integration/
│   │   ├── test_gstreamer_pipeline.c (Integration test: full pipeline startup)
│   │   ├── test_recording_flow.c     (Integration test: record→playback flow)
│   │   └── test_120fps_rendering.c   (Integration test: frame rate validation)
│   │
│   ├── e2e/
│   │   ├── test_app_launch.sh        (E2E test: application startup time)
│   │   ├── test_camera_permission.sh (E2E test: permission handling)
│   │   └── test_grid_display.sh      (E2E test: grid layout verification)
│   │
│   ├── fixtures/
│   │   └── mock_gstreamer.c          (Mock GStreamer elements for testing)
│   │
│   └── meson.build                   (Test configuration)
│
├── build/
│   ├── meson.build                   (Root Meson build configuration)
│   ├── meson_options.txt              (Build options: debug/release, etc)
│   └── [Generated build files]
│
├── third_party/
│   └── [No dependencies; uses system GStreamer]
│
├── scripts/
│   ├── build.sh                      (Build script - runs meson/ninja)
│   ├── run.sh                        (Run script - executes compiled binary)
│   ├── test.sh                       (Test script - runs all tests)
│   ├── profile.sh                    (Performance profiling script)
│   └── format_code.sh                (Code formatting - clang-format)
│
├── .gitignore                        (Git ignore patterns)
├── CMakeLists.txt                    (Alternative: CMake build if preferred)
├── README.md                         (Project overview and getting started)
├── CONTRIBUTING.md                   (Development guidelines)
├── LICENSE                           (MIT or Apache 2.0)
└── meson.build                       (Meson build file for compilation)
```

**Directory Organization Conventions**:

1. **src/**: All source code organized by functional domain (camera, recording, playback, etc.)
2. **test/**: Tests mirroring src/ structure (unit, integration, E2E in subdirectories)
3. **docs/**: All documentation (planning, architecture, API reference)
4. **scripts/**: Build and development scripts
5. **File naming**: snake_case.c for implementations, snake_case.h for headers, test_*.c for tests
6. **Header guards**: `#ifndef FILENAME_H` and `#define FILENAME_H` pattern
7. **Include paths**: Use relative imports within src/, absolute for external libraries

**Build Artifacts**:
- `build/`: Created by Meson, contains compiled binaries and object files
- `build/video-looper`: Final executable (symlink to `build/src/video-looper`)
- All build artifacts in `build/` directory; never committed to git

---

## 7. Error Handling Strategy

### 7.1 Exception/Error Hierarchy

**Custom Error Codes** (not C++ exceptions; uses GLib error handling):

```c
// File: src/app/app_error.h

typedef enum {
    // Application initialization errors
    APP_ERROR_GSTREAMER_INIT_FAILED,        // GStreamer library init failed
    APP_ERROR_WINDOW_CREATE_FAILED,         // OS X window creation failed
    APP_ERROR_CAMERA_NOT_FOUND,             // No built-in camera detected
    APP_ERROR_CAMERA_PERMISSION_DENIED,     // User denied camera access
    APP_ERROR_PIPELINE_BUILD_FAILED,        // Failed to build GStreamer pipeline

    // Runtime errors
    APP_ERROR_CAMERA_DISCONNECTED,          // Camera lost during session
    APP_ERROR_PIPELINE_STATE_CHANGE_FAILED, // Failed to change pipeline state
    APP_ERROR_MEMORY_ALLOCATION_FAILED,     // malloc/calloc failed
    APP_ERROR_RECORDING_BUFFER_FULL,        // Recording buffer exhausted
    APP_ERROR_KEYBOARD_HANDLER_FAILED,      // Keyboard input handler setup failed

    // Warning/non-fatal errors
    APP_WARNING_FRAME_DROP_DETECTED,        // Frames dropped, not meeting 120fps
    APP_WARNING_MEMORY_USAGE_HIGH,          // Memory usage above threshold
} AppErrorCode;

// GLib-style error structure
typedef struct {
    AppErrorCode code;
    gchar *message;
    const gchar *function;
    const gchar *file;
    guint line;
    gpointer user_data;
} AppError;

// Error callback type
typedef void (*AppErrorCallback)(AppError *error, gpointer user_data);

// Register error callback
void app_register_error_handler(AppErrorCallback handler);

// Log and handle error
void app_log_error(AppErrorCode code, const gchar *format, ...);
void app_log_warning(AppErrorCode code, const gchar *format, ...);

// Retrieve last error
AppError* app_get_last_error(void);
```

### 7.2 Error Propagation

**Strategy**: Errors propagate from component to main application loop

```
Low-level error (e.g., GStreamer bus error)
    ↓
Component error handler
    ↓
Main error callback (app_register_error_handler)
    ↓
Log to stderr
    ↓
If fatal: Trigger shutdown
If recoverable: Attempt recovery or continue
```

**Example Error Flow**:

```c
// In pipeline_builder.c
gboolean pipeline_set_state(Pipeline *p, GstState state) {
    GstStateChangeReturn ret = gst_element_set_state(p->pipeline, state);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        app_log_error(APP_ERROR_PIPELINE_STATE_CHANGE_FAILED,
                     "Failed to set pipeline to %s state",
                     gst_element_state_get_name(state));
        return FALSE;  // Propagate to caller
    }

    return TRUE;
}

// In main.c
int main(int argc, char *argv[]) {
    // ... initialization ...

    if (!pipeline_set_state(pipeline, GST_STATE_PLAYING)) {
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED,
                     "Failed to start pipeline");
        return 1;  // Exit with error code
    }

    // ... run main loop ...
}
```

### 7.3 Error Handling by Category

| Category | Error | Handling | Recovery |
|----------|-------|----------|----------|
| **Initialization** | Camera not found | Log error, display dialog, exit(1) | None - fatal |
| **Initialization** | Permission denied | Log error, display system dialog, exit(1) | None - fatal |
| **Initialization** | GStreamer init failed | Log error, exit(2) | None - fatal |
| **Initialization** | Window creation failed | Log error, exit(3) | None - fatal |
| **Runtime** | Camera disconnected | Log warning, stop recording, show message | Reconnect on next key press |
| **Runtime** | Frame drop detected | Log warning, attempt to reduce processing | Continue at lower FPS if needed |
| **Runtime** | Memory allocation failed | Log error, attempt cleanup, exit | None - attempt immediate exit |
| **Runtime** | Recording buffer full | Log warning, drop oldest frame, continue | Continue with new recording in next cell |
| **Runtime** | Keyboard handler failure | Log warning, continue without keyboard | Try to reinitialize on next event |

### 7.4 Logging Approach

**Logging Levels**: DEBUG, INFO, WARNING, ERROR

**Output**: stderr (for terminal applications)

```c
// File: src/utils/logging.c

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
} LogLevel;

// Set global log level (default: INFO in release, DEBUG in debug build)
void logging_set_level(LogLevel level);

// Log a message
void logging_log(LogLevel level, const gchar *category, const gchar *format, ...);

// Convenience macros
#define LOG_DEBUG(fmt, ...)    logging_log(LOG_LEVEL_DEBUG, __func__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)     logging_log(LOG_LEVEL_INFO, __func__, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...)  logging_log(LOG_LEVEL_WARNING, __func__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)    logging_log(LOG_LEVEL_ERROR, __func__, fmt, ##__VA_ARGS__)
```

**Example Log Output**:

```
[2026-01-27 10:15:32.123] [INFO] main: Video Looper starting...
[2026-01-27 10:15:32.145] [DEBUG] camera_source: Requesting camera permission
[2026-01-27 10:15:33.200] [INFO] camera_source: Camera connected (1920x1080 @ 30fps)
[2026-01-27 10:15:33.500] [INFO] pipeline_builder: GStreamer pipeline built successfully
[2026-01-27 10:15:33.600] [INFO] main: Application ready, waiting for input
[2026-01-27 10:15:40.100] [DEBUG] recording_state: Key 1 pressed, starting record
[2026-01-27 10:15:42.200] [DEBUG] recording_state: Key 1 released, stopping record (duration: 2100ms)
[2026-01-27 10:15:42.300] [INFO] playback_manager: Created playback loop for cell 2 (63 frames)
```

### 7.5 User Feedback and Recovery

**Visual/Auditory Feedback**:

```c
// Recording state feedback
// When recording starts:
//   - Cell border flashes (visual indicator)
//   - Brief system beep (audio feedback)
// When recording stops:
//   - Cell border stops flashing
//   - Playback begins immediately

// Error feedback
// Camera permission denied:
//   - System dialog appears (native macOS)
//   - Error message logged to console
//   - Application exits gracefully

// Camera disconnected during session:
//   - Log warning
//   - Live feed freezes on last frame
//   - Recording attempts cause no-op
//   - Message on console
```

**Recovery Mechanisms**:

1. **Transient Errors** (frame drops, temporary disconnect):
   - Log warning, continue operation
   - Automatic retry on next input

2. **Recoverable Errors** (keyboard handler failure):
   - Log error, attempt to reinitialize
   - User can continue with application

3. **Fatal Errors** (camera not found, permissions denied):
   - Log error
   - Display error dialog (system-level)
   - Exit with error code (1-3)
   - No recovery attempted

---

## 8. Testing Strategy

### 8.1 Test Architecture

**Testing Pyramid**:
```
                    ▲
                   ╱ ╲
                  ╱   ╲ E2E Tests (5 tests)
                 ╱     ╲ App launch, grid display, etc.
                ╱───────╲
               ╱         ╲
              ╱           ╲ Integration Tests (8 tests)
             ╱             ╲ Pipeline building, recording→playback flow, FPS measurement
            ╱───────────────╲
           ╱                 ╲
          ╱                   ╲ Unit Tests (25 tests)
         ╱                     ╲ Recording state, buffer mgmt, playback logic, keyboard input
        ╱─────────────────────────╲
```

**Coverage Targets**:
- Unit Tests: 85% code coverage (critical components)
- Integration Tests: 60% coverage (pipeline interactions)
- E2E Tests: Core user stories (record, playback, grid display)

### 8.2 Unit Tests

**Test Framework**: GTest (C testing framework from GStreamer)

**Test Files**:
- `test/unit/test_recording_state.c` - Recording state machine logic
- `test/unit/test_buffer_manager.c` - Ring buffer allocation and access
- `test/unit/test_playback_manager.c` - Palindrome playback algorithm
- `test/unit/test_keyboard_handler.c` - Key press/release event handling

**Example Unit Test**:

```c
// File: test/unit/test_playback_manager.c

#include <gtest/gtest.h>
#include "playback/playback_manager.h"

// Test fixture
typedef struct {
    RingBuffer *buffer;
    PlaybackLoop *loop;
} PlaybackTestFixture;

// Setup
void setup_playback_test(PlaybackTestFixture *f) {
    // Create mock buffer with 10 frames
    f->buffer = buffer_create(10, NULL);  // NULL caps for testing
    for (int i = 0; i < 10; i++) {
        GstBuffer *frame = gst_buffer_new();
        buffer_write_frame(f->buffer, frame);
    }

    f->loop = playback_loop_create(f->buffer);
}

// Teardown
void teardown_playback_test(PlaybackTestFixture *f) {
    playback_loop_cleanup(f->loop);
    buffer_cleanup(f->buffer);
}

// Test 1: Palindrome playback sequence
TEST(PlaybackManager, PalindromeSequence) {
    PlaybackTestFixture f;
    setup_playback_test(&f);

    // Expected sequence: 0,1,2,3,4,5,6,7,8,9,8,7,6,5,4,3,2,1,0,1...
    guint expected[] = {0,1,2,3,4,5,6,7,8,9,8,7,6,5,4,3,2,1,0,1};

    for (int i = 0; i < 20; i++) {
        guint frame_idx = 0;
        gboolean reverse = FALSE;
        GstBuffer *frame = playback_get_next_frame(f.loop);

        EXPECT_EQ(frame_idx, expected[i]);
        playback_advance_frame(f.loop);
    }

    teardown_playback_test(&f);
}

// Test 2: Direction changes at boundaries
TEST(PlaybackManager, DirectionChange) {
    PlaybackTestFixture f;
    setup_playback_test(&f);

    // Advance to end of forward direction
    for (int i = 0; i < 10; i++) playback_advance_frame(f.loop);
    EXPECT_EQ(playback_get_direction(f.loop), PLAYBACK_STATE_REVERSE);

    // Advance to end of reverse direction
    for (int i = 0; i < 10; i++) playback_advance_frame(f.loop);
    EXPECT_EQ(playback_get_direction(f.loop), PLAYBACK_STATE_FORWARD);

    teardown_playback_test(&f);
}

// Run all tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
```

**Unit Test Coverage**:

| Component | Test File | Tests | Coverage Target |
|-----------|-----------|-------|-----------------|
| Recording State | test_recording_state.c | 8 tests | 90% |
| Buffer Manager | test_buffer_manager.c | 7 tests | 85% |
| Playback Manager | test_playback_manager.c | 6 tests | 80% |
| Keyboard Handler | test_keyboard_handler.c | 4 tests | 75% |
| **Total Unit Tests** | **4 files** | **25 tests** | **85%** |

### 8.3 Integration Tests

**Test Framework**: GStreamer testing utilities + custom drivers

**Test Files**:
- `test/integration/test_gstreamer_pipeline.c` - Full pipeline startup and state transitions
- `test/integration/test_recording_flow.c` - Record and playback flow (no actual video)
- `test/integration/test_120fps_rendering.c` - Frame rate measurement and stability

**Example Integration Test**:

```c
// File: test/integration/test_gstreamer_pipeline.c

#include <gst/gst.h>
#include "gstreamer/pipeline_builder.h"

// Test fixture with real GStreamer pipeline
typedef struct {
    Pipeline *pipeline;
    CameraSource *camera;
} PipelineTestFixture;

void setup_pipeline_test(PipelineTestFixture *f) {
    gst_init(NULL, NULL);

    // Create mock camera (no actual hardware)
    f->camera = camera_source_init();

    // Build pipeline
    f->pipeline = pipeline_create(f->camera);
}

void teardown_pipeline_test(PipelineTestFixture *f) {
    pipeline_cleanup(f->pipeline);
    camera_source_cleanup(f->camera);
    gst_deinit();
}

// Test 1: Pipeline initialization
TEST(GStreamerPipeline, Initialization) {
    PipelineTestFixture f;
    setup_pipeline_test(&f);

    EXPECT_NE(f.pipeline->pipeline, nullptr);
    EXPECT_EQ(pipeline_get_state(f.pipeline), GST_STATE_NULL);

    teardown_pipeline_test(&f);
}

// Test 2: Pipeline state transitions
TEST(GStreamerPipeline, StateTransitions) {
    PipelineTestFixture f;
    setup_pipeline_test(&f);

    // NULL → READY
    EXPECT_TRUE(pipeline_set_state(f.pipeline, GST_STATE_READY));
    EXPECT_EQ(pipeline_get_state(f.pipeline), GST_STATE_READY);

    // READY → PLAYING
    EXPECT_TRUE(pipeline_set_state(f.pipeline, GST_STATE_PLAYING));
    EXPECT_EQ(pipeline_get_state(f.pipeline), GST_STATE_PLAYING);

    // PLAYING → PAUSED
    EXPECT_TRUE(pipeline_set_state(f.pipeline, GST_STATE_PAUSED));
    EXPECT_EQ(pipeline_get_state(f.pipeline), GST_STATE_PAUSED);

    // PAUSED → READY
    EXPECT_TRUE(pipeline_set_state(f.pipeline, GST_STATE_READY));
    EXPECT_EQ(pipeline_get_state(f.pipeline), GST_STATE_READY);

    teardown_pipeline_test(&f);
}

// Test 3: Recording bin addition/removal
TEST(GStreamerPipeline, RecordingBinLifecycle) {
    PipelineTestFixture f;
    setup_pipeline_test(&f);

    pipeline_set_state(f.pipeline, GST_STATE_READY);

    // Add recording bin for key 1
    pipeline_add_record_bin(f.pipeline, 1);
    EXPECT_NE(f.pipeline->record_bins[0], nullptr);

    // Remove recording bin
    pipeline_remove_record_bin(f.pipeline, 1);
    EXPECT_EQ(f.pipeline->record_bins[0], nullptr);

    teardown_pipeline_test(&f);
}
```

**Integration Test Coverage**:

| Test | File | Purpose |
|------|------|---------|
| Pipeline init | test_gstreamer_pipeline.c | Verify pipeline creates without errors |
| State transitions | test_gstreamer_pipeline.c | NULL→READY→PLAYING→PAUSED→READY |
| Recording bin lifecycle | test_gstreamer_pipeline.c | Add/remove bins during runtime |
| Recording flow | test_recording_flow.c | Simulate: record key→capture frames→playback |
| Playback loop integration | test_recording_flow.c | Verify palindrome playback in pipeline |
| FPS measurement | test_120fps_rendering.c | Measure frame delivery rate |
| FPS stability | test_120fps_rendering.c | Verify fps ±2 for 30-second session |

### 8.4 End-to-End Tests

**Test Framework**: Bash scripts with automated verification

**Test Files**:
- `test/e2e/test_app_launch.sh` - Application startup time and responsiveness
- `test/e2e/test_camera_permission.sh` - Camera permission handling
- `test/e2e/test_grid_display.sh` - Grid layout and cell rendering verification

**Example E2E Test**:

```bash
#!/bin/bash
# File: test/e2e/test_app_launch.sh

set -e

echo "E2E Test: Application Launch Time"

# Compile application
meson compile -C build

# Measure launch time
START_TIME=$(date +%s%N)
timeout 5 ./build/video-looper &
PID=$!

# Wait for window to appear (check for Cocoa window)
sleep 2

END_TIME=$(date +%s%N)
DURATION=$((($END_TIME - $START_TIME) / 1000000))  # Convert to ms

# Kill application
kill $PID 2>/dev/null || true
wait $PID 2>/dev/null || true

# Check launch time < 2 seconds (2000 ms)
if [ $DURATION -lt 2000 ]; then
    echo "✓ PASS: Application launched in ${DURATION}ms (< 2000ms)"
    exit 0
else
    echo "✗ FAIL: Application launch took ${DURATION}ms (> 2000ms)"
    exit 1
fi
```

**E2E Test Coverage**:

| Test | File | Success Criteria |
|------|------|------------------|
| Launch time | test_app_launch.sh | Launches in < 2 seconds |
| Launch responsiveness | test_app_launch.sh | Accepts keyboard input within 500ms |
| Permission handling | test_camera_permission.sh | Shows dialog when permission not granted |
| Live feed display | test_grid_display.sh | Live feed appears in cell 1 |
| Grid layout | test_grid_display.sh | 10 cells displayed in horizontal row |
| Grid dimensions | test_grid_display.sh | Each cell 320px wide, correct height |

### 8.5 Running Tests

**Test Execution**:

```bash
# Run all tests
./scripts/test.sh

# Run only unit tests
meson test -C build unit

# Run only integration tests
meson test -C build integration

# Run only E2E tests
bash test/e2e/test_app_launch.sh
bash test/e2e/test_camera_permission.sh
bash test/e2e/test_grid_display.sh

# Run tests with coverage
meson test -C build --coverage
```

**Test Output**:

```
Running tests...

Unit Tests:
  test_recording_state..................PASS (8/8)
  test_buffer_manager...................PASS (7/7)
  test_playback_manager.................PASS (6/6)
  test_keyboard_handler.................PASS (4/4)
Total Unit Tests: 25 passed ✓

Integration Tests:
  test_gstreamer_pipeline...............PASS (4/4)
  test_recording_flow...................PASS (3/3)
  test_120fps_rendering.................PASS (2/2)
Total Integration Tests: 9 passed ✓

E2E Tests:
  test_app_launch.sh....................PASS
  test_camera_permission.sh.............PASS
  test_grid_display.sh..................PASS
Total E2E Tests: 3 passed ✓

Overall: 37 tests passed, 0 failed ✓
Code Coverage: 82% ✓

Test execution completed in 23 seconds.
```

### 8.6 CI/CD Integration

**Test Automation**:
- Tests run on every commit (if CI configured)
- Build and test on macOS 15.7+ (GitHub Actions or equivalent)
- Test results published in CI system
- Coverage reports generated and archived

**GitHub Actions Example** (if using GitHub):

```yaml
# .github/workflows/test.yml
name: Test
on: [push, pull_request]

jobs:
  test:
    runs-on: macos-14
    steps:
      - uses: actions/checkout@v3
      - name: Install dependencies
        run: brew install gstreamer gtk+ meson ninja pkg-config
      - name: Build
        run: ./scripts/build.sh
      - name: Run tests
        run: ./scripts/test.sh
      - name: Upload coverage
        uses: codecov/codecov-action@v3
```

### 8.7 Mocking Strategy

**Mock GStreamer Elements**:

For unit tests, mock the GStreamer pipeline to avoid actual hardware:

```c
// File: test/fixtures/mock_gstreamer.c

#include <gst/gst.h>

// Mock camera source (returns fake buffers)
GstElement* gst_element_factory_make_mock_camera(void) {
    GstElement *source = gst_element_factory_make("appsrc", "mock_camera");

    // Configure appsrc to emit test frames
    g_object_set(G_OBJECT(source),
                 "caps", gst_caps_from_string("video/x-raw,width=1920,height=1080"),
                 "block", FALSE,
                 NULL);

    return source;
}

// Mock videosink (captures frames for verification)
typedef struct {
    GQueue *frames;  // Queue of captured GstBuffer*
} MockVideoSink;

MockVideoSink* mock_videosink_create(void) {
    MockVideoSink *sink = g_new0(MockVideoSink, 1);
    sink->frames = g_queue_new();
    return sink;
}

void mock_videosink_cleanup(MockVideoSink *sink) {
    g_queue_free(sink->frames);
    g_free(sink);
}

GstBuffer* mock_videosink_get_frame(MockVideoSink *sink) {
    return (GstBuffer *)g_queue_pop_head(sink->frames);
}
```

---

## Summary

This Software Design Document provides a complete technical specification for the Video Looper macOS application. It translates the PRD into concrete component designs, data models, and implementation guidelines.

**Key Design Highlights**:

1. **GPU-Accelerated Pipeline**: All video processing on GPU via GStreamer; CPU handles only control flow
2. **Component-Based Architecture**: Separate components for camera, recording, playback, and rendering
3. **MVP-First Design**: Phase 1 delivers single record/playback loop; Phase 2 adds remaining cells
4. **Comprehensive Testing**: Unit (25), integration (9), and E2E (3) tests ensure quality
5. **Error Resilience**: Graceful handling of camera issues, permission denials, and runtime errors
6. **Native macOS Integration**: Direct Cocoa and AVFoundation APIs for optimal performance

**Implementation Ready**: All components, interfaces, data structures, and tests are specified in sufficient detail for developers to begin implementation immediately.

**Next Step**: Generate Technical Task List (TTL) from this SDD to break down implementation into specific, trackable tasks.

---

## Document Information

**Document Version**: 1.0
**Last Updated**: January 27, 2026
**Project**: Video Looper for macOS
**Status**: Complete - Ready for Implementation

