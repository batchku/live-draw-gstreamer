/**
 * @file test_camera_error_handler.c
 * @brief Unit tests for camera error handling (T-9.1)
 *
 * Tests comprehensive camera error handling including:
 * - Camera not found error handling
 * - Permission denied error handling
 * - Camera disconnection detection and recovery
 * - Error state transitions
 */

#include "../src/camera/camera_error_handler.h"
#include "../src/camera/camera_monitor.h"
#include "../src/camera/camera_source.h"
#include "../src/utils/logging.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>

/**
 * Test fixture for camera error handler tests
 */
typedef struct {
    CameraErrorHandler *handler;
    CameraSource *camera;
} CameraErrorTestFixture;

/**
 * Setup test fixture
 */
static void setup_camera_error_test(CameraErrorTestFixture *f)
{
    logging_set_level(LOG_LEVEL_DEBUG);

    /* Create a mock camera source */
    f->camera = g_malloc0(sizeof(CameraSource));
    strcpy(f->camera->device_id, "mock-camera");
    f->camera->width = 1920;
    f->camera->height = 1080;
    f->camera->framerate = 30;
    f->camera->source_element = NULL;

    /* Create error handler */
    f->handler = camera_error_handler_create(f->camera);
    g_assert(f->handler != NULL);
}

/**
 * Teardown test fixture
 */
static void teardown_camera_error_test(CameraErrorTestFixture *f)
{
    if (f->handler) {
        camera_error_handler_cleanup(f->handler);
        f->handler = NULL;
    }

    if (f->camera) {
        g_free(f->camera);
        f->camera = NULL;
    }
}

/**
 * Test: Camera error handler creation
 */
void test_camera_error_handler_create(void)
{
    CameraErrorTestFixture f;
    setup_camera_error_test(&f);

    /* Verify handler initialized in correct state */
    g_assert(camera_error_get_state(f.handler) == CAMERA_STATE_UNINITIALIZED);
    g_assert(!camera_error_is_in_error_state(f.handler));
    g_assert(!camera_error_is_accessible(f.handler));

    teardown_camera_error_test(&f);
    printf("✓ test_camera_error_handler_create\n");
}

/**
 * Test: Handle camera not found error (fatal)
 */
void test_camera_error_handle_not_found(void)
{
    CameraErrorTestFixture f;
    setup_camera_error_test(&f);

    gboolean result = camera_error_handle_not_found(f.handler);
    g_assert(result == TRUE);

    /* Verify handler state after error */
    g_assert(camera_error_get_state(f.handler) == CAMERA_STATE_ERROR);
    g_assert(camera_error_is_in_error_state(f.handler));
    g_assert(!camera_error_is_accessible(f.handler));

    /* Verify error info captured */
    CameraErrorInfo *error = camera_error_get_last_error(f.handler);
    g_assert(error != NULL);
    g_assert(error->error_type == CAMERA_ERROR_NOT_FOUND);
    g_assert(error->is_recoverable == FALSE);

    teardown_camera_error_test(&f);
    printf("✓ test_camera_error_handle_not_found\n");
}

/**
 * Test: Handle camera permission denied error (fatal)
 */
void test_camera_error_handle_permission_denied(void)
{
    CameraErrorTestFixture f;
    setup_camera_error_test(&f);

    gboolean result = camera_error_handle_permission_denied(f.handler);
    g_assert(result == TRUE);

    /* Verify handler state */
    g_assert(camera_error_get_state(f.handler) == CAMERA_STATE_ERROR);
    g_assert(camera_error_is_in_error_state(f.handler));

    /* Verify error info */
    CameraErrorInfo *error = camera_error_get_last_error(f.handler);
    g_assert(error != NULL);
    g_assert(error->error_type == CAMERA_ERROR_PERMISSION_DENIED);
    g_assert(error->is_recoverable == FALSE);

    teardown_camera_error_test(&f);
    printf("✓ test_camera_error_handle_permission_denied\n");
}

/**
 * Test: Handle camera disconnection error (recoverable)
 */
void test_camera_error_handle_disconnected(void)
{
    CameraErrorTestFixture f;
    setup_camera_error_test(&f);

    /* First set state to ready */
    camera_error_set_state(f.handler, CAMERA_STATE_READY);
    g_assert(camera_error_is_accessible(f.handler));

    /* Now handle disconnection */
    gboolean result = camera_error_handle_disconnected(f.handler);
    g_assert(result == TRUE);

    /* Verify handler transitions to disconnected state */
    g_assert(camera_error_get_state(f.handler) == CAMERA_STATE_DISCONNECTED ||
             camera_error_get_state(f.handler) == CAMERA_STATE_INITIALIZING);

    /* Verify error info marks as recoverable */
    CameraErrorInfo *error = camera_error_get_last_error(f.handler);
    g_assert(error != NULL);
    g_assert(error->error_type == CAMERA_ERROR_DISCONNECTED);
    g_assert(error->is_recoverable == TRUE);

    teardown_camera_error_test(&f);
    printf("✓ test_camera_error_handle_disconnected\n");
}

/**
 * Test: Reconnection attempts are limited
 */
void test_camera_error_reconnection_limit(void)
{
    CameraErrorTestFixture f;
    setup_camera_error_test(&f);

    /* Set to disconnected state */
    camera_error_set_state(f.handler, CAMERA_STATE_DISCONNECTED);

    /* Attempt multiple reconnections */
    gboolean result = TRUE;
    for (int i = 0; i < 6; i++) {
        result = camera_error_attempt_reconnection(f.handler);
        if (i < 5) {
            g_assert(result == TRUE); /* Should succeed initially */
        } else {
            g_assert(result == FALSE); /* Should fail after 5 attempts */
        }
    }

    /* Verify state is error after max retries */
    g_assert(camera_error_get_state(f.handler) == CAMERA_STATE_ERROR);

    teardown_camera_error_test(&f);
    printf("✓ test_camera_error_reconnection_limit\n");
}

/**
 * Test: State transitions
 */
void test_camera_error_state_transitions(void)
{
    CameraErrorTestFixture f;
    setup_camera_error_test(&f);

    /* Test state transitions */
    camera_error_set_state(f.handler, CAMERA_STATE_INITIALIZING);
    g_assert(camera_error_get_state(f.handler) == CAMERA_STATE_INITIALIZING);

    camera_error_set_state(f.handler, CAMERA_STATE_READY);
    g_assert(camera_error_get_state(f.handler) == CAMERA_STATE_READY);
    g_assert(camera_error_is_accessible(f.handler));

    camera_error_set_state(f.handler, CAMERA_STATE_DISCONNECTED);
    g_assert(!camera_error_is_accessible(f.handler));

    camera_error_set_state(f.handler, CAMERA_STATE_ERROR);
    g_assert(camera_error_is_in_error_state(f.handler));

    teardown_camera_error_test(&f);
    printf("✓ test_camera_error_state_transitions\n");
}

/**
 * Test: Error reset after recovery
 */
void test_camera_error_reset_state(void)
{
    CameraErrorTestFixture f;
    setup_camera_error_test(&f);

    /* Set to error state */
    camera_error_set_state(f.handler, CAMERA_STATE_ERROR);
    camera_error_handle_not_found(f.handler);

    g_assert(camera_error_is_in_error_state(f.handler));
    g_assert(camera_error_get_last_error(f.handler) != NULL);

    /* Reset error state (after successful recovery) */
    camera_error_reset_state(f.handler);

    g_assert(camera_error_get_state(f.handler) == CAMERA_STATE_READY);
    g_assert(!camera_error_is_in_error_state(f.handler));
    g_assert(camera_error_is_accessible(f.handler));
    g_assert(camera_error_get_last_error(f.handler) == NULL);

    teardown_camera_error_test(&f);
    printf("✓ test_camera_error_reset_state\n");
}

/**
 * Test: Error callback invocation
 */
void test_camera_error_callback(void)
{
    CameraErrorTestFixture f;
    setup_camera_error_test(&f);

    /* Track callback invocations */
    gboolean callback_invoked = FALSE;
    void callback_fn(CameraErrorInfo * info, gpointer data)
    {
        gboolean *invoked = (gboolean *) data;
        *invoked = TRUE;
        g_assert(info != NULL);
        g_assert(info->error_type == CAMERA_ERROR_NOT_FOUND);
    }

    camera_error_handler_set_callback(f.handler, callback_fn, &callback_invoked);

    /* Trigger error that should invoke callback */
    camera_error_handle_not_found(f.handler);

    g_assert(callback_invoked == TRUE);

    teardown_camera_error_test(&f);
    printf("✓ test_camera_error_callback\n");
}

/**
 * Run all camera error handler tests
 */
int main(int argc, char **argv)
{
    printf("\nRunning Camera Error Handler Unit Tests (T-9.1)\n");
    printf("==============================================\n\n");

    test_camera_error_handler_create();
    test_camera_error_handle_not_found();
    test_camera_error_handle_permission_denied();
    test_camera_error_handle_disconnected();
    test_camera_error_reconnection_limit();
    test_camera_error_state_transitions();
    test_camera_error_reset_state();
    test_camera_error_callback();

    printf("\n✓ All camera error handler tests passed\n");
    return 0;
}
