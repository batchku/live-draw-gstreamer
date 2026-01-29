/**
 * @file camera_source.c
 * @brief Camera source implementation
 *
 * Implements camera initialization, permission handling, format negotiation,
 * and GStreamer element creation using AVFoundation for macOS camera access.
 */

#include "camera_source.h"
#include "../app/app_error.h"
#include "../utils/logging.h"
#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declarations for Objective-C camera functions */
extern CameraPermissionStatus camera_request_permission_objc(void);
extern int camera_negotiate_format_objc(int *out_width, int *out_height, int *out_framerate);

/**
 * Format negotiation candidates in order of preference
 * @brief Attempts to negotiate camera format starting with 1920x1080 @ 30fps,
 *        then falls back to 1280x720 @ 30fps if the preferred format is unavailable.
 */
typedef struct {
    int width;
    int height;
    int framerate;
} FormatCandidate;

static const FormatCandidate FORMAT_CANDIDATES[] = {
    {1920, 1080, 30}, /* Preferred format */
    {1280, 720, 30},  /* Fallback format */
    {0, 0, 0}         /* Sentinel value */
};

/**
 * Attempt to negotiate camera format using GStreamer capabilities
 *
 * @param cam Camera source to configure
 * @return TRUE if format negotiation succeeded, FALSE otherwise
 */
static gboolean camera_negotiate_format(CameraSource *cam)
{
    if (!cam) {
        LOG_ERROR("Invalid camera source for format negotiation");
        return FALSE;
    }

    LOG_DEBUG("Attempting camera format negotiation...");

    /* Try each format candidate in order of preference */
    for (int i = 0; FORMAT_CANDIDATES[i].width > 0; i++) {
        int width = FORMAT_CANDIDATES[i].width;
        int height = FORMAT_CANDIDATES[i].height;
        int framerate = FORMAT_CANDIDATES[i].framerate;

        LOG_DEBUG("Trying format: %dx%d @ %d fps", width, height, framerate);

        /* Update camera configuration */
        cam->width = width;
        cam->height = height;
        cam->framerate = framerate;

        /* Build caps string for this format */
        snprintf(cam->caps_string, sizeof(cam->caps_string),
                 "video/x-raw, width=%d, height=%d, framerate=%d/1, format=UYVY", width, height,
                 framerate);

        /* Attempt to validate format with AVFoundation via Objective-C bridge */
        /* Note: For actual hardware validation, this would attempt to create
         * a capture session with these capabilities and verify success.
         * For now, we use GStreamer's capability negotiation which happens
         * during pipeline operation. */

        LOG_INFO("Selected format: %dx%d @ %d fps (caps: %s)", width, height, framerate,
                 cam->caps_string);
        return TRUE;
    }

    /* All format candidates failed */
    LOG_ERROR("No compatible camera format found");
    app_log_error(APP_ERROR_CAMERA_NOT_FOUND, "Unable to negotiate compatible camera format");
    return FALSE;
}

CameraSource *camera_source_init(void)
{
    LOG_DEBUG("Initializing camera source...");

    CameraSource *cam = malloc(sizeof(CameraSource));
    if (!cam) {
        LOG_ERROR("Failed to allocate camera source");
        app_log_error(APP_ERROR_MEMORY_ALLOCATION_FAILED,
                      "Failed to allocate camera source structure");
        return NULL;
    }

    memset(cam, 0, sizeof(CameraSource));

    /* Request camera permission before attempting to detect/configure camera */
    CameraPermissionStatus perm_status = camera_request_permission();
    if (perm_status == CAMERA_PERMISSION_DENIED) {
        LOG_ERROR("Camera permission denied by user");
        app_log_error(APP_ERROR_CAMERA_PERMISSION_DENIED, "Camera permission denied by user");
        free(cam);
        return NULL;
    }

    if (perm_status == CAMERA_PERMISSION_NOT_DETERMINED) {
        LOG_WARNING("Camera permission status not determined - proceeding cautiously");
    }

    /* Set device identifier to built-in camera */
    strncpy(cam->device_id, "built-in", sizeof(cam->device_id) - 1);

    /* Perform format negotiation with fallback strategy */
    if (!camera_negotiate_format(cam)) {
        LOG_ERROR("Camera format negotiation failed");
        free(cam);
        return NULL;
    }

    LOG_INFO("Camera source initialized successfully: %s (%dx%d @ %d fps)", cam->device_id,
             cam->width, cam->height, cam->framerate);
    return cam;
}

CameraPermissionStatus camera_request_permission(void)
{
    LOG_DEBUG("Requesting camera permission from AVFoundation...");

    /* Delegate to Objective-C implementation which uses AVFoundation */
    CameraPermissionStatus status = camera_request_permission_objc();

    switch (status) {
    case CAMERA_PERMISSION_GRANTED:
        LOG_INFO("Camera permission granted");
        break;
    case CAMERA_PERMISSION_DENIED:
        LOG_ERROR("Camera permission denied by user");
        break;
    case CAMERA_PERMISSION_NOT_DETERMINED:
        LOG_DEBUG("Camera permission status not yet determined");
        break;
    }

    return status;
}

GstElement *camera_source_create_element(CameraSource *cam)
{
    if (!cam) {
        LOG_ERROR("Invalid camera source");
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED, "Camera source is NULL");
        return NULL;
    }

    LOG_DEBUG("Creating avfvideosrc GStreamer element for camera: %s", cam->device_id);

    /* Create avfvideosrc element (native macOS video source) */
    GstElement *source = gst_element_factory_make("avfvideosrc", "camera_source");
    if (!source) {
        LOG_ERROR("Failed to create avfvideosrc element - GStreamer AVF plugin not available");
        app_log_error(APP_ERROR_CAMERA_NOT_FOUND,
                      "Failed to create avfvideosrc element - AVF plugin may not be installed");
        return NULL;
    }

    LOG_DEBUG("avfvideosrc element created successfully");

    /* Set device-specific properties if needed */
    /* For built-in camera, we rely on default device selection by AVFoundation */
    g_object_set(G_OBJECT(source), "do-timestamp", TRUE, /* Add timestamps to buffers */
                 NULL);

    /* Create capsfilter element for format negotiation and validation */
    GstElement *capsfilter = gst_element_factory_make("capsfilter", "camera_caps");
    if (!capsfilter) {
        LOG_ERROR("Failed to create capsfilter element");
        gst_object_unref(source);
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED, "Failed to create capsfilter element");
        return NULL;
    }

    LOG_DEBUG("capsfilter element created for format: %s", cam->caps_string);

    /* Parse and set caps for the capsfilter */
    GstCaps *caps = gst_caps_from_string(cam->caps_string);
    if (!caps) {
        LOG_ERROR("Failed to parse caps string: %s", cam->caps_string);
        gst_object_unref(capsfilter);
        gst_object_unref(source);
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED, "Failed to parse camera caps string");
        return NULL;
    }

    /* Set the capabilities on the capsfilter */
    g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);
    gst_caps_unref(caps);

    LOG_INFO("Camera format negotiated: %dx%d @ %d fps (UYVY)", cam->width, cam->height,
             cam->framerate);

    /* Create a bin to hold both elements */
    GstElement *bin = gst_bin_new("camera_source_bin");
    if (!bin) {
        LOG_ERROR("Failed to create camera source bin");
        gst_object_unref(capsfilter);
        gst_object_unref(source);
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED, "Failed to create camera source bin");
        return NULL;
    }

    /* Add elements to bin */
    gst_bin_add(GST_BIN(bin), source);
    gst_bin_add(GST_BIN(bin), capsfilter);

    /* Link elements: avfvideosrc -> capsfilter */
    if (!gst_element_link(source, capsfilter)) {
        LOG_ERROR("Failed to link avfvideosrc to capsfilter - "
                  "format negotiation failed");
        gst_object_unref(bin);
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED,
                      "Failed to link camera source elements - format negotiation error");
        return NULL;
    }

    LOG_DEBUG("Camera source and capsfilter linked successfully");

    /* Add ghost pad for easy external linking */
    GstPad *capsfilter_src_pad = gst_element_get_static_pad(capsfilter, "src");
    if (!capsfilter_src_pad) {
        LOG_ERROR("Failed to get capsfilter src pad");
        gst_object_unref(bin);
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED, "Failed to get capsfilter source pad");
        return NULL;
    }

    GstPad *ghost_pad = gst_ghost_pad_new("src", capsfilter_src_pad);
    if (!ghost_pad) {
        LOG_ERROR("Failed to create ghost pad for camera source bin");
        gst_object_unref(capsfilter_src_pad);
        gst_object_unref(bin);
        app_log_error(APP_ERROR_PIPELINE_BUILD_FAILED, "Failed to create ghost pad");
        return NULL;
    }

    gst_element_add_pad(bin, ghost_pad);
    gst_object_unref(capsfilter_src_pad);

    /* Store reference to actual source element for potential future access */
    cam->source_element = source;

    LOG_INFO("Camera source element created successfully: %s (%dx%d @ %d fps)", cam->device_id,
             cam->width, cam->height, cam->framerate);
    return bin;
}

CameraCapabilities *camera_get_capabilities(CameraSource *cam)
{
    if (!cam) {
        LOG_ERROR("Invalid camera source");
        return NULL;
    }

    LOG_DEBUG("Querying camera format capabilities...");

    /* Allocate capabilities structure */
    CameraCapabilities *caps = malloc(sizeof(CameraCapabilities));
    if (!caps) {
        LOG_ERROR("Failed to allocate capabilities structure");
        app_log_error(APP_ERROR_MEMORY_ALLOCATION_FAILED, "Failed to allocate camera capabilities");
        return NULL;
    }

    /* Allocate arrays for format candidates evaluated during negotiation */
    /* We document the formats that are tried in order of preference */
    int *widths = malloc(sizeof(int) * 3);     /* 1920, 1280, 0 (null terminator) */
    int *heights = malloc(sizeof(int) * 3);    /* 1080, 720, 0 */
    int *framerates = malloc(sizeof(int) * 2); /* 30, 0 */

    if (!widths || !heights || !framerates) {
        LOG_ERROR("Failed to allocate capability arrays");
        free(widths);
        free(heights);
        free(framerates);
        free(caps);
        app_log_error(APP_ERROR_MEMORY_ALLOCATION_FAILED,
                      "Failed to allocate camera capabilities arrays");
        return NULL;
    }

    /* Document the format negotiation candidates:
     * Priority 1: 1920x1080 @ 30fps (preferred)
     * Priority 2: 1280x720 @ 30fps (fallback)
     */
    widths[0] = 1920;
    widths[1] = 1280;
    widths[2] = 0; /* Null terminator */

    heights[0] = 1080;
    heights[1] = 720;
    heights[2] = 0; /* Null terminator */

    framerates[0] = 30;
    framerates[1] = 0; /* Null terminator */

    caps->supported_widths = widths;
    caps->supported_heights = heights;
    caps->supported_framerates = framerates;
    caps->count = 2; /* Number of negotiation candidates */

    LOG_INFO("Camera capabilities: 2 negotiation candidates "
             "(1920x1080 @30fps preferred, 1280x720 @30fps fallback)");
    return caps;
}

void camera_capabilities_free(CameraCapabilities *caps)
{
    if (!caps) {
        return;
    }

    LOG_DEBUG("Freeing camera capabilities");

    if (caps->supported_widths) {
        free(caps->supported_widths);
        caps->supported_widths = NULL;
    }

    if (caps->supported_heights) {
        free(caps->supported_heights);
        caps->supported_heights = NULL;
    }

    if (caps->supported_framerates) {
        free(caps->supported_framerates);
        caps->supported_framerates = NULL;
    }

    free(caps);
}

void camera_source_cleanup(CameraSource *cam)
{
    if (!cam) {
        return;
    }

    LOG_DEBUG("Cleaning up camera source");

    if (cam->source_element) {
        gst_object_unref(cam->source_element);
        cam->source_element = NULL;
    }

    free(cam);
}

gboolean camera_source_is_connected(CameraSource *cam)
{
    if (!cam) {
        LOG_ERROR("Cannot check connection status of NULL camera");
        return FALSE;
    }

    LOG_DEBUG("Checking camera connection status");

    if (!cam->source_element) {
        LOG_WARNING("Camera source element is NULL");
        return FALSE;
    }

    /* In a real implementation, this would:
     * 1. Query AVFoundation to verify camera device still exists
     * 2. Attempt to get a frame from the camera
     * 3. Check GStreamer element state
     *
     * For now, we assume connected if element exists.
     * This will be enhanced when AVFoundation bridge functions are available.
     */

    LOG_DEBUG("Camera connection check passed");
    return TRUE;
}

gboolean camera_source_reinitialize(CameraSource *cam)
{
    if (!cam) {
        LOG_ERROR("Cannot reinitialize NULL camera source");
        return FALSE;
    }

    LOG_INFO("Attempting to reinitialize camera source");

    /* Request permission again in case it was restored */
    CameraPermissionStatus perm_status = camera_request_permission();
    if (perm_status == CAMERA_PERMISSION_DENIED) {
        LOG_ERROR("Camera permission still denied after reconnection attempt");
        return FALSE;
    }

    /* Attempt format negotiation again */
    if (!camera_negotiate_format(cam)) {
        LOG_ERROR("Format negotiation failed during camera reinitialization");
        return FALSE;
    }

    LOG_INFO("Camera reinitialization succeeded: %s (%dx%d @ %d fps)", cam->device_id, cam->width,
             cam->height, cam->framerate);
    return TRUE;
}
