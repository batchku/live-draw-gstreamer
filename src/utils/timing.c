/**
 * @file timing.c
 * @brief Timing implementation
 */

#include "timing.h"
#include "logging.h"
#include <string.h>
#include <time.h>

/* FPS measurement state */
typedef struct {
    uint64_t first_frame_time;
    uint64_t last_frame_time;
    int frame_count;
} FPSMeasurement;

static FPSMeasurement g_fps_measurement = {0, 0, 0};

uint64_t timing_get_time_us(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        LOG_ERROR("Failed to get monotonic time");
        return 0;
    }

    uint64_t seconds_us = (uint64_t) ts.tv_sec * 1000000ULL;
    uint64_t nanoseconds_us = (uint64_t) ts.tv_nsec / 1000UL;
    return seconds_us + nanoseconds_us;
}

uint64_t timing_elapsed_us(uint64_t start_time, uint64_t end_time)
{
    if (end_time >= start_time) {
        return end_time - start_time;
    }
    return 0;
}

double timing_us_to_ms(uint64_t microseconds)
{
    return (double) microseconds / 1000.0;
}

double timing_us_to_sec(uint64_t microseconds)
{
    return (double) microseconds / 1000000.0;
}

uint64_t timing_ms_to_us(double milliseconds)
{
    return (uint64_t) (milliseconds * 1000.0);
}

uint64_t timing_sec_to_us(double seconds)
{
    return (uint64_t) (seconds * 1000000.0);
}

char *timing_get_timestamp_string(char *buffer, size_t size)
{
    if (!buffer || size < 20) {
        return NULL;
    }

    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);

    return buffer;
}

double timing_measure_fps(uint64_t frame_timestamp)
{
    if (g_fps_measurement.first_frame_time == 0) {
        /* First frame - initialize measurement */
        g_fps_measurement.first_frame_time = frame_timestamp;
        g_fps_measurement.last_frame_time = frame_timestamp;
        g_fps_measurement.frame_count = 1;
        return 0.0;
    }

    g_fps_measurement.frame_count++;
    uint64_t elapsed = timing_elapsed_us(g_fps_measurement.first_frame_time, frame_timestamp);

    /* Measure FPS only after at least 1 second has passed */
    if (elapsed >= 1000000ULL) {
        double seconds = timing_us_to_sec(elapsed);
        double fps = (double) g_fps_measurement.frame_count / seconds;
        return fps;
    }

    return 0.0;
}

void timing_reset_fps_measurement(void)
{
    memset(&g_fps_measurement, 0, sizeof(g_fps_measurement));
}
