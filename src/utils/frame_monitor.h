#ifndef FRAME_MONITOR_H
#define FRAME_MONITOR_H

#include <glib.h>
#include <gst/gst.h>
#include <stdint.h>

/**
 * Frame rate monitoring and drop detection for video pipeline validation.
 *
 * Tracks frame delivery statistics, detects drops, measures frame rate,
 * and provides metrics for performance validation.
 */

/** Frame rate measurement window in frames */
#define FRAME_MONITOR_WINDOW_SIZE 300 /* ~2.5 seconds at 120 fps */

/** Target frame rate in fps */
#define FRAME_MONITOR_TARGET_FPS 120

/** Frame rate tolerance in fps */
#define FRAME_MONITOR_TOLERANCE_FPS 2 /* 120 ±2 fps acceptable */

/**
 * Frame rate statistics
 */
typedef struct {
    guint64 total_frames;          /* Total frames received */
    guint64 dropped_frames;        /* Frames dropped/skipped */
    gdouble current_fps;           /* Current measured fps */
    gdouble average_fps;           /* Average fps over measurement window */
    gdouble fps_min;               /* Minimum fps observed */
    gdouble fps_max;               /* Maximum fps observed */
    gdouble fps_std_dev;           /* Standard deviation of fps */
    guint64 first_frame_timestamp; /* Timestamp of first frame (ns) */
    guint64 last_frame_timestamp;  /* Timestamp of last frame (ns) */
    guint64 session_duration_ns;   /* Total session time (ns) */
} FrameMonitorStats;

/**
 * Frame drop detection results
 */
typedef struct {
    gboolean has_drops;           /* True if drops detected */
    guint64 drop_count;           /* Number of drops */
    gdouble drop_rate;            /* Drop rate as percentage (0-100) */
    guint64 largest_drop_gap_ns;  /* Largest time gap between frames (ns) */
    guint64 drop_start_timestamp; /* When first drop occurred (ns) */
} FrameDropInfo;

/**
 * Frame rate validation result
 */
typedef enum {
    FRAME_RATE_VALID,             /* Within tolerance */
    FRAME_RATE_LOW,               /* Below minimum */
    FRAME_RATE_HIGH,              /* Above maximum */
    FRAME_RATE_UNSTABLE,          /* High variance */
    FRAME_RATE_INSUFFICIENT_DATA, /* Not enough samples */
} FrameRateValidationResult;

/**
 * Opaque frame monitor context
 */
typedef struct FrameMonitor FrameMonitor;

/**
 * Create a new frame monitor.
 *
 * Initializes frame rate tracking with window size and target frame rate.
 *
 * @return New monitor context, or NULL on error
 */
FrameMonitor *frame_monitor_create(void);

/**
 * Record a frame received event.
 *
 * Should be called once per rendered frame to track frame delivery.
 * Automatically detects and logs frame drops based on timestamp gaps.
 *
 * @param monitor Monitor context
 * @param timestamp Current system timestamp in nanoseconds (use GStreamer clock)
 * @return TRUE if frame recorded successfully
 */
gboolean frame_monitor_on_frame(FrameMonitor *monitor, GstClockTime timestamp);

/**
 * Get current frame rate statistics.
 *
 * Returns accumulated statistics from the monitoring window.
 *
 * @param monitor Monitor context
 * @return Statistics structure (must be freed with g_free)
 */
FrameMonitorStats *frame_monitor_get_stats(FrameMonitor *monitor);

/**
 * Detect dropped frames in the monitoring window.
 *
 * Analyzes frame timestamps to identify gaps indicating dropped frames.
 * Uses adaptive threshold based on observed frame duration.
 *
 * @param monitor Monitor context
 * @return Drop information (must be freed with g_free)
 */
FrameDropInfo *frame_monitor_detect_drops(FrameMonitor *monitor);

/**
 * Validate current frame rate against target.
 *
 * Checks if measured frame rate is within acceptable tolerance
 * (target ±tolerance_fps). Validates consistency and stability.
 *
 * @param monitor Monitor context
 * @param target_fps Target frame rate (default: 120)
 * @param tolerance_fps Acceptable tolerance (default: 2)
 * @return Validation result
 */
FrameRateValidationResult frame_monitor_validate_framerate(FrameMonitor *monitor, guint target_fps,
                                                           guint tolerance_fps);

/**
 * Get a human-readable description of validation result.
 *
 * @param result Validation result
 * @return String description (do not free)
 */
const gchar *frame_monitor_validation_string(FrameRateValidationResult result);

/**
 * Log frame rate statistics to application log.
 *
 * Outputs current statistics in structured format to logging system
 * for performance analysis and debugging.
 *
 * @param monitor Monitor context
 * @param include_detailed Include full statistics or summary only
 */
void frame_monitor_log_stats(FrameMonitor *monitor, gboolean include_detailed);

/**
 * Log detected frame drops to application log.
 *
 * Outputs drop information in warning/error level depending on severity.
 *
 * @param monitor Monitor context
 */
void frame_monitor_log_drops(FrameMonitor *monitor);

/**
 * Reset monitoring statistics.
 *
 * Clears all accumulated frames and statistics. Useful for
 * ignoring startup transients or restarting measurement.
 *
 * @param monitor Monitor context
 */
void frame_monitor_reset(FrameMonitor *monitor);

/**
 * Get the size of the current measurement window.
 *
 * @param monitor Monitor context
 * @return Number of frames currently in window
 */
guint64 frame_monitor_get_window_size(FrameMonitor *monitor);

/**
 * Check if monitor has sufficient data for analysis.
 *
 * Returns true if window has minimum frames for meaningful statistics.
 *
 * @param monitor Monitor context
 * @return TRUE if sufficient data available
 */
gboolean frame_monitor_has_sufficient_data(FrameMonitor *monitor);

/**
 * Cleanup and free frame monitor.
 *
 * @param monitor Monitor context
 */
void frame_monitor_cleanup(FrameMonitor *monitor);

/**
 * Generate a performance report for logging.
 *
 * Creates a formatted report suitable for file or console output
 * summarizing frame rate, drops, and validation results.
 *
 * @param monitor Monitor context
 * @return Allocated string report (must be freed with g_free)
 */
gchar *frame_monitor_generate_report(FrameMonitor *monitor);

#endif /* FRAME_MONITOR_H */
