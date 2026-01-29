/**
 * @file camera_monitor.h
 * @brief Camera connection monitoring and health checking
 *
 * Provides periodic health checks to detect camera disconnection
 * during a video looper session. Monitors camera element state
 * and generates disconnection events for error handling.
 */

#ifndef CAMERA_MONITOR_H
#define CAMERA_MONITOR_H

#include "camera_source.h"
#include <gst/gst.h>

/**
 * Callback type for camera health events
 *
 * @param is_healthy TRUE if camera is healthy, FALSE if problem detected
 * @param error_message Error message if unhealthy
 * @param user_data User context
 */
typedef void (*CameraHealthCallback)(gboolean is_healthy, const gchar *error_message,
                                     gpointer user_data);

/**
 * @struct CameraMonitor
 * @brief Camera health monitoring context
 */
typedef struct CameraMonitor CameraMonitor;

/**
 * Create a new camera monitor
 *
 * Starts periodic health checking of camera connection.
 * Detects disconnections and generates callbacks when issues are found.
 *
 * @param camera_source Camera source to monitor
 * @param camera_element GStreamer camera element to monitor
 * @return Pointer to new monitor, or NULL on failure
 */
CameraMonitor *camera_monitor_create(CameraSource *camera_source, GstElement *camera_element);

/**
 * Register a callback for camera health events
 *
 * @param monitor Camera monitor
 * @param callback Function to call on health status change
 * @param user_data User context data
 */
void camera_monitor_set_callback(CameraMonitor *monitor, CameraHealthCallback callback,
                                 gpointer user_data);

/**
 * Start periodic health monitoring
 *
 * Begins checking camera status at regular intervals (typically 500ms).
 *
 * @param monitor Camera monitor
 * @return TRUE if monitoring started, FALSE on failure
 */
gboolean camera_monitor_start(CameraMonitor *monitor);

/**
 * Stop periodic health monitoring
 *
 * @param monitor Camera monitor
 * @return TRUE if monitoring stopped, FALSE on failure
 */
gboolean camera_monitor_stop(CameraMonitor *monitor);

/**
 * Perform a single health check on the camera
 *
 * Can be called manually to force a health check between periodic checks.
 *
 * @param monitor Camera monitor
 * @return TRUE if camera is healthy, FALSE if disconnected or error
 */
gboolean camera_monitor_check_health(CameraMonitor *monitor);

/**
 * Check if camera is currently healthy
 *
 * @param monitor Camera monitor
 * @return TRUE if last health check succeeded
 */
gboolean camera_monitor_is_healthy(CameraMonitor *monitor);

/**
 * Get the last health check error message
 *
 * @param monitor Camera monitor
 * @return Error message from last failed health check
 */
const gchar *camera_monitor_get_last_error(CameraMonitor *monitor);

/**
 * Cleanup camera monitor
 *
 * Stops monitoring and frees all resources.
 *
 * @param monitor Camera monitor to cleanup
 */
void camera_monitor_cleanup(CameraMonitor *monitor);

#endif /* CAMERA_MONITOR_H */
