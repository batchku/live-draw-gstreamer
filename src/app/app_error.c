/**
 * @file app_error.c
 * @brief Application error handling implementation
 */

#include "app_error.h"
#include "../utils/logging.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Global error handler state */
static AppErrorCallback g_error_handler = NULL;
static gpointer g_error_handler_data = NULL;
static AppError *g_last_error = NULL;

void app_register_error_handler(AppErrorCallback handler, gpointer user_data)
{
    g_error_handler = handler;
    g_error_handler_data = user_data;
}

void app_log_error(AppErrorCode code, const gchar *format, ...)
{
    va_list args;
    gchar buffer[1024];

    va_start(args, format);
    g_vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    LOG_ERROR("%s", buffer);

    /* Create error structure */
    if (g_last_error) {
        g_free(g_last_error->message);
        g_free(g_last_error);
    }

    g_last_error = g_malloc0(sizeof(AppError));
    g_last_error->code = code;
    g_last_error->message = g_strdup(buffer);

    /* Invoke error handler if registered */
    if (g_error_handler) {
        g_error_handler(g_last_error, g_error_handler_data);
    }
}

void app_log_warning(AppErrorCode code, const gchar *format, ...)
{
    va_list args;
    gchar buffer[1024];

    va_start(args, format);
    g_vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    LOG_WARNING("%s", buffer);

    /* Create error structure */
    if (g_last_error) {
        g_free(g_last_error->message);
        g_free(g_last_error);
    }

    g_last_error = g_malloc0(sizeof(AppError));
    g_last_error->code = code;
    g_last_error->message = g_strdup(buffer);

    /* Invoke error handler if registered */
    if (g_error_handler) {
        g_error_handler(g_last_error, g_error_handler_data);
    }
}

AppError *app_get_last_error(void)
{
    return g_last_error;
}
