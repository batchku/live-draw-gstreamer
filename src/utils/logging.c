/**
 * @file logging.c
 * @brief Logging implementation
 */

#include "logging.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

/* Global logging state */
static LogLevel g_log_level = LOG_LEVEL_INFO;

void logging_set_level(LogLevel level)
{
    g_log_level = level;
}

LogLevel logging_get_level(void)
{
    return g_log_level;
}

void logging_log(LogLevel level, const gchar *category, const gchar *format, ...)
{
    if (level < g_log_level) {
        return; /* Skip messages below current level */
    }

    va_list args;
    const gchar *level_str;
    time_t now;
    struct tm *timeinfo;
    gchar time_buffer[32];

    /* Get current time */
    time(&now);
    timeinfo = localtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    /* Convert level to string */
    switch (level) {
    case LOG_LEVEL_DEBUG:
        level_str = "DEBUG";
        break;
    case LOG_LEVEL_INFO:
        level_str = "INFO";
        break;
    case LOG_LEVEL_WARNING:
        level_str = "WARNING";
        break;
    case LOG_LEVEL_ERROR:
        level_str = "ERROR";
        break;
    default:
        level_str = "UNKNOWN";
    }

    /* Print to stderr */
    fprintf(stderr, "[%s] [%s] %s: ", time_buffer, level_str, category);

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
    fflush(stderr);
}

void logging_init(void)
{
    /* Default to INFO to keep console output clean */
    g_log_level = LOG_LEVEL_INFO;

    LOG_INFO("Logging initialized (level: %d)", g_log_level);
}

void logging_cleanup(void)
{
    LOG_INFO("Logging cleanup");
    /* Nothing to cleanup currently */
}
