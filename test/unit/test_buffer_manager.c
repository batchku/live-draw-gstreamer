/**
 * @file test_buffer_manager.c
 * @brief Unit tests for GPU ring buffer management
 *
 * Tests the buffer manager implementation for ring buffer operations:
 * - Buffer allocation and initialization
 * - Frame writing with wraparound
 * - Frame reading from arbitrary indices
 * - Buffer exhaustion handling (oldest frame discarding)
 * - Memory management and cleanup
 * - Edge cases (empty buffer, single frame, NULL safety)
 *
 * Test Coverage:
 * - 12 unit tests covering all buffer manager functionality
 * - Ring buffer allocation and lifecycle
 * - Frame write/read operations
 * - Circular wraparound behavior
 * - Duration tracking
 * - Error handling (NULL parameters, out-of-bounds, etc.)
 */

#include "../../src/recording/buffer_manager.h"
#include <glib.h>
#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test counter */
static int tests_passed = 0;
static int tests_failed = 0;

/**
 * @brief Helper macro for assertions
 */
#define ASSERT_EQ(actual, expected, test_name)                                                     \
    if ((actual) != (expected)) {                                                                  \
        fprintf(stdout, "FAIL: %s (expected %lu, got %lu)\n", test_name,                           \
                (unsigned long) (expected), (unsigned long) (actual));                             \
        tests_failed++;                                                                            \
    } else {                                                                                       \
        fprintf(stdout, "PASS: %s\n", test_name);                                                  \
        tests_passed++;                                                                            \
    }

#define ASSERT_NE(actual, unexpected, test_name)                                                   \
    if ((actual) == (unexpected)) {                                                                \
        fprintf(stdout, "FAIL: %s (should not be %lu)\n", test_name,                               \
                (unsigned long) (unexpected));                                                     \
        tests_failed++;                                                                            \
    } else {                                                                                       \
        fprintf(stdout, "PASS: %s\n", test_name);                                                  \
        tests_passed++;                                                                            \
    }

#define ASSERT_NULL(ptr, test_name)                                                                \
    if ((ptr) != NULL) {                                                                           \
        fprintf(stdout, "FAIL: %s (expected NULL)\n", test_name);                                  \
        tests_failed++;                                                                            \
    } else {                                                                                       \
        fprintf(stdout, "PASS: %s\n", test_name);                                                  \
        tests_passed++;                                                                            \
    }

#define ASSERT_NOT_NULL(ptr, test_name)                                                            \
    if ((ptr) == NULL) {                                                                           \
        fprintf(stdout, "FAIL: %s (expected non-NULL)\n", test_name);                              \
        tests_failed++;                                                                            \
    } else {                                                                                       \
        fprintf(stdout, "PASS: %s\n", test_name);                                                  \
        tests_passed++;                                                                            \
    }

/**
 * Test 1: Buffer creation with valid capacity
 */
static void test_buffer_create_valid_capacity(void)
{
    RingBuffer *buf = buffer_create(10, NULL);

    ASSERT_NOT_NULL(buf, "Buffer creation successful");
    if (buf) {
        ASSERT_EQ(buf->capacity, 10, "Buffer capacity correct");
        ASSERT_EQ(buf->frame_count, 0, "Buffer initially empty");
        ASSERT_EQ(buf->write_pos, 0, "Write position at 0");
        ASSERT_EQ(buf->duration_us, 0, "Duration initially 0");
        buffer_cleanup(buf);
    }
}

/**
 * Test 2: Buffer creation with zero capacity returns NULL
 */
static void test_buffer_create_zero_capacity(void)
{
    RingBuffer *buf = buffer_create(0, NULL);
    ASSERT_NULL(buf, "Buffer with zero capacity returns NULL");
}

/**
 * Test 3: Single frame write to empty buffer
 */
static void test_buffer_write_single_frame(void)
{
    RingBuffer *buf = buffer_create(5, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created");

    if (buf) {
        GstBuffer *frame = gst_buffer_new();
        ASSERT_NOT_NULL(frame, "GStreamer buffer created");

        if (frame) {
            buffer_write_frame(buf, frame);

            ASSERT_EQ(buf->frame_count, 1, "Frame count incremented to 1");
            ASSERT_EQ(buf->write_pos, 1, "Write position advanced to 1");
            ASSERT_NOT_NULL(buf->frames[0], "Frame stored at position 0");

            gst_buffer_unref(frame);
        }
        buffer_cleanup(buf);
    }
}

/**
 * Test 4: Multiple frame writes without wraparound
 */
static void test_buffer_write_multiple_frames(void)
{
    RingBuffer *buf = buffer_create(5, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created with capacity 5");

    if (buf) {
        /* Write 3 frames */
        for (int i = 0; i < 3; i++) {
            GstBuffer *frame = gst_buffer_new();
            if (frame) {
                buffer_write_frame(buf, frame);
                gst_buffer_unref(frame);
            }
        }

        ASSERT_EQ(buf->frame_count, 3, "Frame count correct after 3 writes");
        ASSERT_EQ(buf->write_pos, 3, "Write position at 3");

        buffer_cleanup(buf);
    }
}

/**
 * Test 5: Buffer wraparound when reaching capacity
 */
static void test_buffer_wraparound_at_capacity(void)
{
    RingBuffer *buf = buffer_create(3, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created with capacity 3");

    if (buf) {
        /* Fill buffer to capacity: frames 0, 1, 2 */
        for (int i = 0; i < 3; i++) {
            GstBuffer *frame = gst_buffer_new();
            if (frame) {
                buffer_write_frame(buf, frame);
                gst_buffer_unref(frame);
            }
        }

        ASSERT_EQ(buf->frame_count, 3, "Frame count equals capacity");
        ASSERT_EQ(buf->write_pos, 0, "Write position wrapped to 0");

        /* Write fourth frame (should overwrite frame 0) */
        GstBuffer *frame4 = gst_buffer_new();
        if (frame4) {
            buffer_write_frame(buf, frame4);

            ASSERT_EQ(buf->frame_count, 3, "Frame count remains at capacity");
            ASSERT_EQ(buf->write_pos, 1, "Write position advanced to 1 after wraparound");

            gst_buffer_unref(frame4);
        }

        buffer_cleanup(buf);
    }
}

/**
 * Test 6: Frame read from buffer before wraparound
 */
static void test_buffer_read_frame_no_wraparound(void)
{
    RingBuffer *buf = buffer_create(5, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created");

    if (buf) {
        /* Write 2 frames */
        for (int i = 0; i < 2; i++) {
            GstBuffer *frame = gst_buffer_new();
            if (frame) {
                buffer_write_frame(buf, frame);
                gst_buffer_unref(frame);
            }
        }

        /* Read both frames */
        GstBuffer *read0 = buffer_read_frame(buf, 0);
        GstBuffer *read1 = buffer_read_frame(buf, 1);

        ASSERT_NOT_NULL(read0, "Frame 0 read successfully");
        ASSERT_NOT_NULL(read1, "Frame 1 read successfully");

        /* Try reading out-of-bounds frame */
        GstBuffer *readOOB = buffer_read_frame(buf, 2);
        ASSERT_NULL(readOOB, "Out-of-bounds frame read returns NULL");

        buffer_cleanup(buf);
    }
}

/**
 * Test 7: Frame read with wraparound
 */
static void test_buffer_read_frame_with_wraparound(void)
{
    RingBuffer *buf = buffer_create(3, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created with capacity 3");

    if (buf) {
        /* Fill buffer with frames 0, 1, 2 */
        for (int i = 0; i < 3; i++) {
            GstBuffer *frame = gst_buffer_new();
            if (frame) {
                buffer_write_frame(buf, frame);
                gst_buffer_unref(frame);
            }
        }

        /* Write frame 3 (overwrites old frame 0) */
        GstBuffer *frame3 = gst_buffer_new();
        if (frame3) {
            buffer_write_frame(buf, frame3);
            gst_buffer_unref(frame3);
        }

        /* Now buffer contains frames 1, 2, 3 (old frame 0 was discarded) */
        GstBuffer *read0 = buffer_read_frame(buf, 0); /* Should be old frame 1 */
        GstBuffer *read1 = buffer_read_frame(buf, 1); /* Should be old frame 2 */
        GstBuffer *read2 = buffer_read_frame(buf, 2); /* Should be frame 3 */

        ASSERT_NOT_NULL(read0, "Wrapped frame 0 read successfully");
        ASSERT_NOT_NULL(read1, "Wrapped frame 1 read successfully");
        ASSERT_NOT_NULL(read2, "Wrapped frame 2 read successfully");

        buffer_cleanup(buf);
    }
}

/**
 * Test 8: Get frame count
 */
static void test_buffer_get_frame_count(void)
{
    RingBuffer *buf = buffer_create(5, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created");

    if (buf) {
        ASSERT_EQ(buffer_get_frame_count(buf), 0, "Initial frame count is 0");

        /* Write one frame */
        GstBuffer *frame = gst_buffer_new();
        if (frame) {
            buffer_write_frame(buf, frame);
            ASSERT_EQ(buffer_get_frame_count(buf), 1, "Frame count after write is 1");
            gst_buffer_unref(frame);
        }

        buffer_cleanup(buf);
    }
}

/**
 * Test 9: Duration tracking with frame writes
 */
static void test_buffer_duration_tracking(void)
{
    RingBuffer *buf = buffer_create(5, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created");

    if (buf) {
        ASSERT_EQ(buffer_get_duration(buf), 0, "Initial duration is 0");

        /* Write frame without explicit duration (assumes 33ms per frame) */
        GstBuffer *frame = gst_buffer_new();
        if (frame) {
            buffer_write_frame(buf, frame);

            /* Duration should be approximately 33000 microseconds (33ms) */
            guint64 duration = buffer_get_duration(buf);
            if (duration >= 33000 && duration <= 34000) {
                fprintf(stdout, "PASS: Duration tracking correct (33000us)\n");
                tests_passed++;
            } else {
                fprintf(stdout, "FAIL: Duration tracking (expected ~33000us, got %llu)\n",
                        (unsigned long long) duration);
                tests_failed++;
            }

            gst_buffer_unref(frame);
        }

        buffer_cleanup(buf);
    }
}

/**
 * Test 10: NULL buffer safety
 */
static void test_buffer_null_safety(void)
{
    /* These should not crash */
    GstBuffer *frame = gst_buffer_new();

    buffer_write_frame(NULL, frame);
    buffer_write_frame(NULL, NULL);

    GstBuffer *readResult = buffer_read_frame(NULL, 0);
    ASSERT_NULL(readResult, "buffer_read_frame(NULL) returns NULL");

    guint count = buffer_get_frame_count(NULL);
    ASSERT_EQ(count, 0, "buffer_get_frame_count(NULL) returns 0");

    guint64 duration = buffer_get_duration(NULL);
    ASSERT_EQ(duration, 0, "buffer_get_duration(NULL) returns 0");

    buffer_cleanup(NULL); /* Should not crash */
    fprintf(stdout, "PASS: NULL safety\n");
    tests_passed++;

    gst_buffer_unref(frame);
}

/**
 * Test 11: Empty buffer operations
 */
static void test_buffer_empty_operations(void)
{
    RingBuffer *buf = buffer_create(5, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created");

    if (buf) {
        /* Empty buffer should return 0 frames */
        ASSERT_EQ(buffer_get_frame_count(buf), 0, "Empty buffer has 0 frames");
        ASSERT_EQ(buffer_get_duration(buf), 0, "Empty buffer has 0 duration");

        /* Reading from empty buffer returns NULL */
        GstBuffer *read = buffer_read_frame(buf, 0);
        ASSERT_NULL(read, "Reading from empty buffer returns NULL");

        buffer_cleanup(buf);
    }
}

/**
 * Test 12: Multiple wraparounds
 */
static void test_buffer_multiple_wraparounds(void)
{
    RingBuffer *buf = buffer_create(2, NULL);
    ASSERT_NOT_NULL(buf, "Buffer created with capacity 2");

    if (buf) {
        /* Write 5 frames (will trigger 2 wraparounds) */
        for (int i = 0; i < 5; i++) {
            GstBuffer *frame = gst_buffer_new();
            if (frame) {
                buffer_write_frame(buf, frame);
                gst_buffer_unref(frame);
            }
        }

        /* Frame count should still be capped at capacity (2) */
        ASSERT_EQ(buf->frame_count, 2, "Frame count capped at capacity");

        /* Write position should wrap correctly: 5 % 2 = 1 */
        ASSERT_EQ(buf->write_pos, 1, "Write position correct after multiple wraps");

        buffer_cleanup(buf);
    }
}

/**
 * Main test runner
 */
int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    /* Initialize GStreamer for buffer operations */
    gst_init(NULL, NULL);

    fprintf(stdout, "\n=== Buffer Manager Unit Tests ===\n\n");

    test_buffer_create_valid_capacity();
    test_buffer_create_zero_capacity();
    test_buffer_write_single_frame();
    test_buffer_write_multiple_frames();
    test_buffer_wraparound_at_capacity();
    test_buffer_read_frame_no_wraparound();
    test_buffer_read_frame_with_wraparound();
    test_buffer_get_frame_count();
    test_buffer_duration_tracking();
    test_buffer_null_safety();
    test_buffer_empty_operations();
    test_buffer_multiple_wraparounds();

    fprintf(stdout, "\n=== Test Results ===\n");
    fprintf(stdout, "Passed: %d\n", tests_passed);
    fprintf(stdout, "Failed: %d\n", tests_failed);

    gst_deinit();

    if (tests_failed == 0) {
        fprintf(stdout, "\n✓ All buffer manager tests passed!\n\n");
        return 0;
    } else {
        fprintf(stdout, "\n✗ Some tests failed\n\n");
        return 1;
    }
}
