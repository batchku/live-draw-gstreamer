/**
 * @file app_context.c
 * @brief Application context implementation
 */

#include "app_context.h"
#include "../utils/logging.h"
#include <stdlib.h>

/* Global context instance */
static AppContext *g_app_context = NULL;

AppContext *app_context_create(void)
{
    AppContext *ctx = g_malloc0(sizeof(AppContext));

    if (!ctx) {
        LOG_ERROR("Failed to allocate application context");
        return NULL;
    }

    /* Initialize default values */
    ctx->target_fps = 120;
    ctx->grid_cols = 11;
    ctx->grid_rows = 5;
    ctx->total_layers = 50;
    ctx->cell_width_px = 320;
    ctx->pipeline = NULL;
    ctx->bus = NULL;
    ctx->main_loop = NULL;
    ctx->camera = NULL;
    ctx->gst_pipeline = NULL;
    ctx->recording_state = NULL;
    ctx->window = NULL;
    ctx->playback_mgr = NULL;

    LOG_INFO("Application context created");

    return ctx;
}

void app_context_cleanup(AppContext *ctx)
{
    if (!ctx) {
        return;
    }

    LOG_INFO("Cleaning up application context");

    /* Components are responsible for their own cleanup */
    /* This function is mainly for consistency and future extensions */

    g_free(ctx);
}

AppContext *app_context_get(void)
{
    return g_app_context;
}

void app_context_set(AppContext *ctx)
{
    g_app_context = ctx;
}
