/**
 * @file live_tee.c
 * @brief Live stream tee element configuration for deadlock-free stream splitting.
 *
 * Implements specialized configuration and pad management for the tee element
 * that splits the camera feed to the live queue (cell 1 display) and recording bins.
 *
 * Key deadlock prevention strategy:
 * 1. allow-not-linked = true: Allows pads to be unlinked without blocking
 * 2. has-chain = true: Enables synchronization on input side
 * 3. Leaky queue on live branch: Drops old frames if videomixer gets slow
 * 4. FakeSink in record bins: Consumes frames quickly without buffering
 */

#include "live_tee.h"
#include "../utils/logging.h"
#include <glib.h>
#include <gst/gst.h>
#include <stdio.h>

gboolean live_tee_configure(GstElement *tee_element)
{
    if (!tee_element) {
        LOG_ERROR("tee_element is NULL");
        return FALSE;
    }

    // Check if tee element has the required properties
    GParamSpec *allow_not_linked_spec =
        g_object_class_find_property(G_OBJECT_GET_CLASS(tee_element), "allow-not-linked");

    GParamSpec *has_chain_spec =
        g_object_class_find_property(G_OBJECT_GET_CLASS(tee_element), "has-chain");

    // Configure allow-not-linked if available
    if (allow_not_linked_spec) {
        g_object_set(G_OBJECT(tee_element), "allow-not-linked", TRUE, NULL);
        LOG_DEBUG("Configured tee element: allow-not-linked = TRUE");
    } else {
        LOG_WARNING("tee element does not have 'allow-not-linked' property");
        LOG_WARNING("This may cause deadlock if output pads are unlinked during playback");
    }

    // Configure has-chain if available
    if (has_chain_spec) {
        g_object_set(G_OBJECT(tee_element), "has-chain", TRUE, NULL);
        LOG_DEBUG("Configured tee element: has-chain = TRUE");
    } else {
        LOG_DEBUG("tee element does not have 'has-chain' property (not available in this GStreamer "
                  "version)");
    }

    // Query remaining properties for logging
    GParamSpec *pull_mode_spec =
        g_object_class_find_property(G_OBJECT_GET_CLASS(tee_element), "pull-mode");
    if (pull_mode_spec) {
        guint pull_mode;
        g_object_get(G_OBJECT(tee_element), "pull-mode", &pull_mode, NULL);
        LOG_DEBUG("tee element pull-mode = %u (0=never, 1=single, 2=all)", pull_mode);
    }

    LOG_INFO("Live tee element configured for deadlock-free stream splitting");
    return TRUE;
}

GstPad *live_tee_request_pad(GstElement *tee_element, int record_bin_id)
{
    if (!tee_element) {
        LOG_ERROR("tee_element is NULL");
        return NULL;
    }

    // Request a new source pad from the tee element
    // GStreamer will automatically assign an incrementing name (src_0, src_1, etc)
    GstPad *tee_pad = gst_element_request_pad_simple(tee_element, "src_%u");
    if (!tee_pad) {
        LOG_ERROR("Failed to request source pad from tee element for record bin %d", record_bin_id);
        return NULL;
    }

    LOG_DEBUG("Requested tee output pad for record bin %d (pad name: %s)", record_bin_id,
              GST_PAD_NAME(tee_pad));

    return tee_pad;
}

gboolean live_tee_release_pad(GstElement *tee_element, GstPad *tee_pad)
{
    if (!tee_element || !tee_pad) {
        LOG_ERROR("tee_element or tee_pad is NULL");
        return FALSE;
    }

    // Ensure the pad is unlinked before releasing
    GstPad *peer_pad = gst_pad_get_peer(tee_pad);
    if (peer_pad) {
        // Unlink the tee pad from its peer (the record bin's input)
        GstPadLinkReturn link_ret = gst_pad_unlink(tee_pad, peer_pad);
        if (link_ret != GST_PAD_LINK_OK) {
            LOG_WARNING("Failed to unlink tee pad from peer (link return: %d)", link_ret);
            // Continue anyway; the release may still work
        }
        gst_object_unref(peer_pad);
    }

    // Release the pad from the tee element
    // This will also remove the pad from the element
    gst_element_release_request_pad(tee_element, tee_pad);
    LOG_DEBUG("Released tee output pad (name: %s)", GST_PAD_NAME(tee_pad));

    return TRUE;
}
