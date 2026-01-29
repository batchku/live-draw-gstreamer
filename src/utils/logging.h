#ifndef LOGGING_H
#define LOGGING_H

/**
 * @file logging.h
 * @brief Centralized logging utilities
 *
 * Provides logging functions with configurable levels and output
 * to stderr. Supports DEBUG, INFO, WARNING, and ERROR levels.
 */

#include <glib.h>

/**
 * @enum LogLevel
 * @brief Logging severity levels
 */
typedef enum {
    LOG_LEVEL_DEBUG = 0,   /**< Detailed debug information */
    LOG_LEVEL_INFO = 1,    /**< General informational messages */
    LOG_LEVEL_WARNING = 2, /**< Warning messages for potential issues */
    LOG_LEVEL_ERROR = 3,   /**< Error messages for critical issues */
} LogLevel;

/**
 * Set the global log level threshold
 *
 * Messages below this level will not be displayed.
 *
 * @param level Minimum log level to display
 */
void logging_set_level(LogLevel level);

/**
 * Get the current global log level
 *
 * @return Current log level threshold
 */
LogLevel logging_get_level(void);

/**
 * Log a message with the specified level
 *
 * @param level Log level for this message
 * @param category Category/source of the message (usually __func__)
 * @param format Format string (printf-style)
 * @param ... Arguments for format string
 */
void logging_log(LogLevel level, const gchar *category, const gchar *format, ...);

/**
 * Convenience macro for debug logging
 *
 * Example: LOG_DEBUG("Value: %d", x);
 */
#define LOG_DEBUG(fmt, ...) logging_log(LOG_LEVEL_DEBUG, __func__, fmt, ##__VA_ARGS__)

/**
 * Convenience macro for info logging
 *
 * Example: LOG_INFO("Starting initialization");
 */
#define LOG_INFO(fmt, ...) logging_log(LOG_LEVEL_INFO, __func__, fmt, ##__VA_ARGS__)

/**
 * Convenience macro for warning logging
 *
 * Example: LOG_WARNING("Unexpected frame drop");
 */
#define LOG_WARNING(fmt, ...) logging_log(LOG_LEVEL_WARNING, __func__, fmt, ##__VA_ARGS__)

/**
 * Convenience macro for error logging
 *
 * Example: LOG_ERROR("Failed to allocate buffer");
 */
#define LOG_ERROR(fmt, ...) logging_log(LOG_LEVEL_ERROR, __func__, fmt, ##__VA_ARGS__)

/**
 * Initialize the logging system
 *
 * Should be called once at application startup.
 */
void logging_init(void);

/**
 * Cleanup the logging system
 *
 * Should be called once at application shutdown.
 */
void logging_cleanup(void);

#endif /* LOGGING_H */
