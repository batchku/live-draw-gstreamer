#ifndef TIMING_H
#define TIMING_H

/**
 * @file timing.h
 * @brief High-resolution timing utilities
 *
 * Provides functions for measuring time intervals and frame rates
 * with microsecond precision.
 */

#include <stdint.h>
#include <time.h>

/**
 * Get current time in microseconds since arbitrary point
 *
 * @return Current time in microseconds
 *
 * Uses high-resolution clock (CLOCK_MONOTONIC) for best accuracy.
 * The absolute value is not meaningful; use for calculating deltas.
 */
uint64_t timing_get_time_us(void);

/**
 * Calculate elapsed time between two timestamps
 *
 * @param start_time Earlier timestamp (from timing_get_time_us)
 * @param end_time Later timestamp (from timing_get_time_us)
 * @return Elapsed time in microseconds
 */
uint64_t timing_elapsed_us(uint64_t start_time, uint64_t end_time);

/**
 * Convert microseconds to milliseconds
 *
 * @param microseconds Time in microseconds
 * @return Time in milliseconds (rounded)
 */
double timing_us_to_ms(uint64_t microseconds);

/**
 * Convert microseconds to seconds
 *
 * @param microseconds Time in microseconds
 * @return Time in seconds (rounded)
 */
double timing_us_to_sec(uint64_t microseconds);

/**
 * Convert milliseconds to microseconds
 *
 * @param milliseconds Time in milliseconds
 * @return Time in microseconds
 */
uint64_t timing_ms_to_us(double milliseconds);

/**
 * Convert seconds to microseconds
 *
 * @param seconds Time in seconds
 * @return Time in microseconds
 */
uint64_t timing_sec_to_us(double seconds);

/**
 * Get current wallclock time as formatted string
 *
 * @param buffer Output buffer for string
 * @param size Size of buffer
 * @return Pointer to buffer (for convenience)
 *
 * Format: "YYYY-MM-DD HH:MM:SS"
 */
char *timing_get_timestamp_string(char *buffer, size_t size);

/**
 * Measure frame rate over a period
 *
 * Call once per frame, provides running average fps.
 *
 * @param frame_timestamp Timestamp of current frame (from timing_get_time_us)
 * @return Estimated frames per second, or 0 if measurement in progress
 */
double timing_measure_fps(uint64_t frame_timestamp);

/**
 * Reset frame rate measurement
 */
void timing_reset_fps_measurement(void);

#endif /* TIMING_H */
