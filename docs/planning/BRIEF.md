# Video Looper

## What
A simple applications that allows the user to use the keyboard to make short video loops of the video stream from the computer's built-in camera

## Key Features
- User opens the application by running and exectuable in the terminal
- When the application starts, it opens the connection to the built in camera (get permissions if necessary) and opens a window where video is rendered
- The video window is a 10 x 1 grid of 320 pixel wide videos; each video has the aspect ratio of the input camera 
- When the app starts, the 1st left most cell in the grid shows the live camera feed
- App allows recording video loops with the 1-9 keys on the keyboard; as user holds a numbber key, video is recorded and when she lets go, recording stops; then the system starts looping the recorded video and displaying the video loop in the corrsponding cell of the 10 x 1 grid in the video window; live feed remains in cell 1; first recording is in cell 2; 2nd recording in cell 3, and so on.
- Videos are looped for palyndrome playback

## Constraints
- Implemented entirely as a GStreamer pipeline
- Uses as few external systems and dependencies as possible; make it as close to vanilla GStreamer as possible
- Video never leaves the GPU; all processing/recording/playback are done on GPU (not CPU)
- 120 fps playback
- App must work on MacOS 15.7 or later; everything should be made for OS X; all of the platform-specific parts of GStreamer (e.g. how windows are opened and video is rendered) should use the OS X specific method
- App must run as a single terminal command

# User Interface
- App is launched from terminal with a single command
- App opens a new window with for rendering video, with a 10 x 1 grid of videos
- Live input in the first cell 
- Video loops in the remaining cells of the grid

## Success
- All operations work correctly
- Clean, intuitive GUI layout

