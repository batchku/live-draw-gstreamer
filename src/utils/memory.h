#ifndef MEMORY_H
#define MEMORY_H

/**
 * @file memory.h
 * @brief Memory allocation tracking and debugging utilities
 *
 * Provides wrappers around malloc/calloc/free for tracking memory
 * allocations and detecting leaks during development.
 */

#include <stddef.h>

/**
 * Allocate zero-initialized memory
 *
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 *
 * Wrapper around calloc with tracking capabilities.
 */
void *mem_calloc(size_t count, size_t size);

/**
 * Allocate uninitialized memory
 *
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL on failure
 *
 * Wrapper around malloc with tracking capabilities.
 */
void *mem_malloc(size_t size);

/**
 * Reallocate memory
 *
 * @param ptr Pointer to existing allocation
 * @param size New size in bytes
 * @return Pointer to reallocated memory, or NULL on failure
 *
 * Wrapper around realloc with tracking capabilities.
 */
void *mem_realloc(void *ptr, size_t size);

/**
 * Free allocated memory
 *
 * @param ptr Pointer to memory to free
 *
 * Wrapper around free with tracking capabilities.
 */
void mem_free(void *ptr);

/**
 * Get total allocated memory
 *
 * @return Total bytes currently allocated
 */
size_t mem_get_total_allocated(void);

/**
 * Get allocation statistics
 *
 * @param total_allocated Output for total allocated bytes
 * @param total_freed Output for total freed bytes
 * @param allocation_count Output for number of active allocations
 */
void mem_get_stats(size_t *total_allocated, size_t *total_freed, size_t *allocation_count);

/**
 * Print memory statistics to stderr
 */
void mem_print_stats(void);

/**
 * Detect memory leaks (should be called at cleanup time)
 *
 * @return Number of unfreed allocations
 */
size_t mem_detect_leaks(void);

/**
 * Initialize memory tracking system
 */
void mem_init(void);

/**
 * Cleanup memory tracking system
 */
void mem_cleanup(void);

#endif /* MEMORY_H */
