/**
 * @file cleanup_handlers.h
 * @brief Cleanup and atexit handlers for graceful resource deallocation
 *
 * This module manages cleanup routines that are called during normal
 * application shutdown and via atexit handlers to ensure proper resource
 * deallocation even in abnormal termination scenarios.
 *
 * Responsible for cleaning up:
 * - GStreamer pipeline and elements
 * - macOS window and Cocoa resources
 * - Camera connections and AVFoundation resources
 * - Memory allocations and GPU buffers
 *
 * The atexit handlers are automatically registered and ensure that
 * even if the application crashes or exits abnormally, core resources
 * are properly released.
 */

#ifndef CLEANUP_HANDLERS_H
#define CLEANUP_HANDLERS_H

#include <glib.h>

/**
 * Initialize cleanup handlers
 *
 * Registers atexit handlers that will be called during program termination
 * to ensure proper cleanup of GStreamer pipeline, window, camera, and memory.
 *
 * This should be called early in application startup, before initializing
 * other components.
 *
 * @return TRUE on success, FALSE on failure
 */
gboolean cleanup_handlers_init(void);

/**
 * Register a cleanup callback to be executed during shutdown
 *
 * Custom cleanup callbacks can be registered to be called alongside
 * the standard cleanup operations. Callbacks are executed in LIFO order
 * (last registered, first executed).
 *
 * @param callback Function to call during cleanup (takes no parameters)
 * @return TRUE on success, FALSE on failure (callback already registered or limit reached)
 */
gboolean cleanup_handlers_register_callback(void (*callback)(void));

/**
 * Unregister a cleanup callback
 *
 * @param callback Function to unregister
 * @return TRUE if callback was found and removed, FALSE otherwise
 */
gboolean cleanup_handlers_unregister_callback(void (*callback)(void));

/**
 * Execute all registered cleanup handlers
 *
 * This function executes all registered cleanup callbacks in LIFO order,
 * followed by standard cleanup of core components (pipeline, window, camera, memory).
 *
 * This is called automatically by the atexit handlers but can also be
 * called explicitly during normal shutdown.
 *
 * Note: This function is idempotent and can be called multiple times safely.
 */
void cleanup_handlers_execute(void);

/**
 * Check if cleanup has been executed
 *
 * @return TRUE if cleanup has already been executed, FALSE otherwise
 */
gboolean cleanup_handlers_has_executed(void);

#endif /* CLEANUP_HANDLERS_H */
