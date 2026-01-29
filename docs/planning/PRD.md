# Product Requirements Document: Video Looper for macOS

## 1. Executive Summary

### Vision Statement
Video Looper is a lightweight macOS application that enables users to create and manage short video loops from their built-in camera feed in real-time. The application provides an intuitive keyboard-driven interface for recording, looping, and displaying multiple video streams simultaneously in a grid layout.

### Target Users and Personas
- **Primary User**: Content creators and performers who want to capture and loop short video segments during live performances or creative sessions
- **Secondary User**: Developers and technical users who prefer terminal-based, GPU-accelerated applications

### Value Proposition
Video Looper delivers a minimal, efficient solution for real-time video looping without external dependencies. By leveraging GStreamer's GPU acceleration, the application provides smooth, responsive video recording and palindrome playback with zero CPU overhead for video processing. The terminal-based launch and single-command execution make it accessible to power users and developers.

---

## 2. Goals and Objectives

### User Goals
- Create short video loops by pressing and holding keyboard keys
- View multiple video loops simultaneously in an organized grid display
- Maintain a live camera feed alongside recorded video loops
- Experience responsive, low-latency video recording and playback

### Technical Goals
- Implement a robust GStreamer pipeline for GPU-accelerated video processing
- Achieve 120 fps video playback across all grid cells
- Support macOS 15.7 or later with native OS X rendering and camera access
- Eliminate external system dependencies and maximize vanilla GStreamer usage
- Maintain video processing entirely on GPU to minimize CPU load

---

## 3. User Stories

As a user, I want to start the application with a single terminal command so that I can quickly begin recording video loops.

**Acceptance Criteria:**
- Application can be launched by running a single executable command in the terminal
- System requests camera permissions from the operating system if not previously granted
- A new window opens within 2 seconds of command execution
- The system establishes connection to the built-in camera automatically
- Application remains responsive and ready to accept keyboard input

---

As a user, I want to see the live camera feed in the first cell of the grid immediately upon startup so that I can monitor what is being recorded.

**Acceptance Criteria:**
- Live camera feed renders in the leftmost cell (cell 1) of the 10 x 1 grid
- Video stream appears immediately after camera connection is established
- Live feed remains visible and updates at 120 fps throughout application runtime
- Live feed continues to stream even while recording video loops in other cells
- Video in the first cell displays with correct aspect ratio from the input camera

---

As a user, I want to record video loops by holding down a number key (1-9) on the keyboard so that recording starts and stops based on how long I press the key.

**Acceptance Criteria:**
- Pressing and holding keys 1-9 initiates video recording
- Recording continues while the key is held down
- Recording stops immediately when the key is released
- Each number key maps to a corresponding cell in the grid (key 1 = cell 2, key 2 = cell 3, etc.)
- Multiple simultaneous key presses do not cause conflicts or missed recordings
- System provides clear feedback that recording is active

---

As a user, I want recorded video loops to be displayed in their corresponding grid cells with continuous palindrome playback so that I can see multiple loops running simultaneously.

**Acceptance Criteria:**
- First recorded video loop displays in cell 2 of the 10 x 1 grid
- Second recorded video loop displays in cell 3, and subsequent recordings follow sequentially
- All video loops play using palindrome playback (forward then reverse)
- Video loops restart automatically after completing a palindrome cycle
- Video playback remains at 120 fps across all grid cells
- Video loops maintain correct aspect ratio from the original recording
- Live feed in cell 1 remains unaffected by loop playback in other cells

---

As a user, I want to view video loops in a well-organized 10 x 1 grid layout so that I can monitor all recordings at once without visual clutter.

**Acceptance Criteria:**
- Application window displays exactly 10 video cells arranged in a single horizontal row
- Each video cell is 320 pixels wide
- Each video cell maintains the aspect ratio of the input camera stream
- Live feed in cell 1 is visually distinct or labeled
- All cells are evenly sized and aligned
- Grid layout is clean and intuitive with minimal visual artifacts

---

As a user, I want the live feed to remain in cell 1 while I work with other cells so that the application state remains stable throughout my session.

**Acceptance Criteria:**
- Live camera feed in cell 1 persists for the entire application session
- Recording in cell 2 does not affect live feed in cell 1
- Adding more video loops does not interrupt existing recordings or playback
- Application remains stable and responsive with multiple loops playing simultaneously

---

## 4. Functional Requirements

### 4.1 Application Launch and Window Management

**Requirement 4.1.1: Single Command Execution**
The application must run as a single terminal command with no additional arguments or configuration required. The executable shall be built and deployed such that users can start the application from any directory by invoking the command.

**Requirement 4.1.2: OS X Window Opening**
Upon successful launch, the application shall open a dedicated window for video rendering using OS X-specific window management APIs. The window shall remain visible and active for the entire duration of the application session.

**Requirement 4.1.3: Camera Permission Handling**
The application shall request and handle camera permissions automatically. If permissions have not been granted, the system shall present the native macOS permission dialog. If the user denies permissions, the application shall display an appropriate error message and exit gracefully.

### 4.2 Camera Input and Live Stream

**Requirement 4.2.1: Built-In Camera Connection**
The application shall automatically detect and connect to the computer's built-in camera upon startup. The system shall negotiate camera format and resolution with the built-in camera hardware.

**Requirement 4.2.2: Live Feed Rendering**
The live camera feed shall be rendered continuously in cell 1 of the video grid at 120 fps. The video stream shall begin rendering immediately after the camera connection is successfully established.

**Requirement 4.2.3: Aspect Ratio Preservation**
The live feed and all video loops shall maintain the native aspect ratio of the input camera stream. Each cell shall display video proportionally scaled to fit within its 320-pixel width while preserving the camera's original aspect ratio.

### 4.3 Video Grid Layout and Display

**Requirement 4.3.1: Grid Dimensions**
The application shall display a grid containing exactly 10 video cells arranged in a single horizontal row (10 x 1 layout). Cell 1 (leftmost) shall always display the live camera feed.

**Requirement 4.3.2: Cell Sizing**
Each video cell shall be exactly 320 pixels wide. The height of each cell shall be calculated based on the camera's native aspect ratio to ensure proper video display without stretching or distortion.

**Requirement 4.3.3: Grid Rendering Performance**
The entire grid, including all 10 cells and all video loops, shall render at a minimum of 120 fps. Video playback shall remain smooth even when all 10 cells are actively displaying content.

### 4.4 Keyboard Input and Video Recording

**Requirement 4.4.1: Keyboard Input Mapping**
The application shall accept input from keyboard keys 1 through 9 on the primary keyboard. Each key shall map to a corresponding cell in the grid: key 1 maps to cell 2, key 2 maps to cell 3, continuing through key 9 mapping to cell 10.

**Requirement 4.4.2: Hold-to-Record Mechanism**
When a user presses and holds a number key, video recording shall begin immediately. Recording shall continue for the duration that the key is held. When the user releases the key, recording shall stop immediately and the recorded video loop shall enter playback mode.

**Requirement 4.4.3: Multiple Simultaneous Recordings**
The system shall support multiple number keys being held simultaneously. If multiple keys are pressed at the same time, each shall record independently to its corresponding cell without conflict or interference.

**Requirement 4.4.4: Recording State Feedback**
The application shall provide visual or auditory feedback indicating that recording is active. This feedback shall be clear enough for the user to know when recording has started and stopped.

### 4.5 Video Loop Recording and Storage

**Requirement 4.5.1: Loop Storage**
When a user releases a number key, the recorded video shall be stored in memory as a looping video asset. The system shall maintain the recorded video for the entire application session.

**Requirement 4.5.2: Palindrome Playback**
All recorded video loops shall play using palindrome playback, meaning the video plays forward from start to end, then immediately plays backward from end to start, then repeats. This creates a smooth, seamless looping effect without visible jumps or artifacts.

**Requirement 4.5.3: Loop Management**
The first recorded video loop shall display in cell 2. The second recorded video loop shall display in cell 3. Subsequent recordings shall fill cells 4 through 10 in sequential order. If the user records a new video after cell 10 is filled, the oldest loop shall be replaced by the new recording.

### 4.6 GPU-Accelerated Video Processing

**Requirement 4.6.1: GPU Processing Pipeline**
All video capture, recording, loop processing, and playback shall occur entirely on the GPU. No video frames shall be transferred to CPU memory during recording, playback, or grid rendering.

**Requirement 4.6.2: Performance Target**
The application shall achieve and maintain 120 fps video playback across all grid cells simultaneously. Frame drops or stuttering shall not occur during normal operation.

**Requirement 4.6.3: GStreamer Implementation**
The entire video pipeline shall be implemented using GStreamer with a focus on vanilla GStreamer elements. Custom or proprietary video processing shall be minimized in favor of standard GStreamer components.

### 4.7 Platform-Specific Implementation

**Requirement 4.7.1: macOS Compatibility**
The application shall support macOS version 15.7 or later. The application shall use OS X-specific implementations for all platform-dependent functionality, including camera access, window management, and GPU rendering.

**Requirement 4.7.2: OS X Video Rendering**
Video rendering shall use OS X-specific GStreamer video sink elements to display video in the application window. The rendering approach shall integrate with native macOS window management and display systems.

**Requirement 4.7.3: Native Camera Access**
Camera access shall use the native macOS camera API to obtain video from the built-in camera. The implementation shall respect macOS security and privacy requirements for camera access.

### 4.8 Application State and Stability

**Requirement 4.8.1: Live Feed Persistence**
The live camera feed in cell 1 shall remain visible and active throughout the entire application session. Recording, playback, or other operations in other cells shall not interrupt or affect the live feed.

**Requirement 4.8.2: Graceful Error Handling**
If the camera becomes unavailable, loses connection, or encounters an error, the application shall handle the error gracefully. The application shall remain responsive and shall not crash or freeze.

**Requirement 4.8.3: Application Responsiveness**
The application shall remain responsive to keyboard input at all times. Input processing shall not be delayed by video processing or rendering operations.

### 4.9 Keyboard Control and Recording Permissions

**Requirement 4.9.1: Allow Keyboard-Driven Recording**
The application shall allow users to initiate video recording through keyboard input without requiring mouse interaction or GUI button clicks. All recording operations shall be accessible exclusively through keyboard commands.

**Requirement 4.9.2: Apply Recording State Transitions**
The application shall apply state transitions for recording (idle, recording, playback) based on keyboard key press and release events. State changes shall be instantaneous and shall not introduce latency in user feedback.

---

## 5. Non-Functional Requirements

### 5.1 Performance

**Performance 5.1.1: Frame Rate Target**
The application shall maintain a minimum of 120 fps video playback across all 10 grid cells simultaneously. This requirement applies when displaying the live feed in cell 1 plus multiple video loops in cells 2-10.

**Performance 5.1.2: Input Latency**
Keyboard input shall be processed and reflected in video recording state within 50 milliseconds of key press or release. Users shall perceive recording as starting and stopping instantaneously.

**Performance 5.1.3: GPU Utilization**
All video processing shall occur on the GPU. CPU utilization for video processing shall be minimal (less than 5% of a single CPU core). The application shall not transfer video frames between GPU and CPU memory.

**Performance 5.1.4: Memory Efficiency**
The application shall support storing up to 9 video loops simultaneously without significant performance degradation. Video loop storage shall be efficient and shall not consume excessive RAM.

### 5.2 Reliability

**Reliability 5.2.1: Uptime**
The application shall remain stable and operational for extended sessions (multiple hours of continuous use). Crashes, freezes, or unexpected terminations shall not occur during normal operation.

**Reliability 5.2.2: Error Recovery**
If a temporary error occurs (e.g., brief camera disconnect), the application shall recover gracefully. Upon recovery, the application shall resume normal operation without user intervention.

**Reliability 5.2.3: Resource Management**
The application shall not leak memory or GPU resources during extended operation. Resource usage shall remain stable over time even with repeated recording and playback operations.

### 5.3 Compatibility

**Compatibility 5.3.1: macOS Version Support**
The application shall be fully compatible with macOS 15.7 and all later versions. The application shall not use deprecated APIs or features.

**Compatibility 5.3.2: Camera Hardware**
The application shall work with the standard built-in camera on all modern Mac computers (MacBook, iMac, Mac mini with compatible hardware). The application shall adapt to different camera resolutions and aspect ratios.

**Compatibility 5.3.3: Minimal Dependencies**
The application shall use standard GStreamer libraries and macOS system libraries. Third-party dependencies shall be minimized. All external dependencies shall be open-source and widely available.

### 5.4 Accessibility

**Accessibility 5.4.1: Keyboard Navigation**
The application shall be fully controllable via keyboard. All video recording and playback operations shall be achievable without mouse or touchpad input.

**Accessibility 5.4.2: Visual Clarity**
The application window and grid layout shall be clear and easily readable. Video cells shall be visually distinct from one another.

### 5.5 Security

**Security 5.5.1: Camera Permissions**
The application shall respect macOS security and privacy requirements for camera access. The application shall not bypass or circumvent system camera permission controls.

**Security 5.5.2: Local Operation**
The application shall operate entirely locally on the user's machine. No data shall be transmitted to external servers or networks. All video processing and storage shall remain on the local system.

---

## 6. Technical Constraints

### Technical Constraint 6.1: GStreamer Pipeline Architecture
The application must be implemented entirely as a GStreamer pipeline. All video capture, recording, loop management, and playback shall use GStreamer elements and components. Custom video processing code shall be minimized in favor of standard GStreamer elements.

### Technical Constraint 6.2: Minimal External Dependencies
The application shall use as few external systems and dependencies as possible. The implementation shall favor vanilla GStreamer elements over proprietary or specialized libraries. External library usage shall be justified and documented.

### Technical Constraint 6.3: GPU-Only Video Processing
All video frames shall remain on the GPU throughout their lifecycle. Video shall not be transferred to CPU memory for processing, storage, or rendering. All pipeline stages (capture, recording, looping, playback, grid rendering) shall operate exclusively on GPU.

### Technical Constraint 6.4: macOS and OS X Specific Implementation
The application must target macOS 15.7 or later. All platform-specific functionality shall use OS X-specific methods. GStreamer videosink elements shall be OS X-compatible. Window management shall use native macOS APIs.

### Technical Constraint 6.5: Single Terminal Command Execution
The application must be executable as a single command from the terminal. No additional configuration files, arguments, or setup steps shall be required. The user shall be able to run the application immediately after building.

### Technical Constraint 6.6: 120 FPS Playback Requirement
The video rendering pipeline must support and maintain 120 fps video playback. This applies to the live stream in cell 1 and all video loops in cells 2-10 simultaneously.

---

## 7. Assumptions and Dependencies

### 7.1 Assumptions

**Assumption 7.1.1**: Users have macOS 15.7 or later installed on their computer.

**Assumption 7.1.2**: The target computer has a built-in camera that is functional and accessible.

**Assumption 7.1.3**: The target computer has a GPU capable of supporting GStreamer video processing and 120 fps rendering.

**Assumption 7.1.4**: Users have basic familiarity with terminal commands and can execute shell commands.

**Assumption 7.1.5**: GStreamer libraries and standard macOS system libraries are available on the deployment system.

### 7.2 External Dependencies

**External Dependency 7.2.1**: GStreamer framework and core plugins must be installed and available on the macOS system. The application requires GStreamer for all video processing operations.

**External Dependency 7.2.2**: macOS system libraries (Camera framework, AVFoundation, etc.) must be available for camera access and window rendering.

**External Dependency 7.2.3**: OpenGL or Metal frameworks for GPU acceleration must be available on the target macOS version.

### 7.3 Internal Dependencies

**Internal Dependency 7.3.1**: The build system must be capable of compiling a GStreamer-based application for macOS.

### 7.4 Risks and Potential Blockers

**Risk 7.4.1**: GStreamer GPU acceleration on macOS may have platform-specific limitations or compatibility issues. This requires early prototyping and validation.

**Risk 7.4.2**: Achieving consistent 120 fps playback across 10 simultaneous video streams may require GPU hardware of sufficient capability. Older Mac models may not meet this requirement.

**Risk 7.4.3**: Palindrome playback implementation in a GStreamer pipeline may require custom element development if standard GStreamer elements do not support this feature natively.

**Risk 7.4.4**: macOS camera permission handling and security policies may introduce complexity in camera initialization and error handling.

---

## 8. Out of Scope

### Features Not Included in Initial Release

**Out of Scope 8.1**: Video export or saving functionality. Video loops shall exist only in application memory during the session and shall not persist after application termination.

**Out of Scope 8.2**: Audio capture or playback. The application focuses solely on video looping.

**Out of Scope 8.3**: User interface customization, themes, or settings. The application shall have a fixed grid layout and keyboard control scheme.

**Out of Scope 8.4**: Network streaming or remote camera support. The application shall work only with the built-in local camera.

**Out of Scope 8.5**: Video effects, filters, or post-processing. Video loops shall be displayed as recorded without modification.

**Out of Scope 8.6**: Mouse or touchpad control. The application shall support keyboard input only.

**Out of Scope 8.7**: Multi-camera support. The application shall work with the single built-in camera.

**Out of Scope 8.8**: Undo/redo functionality. Recorded loops cannot be undone; a new recording will replace the old loop.

**Out of Scope 8.9**: Application preferences or configuration files. All behavior shall be determined by default settings.

### Future Considerations

These features may be considered in future releases but are explicitly not part of the current scope:
- Video file export to common formats (MP4, MOV, etc.)
- Audio mixing or sound looping
- Custom grid layouts (different dimensions, variable cell sizes)
- Mouse-based interface or GUI controls
- Multiple camera support
- Video effects and filters
- Cloud storage or streaming integration
- Macros or scripting support

---

## 9. Success Metrics

### 9.1 Functional Success Metrics

**Metric 9.1.1: Application Launch Time**
The application shall launch and display the live camera feed within 2 seconds of command execution.
- **Target**: 100% of launch attempts complete within 2 seconds
- **Measurement**: Automated testing script measures time from command execution to first frame rendered

**Metric 9.1.2: Frame Rate Consistency**
The application shall maintain 120 fps ±2 fps (118-122 fps) across all grid cells during normal operation.
- **Target**: 95% of frames rendered at target frame rate
- **Measurement**: Performance profiling tools measure actual frame delivery rate

**Metric 9.1.3: Recording Accuracy**
Recorded video loops shall be captured accurately without frame loss or corruption.
- **Target**: 100% of recorded frames match source camera frames
- **Measurement**: Frame-by-frame comparison of recorded video against camera input

**Metric 9.1.4: Keyboard Input Responsiveness**
Keyboard input shall be processed and reflected in video recording state within 50 milliseconds.
- **Target**: 95% of input events processed within 50ms latency
- **Measurement**: Input timing logs and performance monitoring

**Metric 9.1.5: Grid Display Stability**
The application shall display all 10 grid cells without visual artifacts, flickering, or rendering errors.
- **Target**: 100% uptime of stable grid display during testing sessions
- **Measurement**: Visual inspection and automated frame analysis

### 9.2 Non-Functional Success Metrics

**Metric 9.2.1: Stability and Uptime**
The application shall remain stable and operational during extended use (minimum 1 hour continuous operation).
- **Target**: 100% stability over 1-hour test sessions
- **Measurement**: Extended runtime testing with automated crash/freeze detection

**Metric 9.2.2: Memory Usage**
Application memory usage shall remain stable during extended operation with multiple video loops.
- **Target**: Memory growth less than 10% over 1-hour session with active recording
- **Measurement**: Memory profiling tools track heap and GPU memory allocation

**Metric 9.2.3: CPU Utilization**
CPU usage for video processing shall remain below 5% of a single core.
- **Target**: 100% of frames processed with CPU video processing below 5%
- **Measurement**: CPU profiling tools measure actual CPU load

**Metric 9.2.4: macOS Compatibility**
The application shall work correctly on all tested macOS versions from 15.7 onward.
- **Target**: 100% compatibility with target macOS versions
- **Measurement**: Testing on multiple macOS versions and hardware configurations

### 9.3 User Experience Metrics

**Metric 9.3.1: Intuitiveness**
Users with basic terminal experience shall understand how to launch and operate the application.
- **Target**: 100% of test users successfully complete basic looping tasks on first attempt
- **Measurement**: User testing with observation and feedback collection

**Metric 9.3.2: Visual Clarity**
The grid layout shall be clear and intuitive with no user confusion about cell organization or video content.
- **Target**: 100% of test users correctly identify which cell contains which content
- **Measurement**: User feedback and observation during testing

### 9.4 Technical Quality Metrics

**Metric 9.4.1: Code Quality**
The implementation shall follow GStreamer best practices and coding standards.
- **Target**: Code review sign-off with no critical issues
- **Measurement**: Peer code review and static analysis tools

**Metric 9.4.2: Dependency Minimization**
External dependencies shall be minimized with preference for vanilla GStreamer.
- **Target**: Fewer than 3 external library dependencies beyond GStreamer
- **Measurement**: Dependency audit and documentation

**Metric 9.4.3: Platform Integration**
The application shall use native macOS and OS X APIs appropriately.
- **Target**: 100% compliance with macOS API usage guidelines
- **Measurement**: Code review against Apple's macOS development guidelines

---

## 10. MVP-First Delivery Strategy

This PRD is structured to support incremental, MVP-first delivery:

### Phase 1: Minimal Viable Product (MVP)
The Phase 1 MVP delivers the core video looping functionality with a single focused feature set:
- Application launches from terminal with single command
- System connects to built-in camera and requests permissions
- Live camera feed displays in cell 1 of the grid
- Grid displays 10 x 1 cells at 320px width each
- User can record video loop by holding a keyboard key
- Video loop plays back using palindrome playback
- All video rendering occurs at 120 fps on GPU
- Application remains stable during 30-minute testing session

**MVP Acceptance Criteria**:
- Successful launch from terminal command
- Live feed visible immediately after launch
- At least one video loop can be recorded and played back
- Grid displays correctly with proper aspect ratio
- Frame rate target achieved (120 fps)
- No crashes during extended operation

### Phase 2+: Feature Completeness (Post-MVP)
Future phases may add additional features:
- Support for multiple simultaneous video loops (cells 2-10)
- Advanced keyboard input handling for edge cases
- Performance optimization for older hardware
- Extended stability testing and error handling improvements
- User experience refinements based on feedback

This MVP-first approach ensures the core product can be validated before investing in full feature completeness.

---

## Document Information

**Document Version**: 1.0
**Last Updated**: January 27, 2026
**Project**: Video Looper for macOS
**Status**: Complete

---
