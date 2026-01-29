/**
 * @file e2e_coordinator.h
 * @brief End-to-end flow coordinator
 *
 * Coordinates the complete recording→buffer→playback→display flow:
 * - Keyboard events → Recording state
 * - Recording start → Record bin signaling
 * - Recording stop → Playback bin creation
 * - Playback bin → Videomixer → Display
 *
 * This module acts as the glue between the keyboard input handler,
 * recording state manager, buffer manager, and playback system.
 */

#ifndef E2E_COORDINATOR_H
#define E2E_COORDINATOR_H

#include <glib.h>

/* Forward declarations - full definitions in respective header files */
struct RingBuffer;
struct PlaybackLoop;

/**
 * Initialize the E2E coordinator
 *
 * Sets up the recording and playback infrastructure and connects
 * the components for the full end-to-end flow.
 *
 * @param app_ctx Application context with initialized pipeline and recording state
 * @return TRUE on success, FALSE on failure
 *
 * Note: AppContext typedef is from app_context.h
 */
gboolean e2e_coordinator_init(void *app_ctx);

/**
 * Handle a keyboard key event in the E2E flow
 *
 * This is the main entry point that orchestrates the complete flow:
 * 1. On key press: Start recording by signaling the record bin
 * 2. On key release: Stop recording and create playback bin
 *
 * @param key_number Key number (1-9) or -1 for quit
 * @param is_pressed TRUE if key was pressed, FALSE if released
 */
void e2e_on_key_event(int key_number, gboolean is_pressed);

/**
 * Get the recording buffer for a specific cell
 *
 * Used by playback bins to access the recorded video frames.
 *
 * @param cell_num Cell number (2-10)
 * @return RingBuffer pointer, or NULL if not found
 */
void *e2e_get_recording_buffer(int cell_num);

/**
 * Cleanup the E2E coordinator
 *
 * Frees all recording buffers and playback state.
 */
void e2e_coordinator_cleanup(void);

#endif /* E2E_COORDINATOR_H */
