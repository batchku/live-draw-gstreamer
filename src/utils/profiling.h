/**
 * @file profiling.h
 * @brief GStreamer pipeline profiling and performance measurement utilities
 *
 * Provides tools for measuring pipeline latency, queue buffering, synchronization
 * delays, and frame delivery performance. Integrates with GStreamer tracepoints
 * and GStreamer's built-in performance metrics.
 */

#ifndef PROFILING_H
#define PROFILING_H

#include <glib.h>
#include <gst/gst.h>
#include <stdint.h>

G_BEGIN_DECLS

/**
 * Performance metrics for a single element or pipeline
 */
typedef struct {
    const char *element_name;        /**< GStreamer element name */
    guint64 total_buffers_processed; /**< Total buffers seen by element */
    guint64 total_bytes_processed;   /**< Total bytes processed */
    guint64 total_time_us;           /**< Total time spent in element (microseconds) */
    guint64 min_latency_us;          /**< Minimum latency observed */
    guint64 max_latency_us;          /**< Maximum latency observed */
    guint64 avg_latency_us;          /**< Average latency */
    gdouble buffer_drop_ratio;       /**< Ratio of dropped buffers (0.0-1.0) */
    guint queue_max_level;           /**< Maximum queue level observed */
    gboolean is_bottleneck;          /**< TRUE if element is identified as bottleneck */
} ProfileMetrics;

/**
 * Performance sample snapshot (captures metrics at a point in time)
 */
typedef struct {
    guint64 timestamp_us;      /**< When sample was taken */
    guint frame_number;        /**< Frame number at sample time */
    gdouble current_fps;       /**< Current frame rate */
    gdouble cpu_usage_percent; /**< Estimated CPU usage */
    guint64 queue_depth_bytes; /**< Total bytes in all queues */
    gint num_dropped_frames;   /**< Frames dropped since last sample */
} PerformanceSample;

/**
 * Frame timing information for latency analysis
 */
typedef struct {
    guint frame_number;
    guint64 timestamp_ns;              /**< GStreamer clock time in nanoseconds */
    guint64 arrival_time_us;           /**< System time when frame arrived */
    guint64 source_to_sink_latency_us; /**< End-to-end latency */
    gboolean was_dropped;              /**< TRUE if frame was dropped */
} FrameTiming;

/**
 * Profiling context for a GStreamer pipeline
 */
typedef struct {
    GstElement *pipeline;
    GQueue *frame_timings;       /**< Queue of FrameTiming structs */
    GQueue *performance_samples; /**< Queue of PerformanceSample structs */
    GHashTable *element_metrics; /**< Hash: element_name -> ProfileMetrics */
    guint64 profile_start_time_us;
    guint sample_interval_ms; /**< Sampling interval (default 100ms) */
    gboolean is_active;
    guint max_samples; /**< Max samples to keep (circular buffer) */
} ProfilingContext;

/**
 * Create and initialize a profiling context for a pipeline
 *
 * @param pipeline GStreamer pipeline to profile
 * @param sample_interval_ms Interval between samples (milliseconds)
 * @param max_samples Maximum samples to retain (0 = unlimited)
 * @return Profiling context, or NULL on error
 */
ProfilingContext *profiling_context_create(GstElement *pipeline, guint sample_interval_ms,
                                           guint max_samples);

/**
 * Start continuous profiling of a pipeline
 *
 * Installs probes on key elements and begins sampling at specified interval.
 *
 * @param ctx Profiling context
 * @return TRUE on success, FALSE on error
 */
gboolean profiling_start(ProfilingContext *ctx);

/**
 * Stop profiling and finalize metrics
 *
 * @param ctx Profiling context
 */
void profiling_stop(ProfilingContext *ctx);

/**
 * Collect current performance sample
 *
 * Called periodically to capture frame rate, queue depths, CPU usage.
 *
 * @param ctx Profiling context
 * @return TRUE if sample collected successfully
 */
gboolean profiling_collect_sample(ProfilingContext *ctx);

/**
 * Record timing information for a frame
 *
 * Should be called when frame passes through key points in pipeline.
 *
 * @param ctx Profiling context
 * @param frame_num Frame number
 * @param timestamp_ns GStreamer timestamp (nanoseconds)
 * @param arrival_time_us System time when frame arrived (microseconds)
 * @param dropped TRUE if frame was dropped
 */
void profiling_record_frame(ProfilingContext *ctx, guint frame_num, guint64 timestamp_ns,
                            guint64 arrival_time_us, gboolean dropped);

/**
 * Identify bottleneck elements in pipeline
 *
 * Analyzes queue depths, latency measurements, and buffer drop rates
 * to identify which elements are performance bottlenecks.
 *
 * @param ctx Profiling context
 * @return Array of ProfileMetrics for bottleneck elements (must free with g_free)
 */
ProfileMetrics *profiling_identify_bottlenecks(ProfilingContext *ctx);

/**
 * Get profiling metrics for a specific element
 *
 * @param ctx Profiling context
 * @param element_name Name of GStreamer element
 * @return Metrics, or NULL if not found
 */
ProfileMetrics *profiling_get_element_metrics(ProfilingContext *ctx, const char *element_name);

/**
 * Calculate frame rate statistics from collected samples
 *
 * @param ctx Profiling context
 * @param current_fps Output: current FPS
 * @param average_fps Output: average FPS
 * @param min_fps Output: minimum FPS observed
 * @param max_fps Output: maximum FPS observed
 * @param std_dev_fps Output: standard deviation
 * @return TRUE if statistics calculated successfully
 */
gboolean profiling_get_fps_stats(ProfilingContext *ctx, gdouble *current_fps, gdouble *average_fps,
                                 gdouble *min_fps, gdouble *max_fps, gdouble *std_dev_fps);

/**
 * Get queue buffering statistics
 *
 * @param ctx Profiling context
 * @param total_queue_bytes Output: total bytes across all queues
 * @param max_queue_depth Output: maximum queue depth observed
 * @param num_queues Output: number of queues being monitored
 * @return TRUE on success
 */
gboolean profiling_get_queue_stats(ProfilingContext *ctx, guint64 *total_queue_bytes,
                                   guint *max_queue_depth, guint *num_queues);

/**
 * Get synchronization metrics
 *
 * @param ctx Profiling context
 * @param buffer_drop_ratio Output: ratio of dropped buffers (0.0-1.0)
 * @param jitter_us Output: timing jitter in microseconds
 * @param max_latency_us Output: maximum observed latency
 * @return TRUE on success
 */
gboolean profiling_get_sync_metrics(ProfilingContext *ctx, gdouble *buffer_drop_ratio,
                                    guint64 *jitter_us, guint64 *max_latency_us);

/**
 * Generate profiling report as formatted string
 *
 * Includes FPS statistics, queue analysis, bottleneck identification,
 * and recommendations for optimization.
 *
 * @param ctx Profiling context
 * @return Formatted report string (must free with g_free)
 */
gchar *profiling_generate_report(ProfilingContext *ctx);

/**
 * Export profiling data to JSON file
 *
 * Exports all metrics, samples, and frame timings for further analysis.
 *
 * @param ctx Profiling context
 * @param filename Path to output JSON file
 * @return TRUE on success, FALSE on error
 */
gboolean profiling_export_json(ProfilingContext *ctx, const char *filename);

/**
 * Clean up and free profiling context
 *
 * @param ctx Profiling context
 */
void profiling_context_free(ProfilingContext *ctx);

G_END_DECLS

#endif /* PROFILING_H */
