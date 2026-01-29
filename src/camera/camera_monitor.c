/**
 * @file camera_monitor.c
 * @brief Camera connection monitoring implementation
 *
 * Implements periodic health checking to detect camera disconnection
 * and generates notifications for error handling.
 */

#include "camera_monitor.h"
#include "../app/app_error.h"
#include "../utils/logging.h"
#include <stdlib.h>
#include <string.h>

#define HEALTH_CHECK_INTERVAL_MS 500 /**< Health check frequency in milliseconds */
#define MAX_CONSECUTIVE_FAILURES 3   /**< Failed checks before reporting disconnect */

/**
 * Internal camera monitor context
 */
struct CameraMonitor {
    CameraSource *camera_source;          /**< Reference to camera source */
    GstElement *camera_element;           /**< GStreamer camera element to monitor */
    CameraHealthCallback health_callback; /**< Health event callback */
    gpointer callback_user_data;          /**< User data for callback */
    gboolean is_monitoring;               /**< Monitoring in progress */
    gboolean is_healthy;                  /**< Current health status */
    guint consecutive_failures;           /**< Count of consecutive failed checks */
    gchar *last_error_message;            /**< Last error from health check */
    guint timeout_source_id;              /**< GSource ID for monitoring timer */
};

/**
 * Perform internal health check logic
 *
 * Checks camera element state and connectivity.
 *
 * @param monitor Camera monitor
 * @return TRUE if camera is healthy
 */
static gboolean camera_monitor_perform_health_check(CameraMonitor *monitor)
{
    if (!monitor || !monitor->camera_element) {
        return FALSE;
    }

    LOG_DEBUG("Performing camera health check");

    /* Check GStreamer element state */
    GstState state;
    GstStateChangeReturn ret = gst_element_get_state(monitor->camera_element, &state, NULL,
                                                     1000000000); /* 1 second timeout */

    if (ret == GST_STATE_CHANGE_FAILURE) {
        LOG_WARNING("Camera element state change failed");
        return FALSE;
    }

    /* Element should be in PLAYING or READY state for normal operation */
    if (state != GST_STATE_PLAYING && state != GST_STATE_READY) {
        LOG_WARNING("Camera element in unexpected state: %s", gst_element_state_get_name(state));
        return FALSE;
    }

    /* In a real implementation, we would:
     * 1. Query AVFoundation to verify camera device exists
     * 2. Attempt to capture a frame from the camera
     * 3. Check for recent buffer flow (no stalled pipeline)
     * 4. Monitor for missing-plugin errors
     *
     * For now, we pass basic state checks.
     */

    LOG_DEBUG("Camera health check passed");
    return TRUE;
}

/**
 * Timer callback for periodic health checking
 *
 * Called at HEALTH_CHECK_INTERVAL_MS intervals to monitor camera status.
 *
 * @param user_data Camera monitor pointer
 * @return G_SOURCE_CONTINUE to keep timer running
 */
static gboolean camera_monitor_timeout_callback(gpointer user_data)
{
    CameraMonitor *monitor = (CameraMonitor *) user_data;

    if (!monitor || !monitor->is_monitoring) {
        return G_SOURCE_REMOVE;
    }

    gboolean currently_healthy = camera_monitor_perform_health_check(monitor);

    if (currently_healthy) {
        /* Camera is healthy */
        if (monitor->consecutive_failures > 0) {
            LOG_INFO("Camera recovered after %u failures", monitor->consecutive_failures);
        }
        monitor->consecutive_failures = 0;

        if (!monitor->is_healthy) {
            /* Transition from unhealthy to healthy */
            LOG_INFO("Camera health transitioned from unhealthy to healthy");
            monitor->is_healthy = TRUE;

            if (monitor->health_callback) {
                monitor->health_callback(TRUE, NULL, monitor->callback_user_data);
            }
        }
    } else {
        /* Camera check failed */
        monitor->consecutive_failures++;
        LOG_WARNING("Camera health check failed (failure count: %u/%u)",
                    monitor->consecutive_failures, MAX_CONSECUTIVE_FAILURES);

        if (monitor->consecutive_failures >= MAX_CONSECUTIVE_FAILURES) {
            /* Report disconnection after threshold of failures */
            if (monitor->is_healthy) {
                LOG_ERROR("Camera disconnection detected");
                monitor->is_healthy = FALSE;

                /* Update error message */
                if (monitor->last_error_message) {
                    g_free(monitor->last_error_message);
                }
                monitor->last_error_message = g_strdup("Camera failed health check");

                /* Dispatch callback */
                if (monitor->health_callback) {
                    monitor->health_callback(FALSE, monitor->last_error_message,
                                             monitor->callback_user_data);
                }

                /* Log to application error system */
                app_log_warning(APP_ERROR_CAMERA_DISCONNECTED,
                                "Camera disconnected: failed %u consecutive health checks",
                                monitor->consecutive_failures);
            }
        }
    }

    return G_SOURCE_CONTINUE; /* Keep timer running */
}

CameraMonitor *camera_monitor_create(CameraSource *camera_source, GstElement *camera_element)
{
    if (!camera_source || !camera_element) {
        LOG_ERROR("Cannot create monitor with NULL camera source or element");
        return NULL;
    }

    CameraMonitor *monitor = malloc(sizeof(CameraMonitor));
    if (!monitor) {
        LOG_ERROR("Failed to allocate camera monitor");
        return NULL;
    }

    monitor->camera_source = camera_source;
    monitor->camera_element = camera_element;
    monitor->health_callback = NULL;
    monitor->callback_user_data = NULL;
    monitor->is_monitoring = FALSE;
    monitor->is_healthy = TRUE;
    monitor->consecutive_failures = 0;
    monitor->last_error_message = NULL;
    monitor->timeout_source_id = 0;

    LOG_INFO("Camera monitor created");
    return monitor;
}

void camera_monitor_set_callback(CameraMonitor *monitor, CameraHealthCallback callback,
                                 gpointer user_data)
{
    if (!monitor) {
        LOG_ERROR("Cannot set callback on NULL monitor");
        return;
    }

    monitor->health_callback = callback;
    monitor->callback_user_data = user_data;

    LOG_DEBUG("Camera monitor callback registered");
}

gboolean camera_monitor_start(CameraMonitor *monitor)
{
    if (!monitor) {
        LOG_ERROR("Cannot start NULL monitor");
        return FALSE;
    }

    if (monitor->is_monitoring) {
        LOG_WARNING("Camera monitor already running");
        return TRUE;
    }

    LOG_INFO("Starting camera health monitoring (interval: %dms)", HEALTH_CHECK_INTERVAL_MS);

    /* Schedule periodic health checks */
    monitor->timeout_source_id =
        g_timeout_add(HEALTH_CHECK_INTERVAL_MS, camera_monitor_timeout_callback, monitor);

    if (monitor->timeout_source_id == 0) {
        LOG_ERROR("Failed to schedule camera monitor timeout");
        return FALSE;
    }

    monitor->is_monitoring = TRUE;
    monitor->is_healthy = TRUE;
    monitor->consecutive_failures = 0;

    LOG_INFO("Camera health monitoring started (source_id: %u)", monitor->timeout_source_id);
    return TRUE;
}

gboolean camera_monitor_stop(CameraMonitor *monitor)
{
    if (!monitor) {
        LOG_ERROR("Cannot stop NULL monitor");
        return FALSE;
    }

    if (!monitor->is_monitoring) {
        LOG_WARNING("Camera monitor is not running");
        return TRUE;
    }

    LOG_INFO("Stopping camera health monitoring");

    /* Remove timer source */
    if (monitor->timeout_source_id > 0) {
        g_source_remove(monitor->timeout_source_id);
        monitor->timeout_source_id = 0;
    }

    monitor->is_monitoring = FALSE;

    LOG_INFO("Camera health monitoring stopped");
    return TRUE;
}

gboolean camera_monitor_check_health(CameraMonitor *monitor)
{
    if (!monitor) {
        LOG_ERROR("Cannot check health of NULL monitor");
        return FALSE;
    }

    return camera_monitor_perform_health_check(monitor);
}

gboolean camera_monitor_is_healthy(CameraMonitor *monitor)
{
    if (!monitor) {
        return FALSE;
    }

    return monitor->is_healthy;
}

const gchar *camera_monitor_get_last_error(CameraMonitor *monitor)
{
    if (!monitor || !monitor->last_error_message) {
        return "No error";
    }

    return monitor->last_error_message;
}

void camera_monitor_cleanup(CameraMonitor *monitor)
{
    if (!monitor) {
        return;
    }

    LOG_DEBUG("Cleaning up camera monitor");

    /* Stop monitoring */
    camera_monitor_stop(monitor);

    /* Free error message */
    if (monitor->last_error_message) {
        g_free(monitor->last_error_message);
        monitor->last_error_message = NULL;
    }

    /* Clear references */
    monitor->health_callback = NULL;
    monitor->callback_user_data = NULL;
    monitor->camera_source = NULL;
    monitor->camera_element = NULL;

    free(monitor);
}
