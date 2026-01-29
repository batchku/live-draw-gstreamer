#ifndef PIPELINE_BUILDER_H
#define PIPELINE_BUILDER_H

#include <gst/gst.h>
#include "../app/app_config.h"

/**
 * Callback function type for pipeline bus messages.
 *
 * @param type       Message type (error, warning, info, state_changed)
 * @param message    Descriptive message string
 */
typedef void (*PipelineMessageCallback)(const char *type, const char *message);

/**
 * Pipeline structure containing all GStreamer elements for the video looper.
 *
 * Manages the main GStreamer pipeline with:
 * - Camera source input
 * - Live queue for cell 1 (GPU memory buffer)
 * - 20 record bins (one per key 1-20)
 * - 20 playback bins (created dynamically)
 * - Videomixer compositor for 11x2 grid layout with proper pad configuration:
 *   * Live feed: row 0, col 0 (xpos=0, ypos=0, zorder=0)
 *   * Layers 1-10: row 0, cols 1-10
 *   * Layers 11-20: row 1, cols 1-10
 * - osxvideosink for window rendering
 *
 * Note: record_bins are opaque pointers to RecordBin structures
 */
typedef struct {
    GstElement *pipeline;           /**< Main GStreamer pipeline element */
    GstElement *camera_source;      /**< Camera source element (avfvideosrc) */
    GstElement *live_queue;         /**< Queue for live feed (cell 1) */
    GstElement *live_caps;          /**< Capsfilter for live feed format */
    GstElement *live_tee;           /**< Tee element to split live stream */
    gpointer record_bins[TOTAL_LAYERS];        /**< Record bins for keys 1-20 */
    GstElement *playback_bins[TOTAL_LAYERS];   /**< Playback bins (created dynamically) */
    GstElement *playback_queues[TOTAL_LAYERS]; /**< Queues for playback bins */
    GstElement *preview_bins[TOTAL_LAYERS];    /**< Live preview bins while recording */
    GstPad *preview_tee_pads[TOTAL_LAYERS];    /**< Tee pads for live preview connections */
    GstPad *cell_sink_pads[TOTAL_LAYERS];      /**< Videomixer sink pads for layers */
    GstElement *videomixer;         /**< Videomixer compositor for 11x2 grid */
    GstElement *videoconvert;       /**< Format converter for videomixer → osxvideosink */
    GstElement *composite_caps;     /**< Capsfilter for format conversion to osxvideosink */
    GstElement *osxvideosink;       /**< OS X video sink for window rendering */
    GstBus *bus;                    /**< GStreamer bus for message handling */
    PipelineMessageCallback msg_callback; /**< User-registered message callback */
    gpointer window;                /**< OSXWindow pointer for video overlay (opaque) */
} Pipeline;

/**
 * Create and initialize the main GStreamer pipeline.
 *
 * This function constructs the core pipeline with:
 * - Camera source connected to a tee element
 * - Live queue branching to videomixer input pad 0 (cell 1)
 * - Nine record bin connection points on tee output (initially unused)
 * - Videomixer configured for 10-cell grid (1 row × 10 columns) with proper pad configuration:
 *   * Cell 1 (pad 0): Live feed at xpos=0, zorder=0 (background)
 *   * Cells 2-10 (pads 1-9): Pre-allocated with xpos=320-2880, zorder=1-9
 *   * All pads: ypos=0, width=320 pixels, alpha=1.0 (fully opaque)
 * - Composite capsfilter and osxvideosink for rendering
 *
 * Videomixer Pad Configuration Details:
 * - Total grid width: 3200 pixels (320 × 10 cells)
 * - Each cell: 320 pixels wide, aspect ratio height
 * - zorder controls layering (0=background, 9=foreground)
 * - All sink pads are pre-configured during pipeline creation
 * - Playback bins will link to pre-allocated pads as needed
 *
 * @param camera_source_element  Pre-initialized camera source GStreamer element
 * @return                        Pointer to Pipeline struct on success, NULL on failure
 *
 * Error handling:
 * - If element creation fails, logs error and returns NULL
 * - If element linking fails, unrefs all elements and returns NULL
 * - If sink pad request fails for any cell, logs warning but continues
 * - If state change to READY fails, returns NULL
 *
 * The caller is responsible for calling pipeline_cleanup() to free resources.
 */
Pipeline *pipeline_create(GstElement *camera_source_element);

/**
 * Add a recording bin to the pipeline at runtime.
 *
 * Dynamically creates and links a record bin to the tee element for a specific key.
 * Record bins are initially idle and start capturing when recording is triggered.
 *
 * @param p        Pipeline pointer
 * @param key_num  Key number (1-9); maps to record_bins[key_num-1]
 * @return         TRUE if bin added successfully, FALSE on failure
 */
gboolean pipeline_add_record_bin(Pipeline *p, int key_num);

/**
 * Remove a recording bin from the pipeline at runtime.
 *
 * Unlinks and removes a record bin from the pipeline.
 * Safe to call even if bin was not previously added.
 *
 * @param p        Pipeline pointer
 * @param key_num  Key number (1-9)
 * @return         TRUE if bin removed successfully, FALSE on failure
 */
gboolean pipeline_remove_record_bin(Pipeline *p, int key_num);

/**
 * Add a playback bin to the pipeline at runtime.
 *
 * Dynamically creates and links a playback bin to the videomixer.
 * Called after a recording completes to display the recorded video loop.
 *
 * @param p           Pipeline pointer
 * @param cell_num    Cell number (2-10, maps to videomixer pad 1-9)
 * @param duration_us Duration of recorded video in microseconds
 * @return            TRUE if bin added successfully, FALSE on failure
 */
gboolean pipeline_add_playback_bin(Pipeline *p, int cell_num, guint64 duration_us);

/**
 * Remove a playback bin from the pipeline at runtime.
 *
 * Unlinks and removes a playback bin from the videomixer.
 *
 * @param p        Pipeline pointer
 * @param cell_num Cell number (2-10)
 * @return         TRUE if bin removed successfully, FALSE on failure
 */
gboolean pipeline_remove_playback_bin(Pipeline *p, int cell_num);

/**
 * Connect live preview to a cell while recording.
 *
 * Shows the live camera feed in the specified cell (2-10) while a key is held.
 * Creates a pipeline branch: tee → queue → videoconvert → videoscale → capsfilter → compositor
 *
 * @param p        Pipeline pointer
 * @param cell_num Cell number (2-10, maps to key_num + 1)
 * @return         TRUE if preview connected successfully, FALSE on failure
 */
gboolean pipeline_connect_live_preview(Pipeline *p, int cell_num);

/**
 * Disconnect live preview from a cell.
 *
 * Removes the live feed from the specified cell when key is released.
 * The cell can then be used for playback.
 *
 * @param p        Pipeline pointer
 * @param cell_num Cell number (2-10)
 * @return         TRUE if preview disconnected successfully, FALSE on failure
 */
gboolean pipeline_disconnect_live_preview(Pipeline *p, int cell_num);

/**
 * Set the GStreamer pipeline state.
 *
 * Changes the pipeline state (NULL, READY, PAUSED, PLAYING).
 * Handles state change returns and logs warnings for async state changes.
 *
 * @param p     Pipeline pointer
 * @param state Target GStreamer state (GST_STATE_READY, GST_STATE_PLAYING, etc.)
 * @return      TRUE if state change succeeded, FALSE on failure
 */
gboolean pipeline_set_state(Pipeline *p, GstState state);

/**
 * Get the current pipeline state.
 *
 * @param p Pipeline pointer
 * @return  Current GStreamer pipeline state
 */
GstState pipeline_get_state(Pipeline *p);

/**
 * Set the window for video overlay.
 *
 * Associates the pipeline with an OSXWindow so that the osxvideosink
 * can render to the window's NSView when prepare-window-handle messages arrive.
 *
 * @param p      Pipeline pointer
 * @param window OSXWindow pointer (opaque)
 */
void pipeline_set_window(Pipeline *p, gpointer window);

/**
 * Register a callback for pipeline bus messages.
 *
 * The callback will be invoked for ERROR, WARNING, INFO, and STATE_CHANGED messages.
 * Only one callback can be registered at a time.
 *
 * @param p  Pipeline pointer
 * @param cb Callback function pointer (NULL to unregister)
 */
void pipeline_set_message_callback(Pipeline *p, PipelineMessageCallback cb);

/**
 * Clean up and destroy the pipeline.
 *
 * Stops the pipeline, unlinks all elements, unrefs them, and frees the Pipeline struct.
 * Safe to call multiple times (checks for NULL pointers).
 *
 * @param p Pipeline pointer (may be NULL)
 */
void pipeline_cleanup(Pipeline *p);

#endif // PIPELINE_BUILDER_H
