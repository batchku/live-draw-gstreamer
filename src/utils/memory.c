/**
 * @file memory.c
 * @brief Memory tracking implementation
 */

#include "memory.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Memory tracking state */
typedef struct {
    size_t total_allocated;
    size_t total_freed;
    size_t allocation_count;
} MemStats;

static MemStats g_mem_stats = {0, 0, 0};

void *mem_calloc(size_t count, size_t size)
{
    void *ptr = calloc(count, size);
    if (ptr && count > 0 && size > 0) {
        g_mem_stats.total_allocated += (count * size);
        g_mem_stats.allocation_count++;
    }
    return ptr;
}

void *mem_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr && size > 0) {
        g_mem_stats.total_allocated += size;
        g_mem_stats.allocation_count++;
    }
    return ptr;
}

void *mem_realloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (new_ptr && size > 0) {
        g_mem_stats.total_allocated += size;
        g_mem_stats.allocation_count++;
    }
    return new_ptr;
}

void mem_free(void *ptr)
{
    if (ptr) {
        free(ptr);
        if (g_mem_stats.allocation_count > 0) {
            g_mem_stats.allocation_count--;
        }
    }
}

size_t mem_get_total_allocated(void)
{
    return g_mem_stats.total_allocated;
}

void mem_get_stats(size_t *total_allocated, size_t *total_freed, size_t *allocation_count)
{
    if (total_allocated) {
        *total_allocated = g_mem_stats.total_allocated;
    }
    if (total_freed) {
        *total_freed = g_mem_stats.total_freed;
    }
    if (allocation_count) {
        *allocation_count = g_mem_stats.allocation_count;
    }
}

void mem_print_stats(void)
{
    fprintf(stderr,
            "Memory Statistics:\n"
            "  Total allocated: %zu bytes\n"
            "  Total freed: %zu bytes\n"
            "  Active allocations: %zu\n"
            "  Net allocation: %zu bytes\n",
            g_mem_stats.total_allocated, g_mem_stats.total_freed, g_mem_stats.allocation_count,
            g_mem_stats.total_allocated - g_mem_stats.total_freed);
}

size_t mem_detect_leaks(void)
{
    if (g_mem_stats.allocation_count > 0) {
        LOG_WARNING("%zu potential memory leaks detected", g_mem_stats.allocation_count);
        mem_print_stats();
    }
    return g_mem_stats.allocation_count;
}

void mem_init(void)
{
    memset(&g_mem_stats, 0, sizeof(g_mem_stats));
    LOG_DEBUG("Memory tracking initialized");
}

void mem_cleanup(void)
{
    if (mem_detect_leaks() == 0) {
        LOG_INFO("No memory leaks detected");
    }
}
