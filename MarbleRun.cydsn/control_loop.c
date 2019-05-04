/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "control_loop.h"
#include <assert.h>
#include <math.h>
#include <FreeRTOS.h>
#include <task.h>

// as defined in 3.2.1
struct servo_point target_position = {
    0.0,
    0.0,
    0.0,
    0.0,
};

// as defined in 3.2.1.2
SemaphoreHandle_t position_mutex = NULL;

// as defined in 3.2.2
struct servo_point inferred_position = {
    0.0,
    0.0,
    0.0,
    0.0,
};

// as defined in 3.2.3
bool at_target_position = false;

// TODO: condition variable

// as defined in 3.2.4
static struct servo_point driven_position = {
    0.0,
    0.0,
    0.0,
    0.0,
};

// as defined in 3.3.1
static struct servo_velocity maximum_speed = {
    // these are measured in degrees per second
    .arm_spin = 10.0,
    .arm_grip = 10.0,
    .arm_left = 10.0,
    .arm_right = 10.0,
    // TODO: calibrate
};

#define CONTROL_LOOP_UPDATE_PERIOD_MS (20)

static float float_update_clamped(float current, float target, float max_update) {
    assert(max_update > 0);
    if (target > current + max_update) {
        return current + max_update;
    } else if (target < current - max_update) {
        return current - max_update;
    } else {
        return target;
    }
}

static struct servo_point servo_update_clamped(struct servo_point current, struct servo_point target, struct servo_point max_update) {
    return (struct servo_point) {
        .arm_spin = float_update_clamped(current.arm_spin, target.arm_spin, max_update.arm_spin),
        .arm_grip = float_update_clamped(current.arm_grip, target.arm_grip, max_update.arm_grip),
        .arm_left = float_update_clamped(current.arm_left, target.arm_left, max_update.arm_left),
        .arm_right = float_update_clamped(current.arm_right, target.arm_right, max_update.arm_right),
    };
}

#if ( INCLUDE_vTaskSuspend != 1)
#error INCLUDE_vTaskSuspend must be true in order to support blocking mutexes
#endif

// as defined in 3.4
static void update_control_loop(void) {
    BaseType_t ret;
    ret = xSemaphoreTake(position_mutex, portMAX_DELAY);
    assert(ret == pdTRUE);

    // per 3.4.3
    inferred_position = driven_position;

    // per 3.4.4
    if (!servo_point_equal(driven_position, target_position)) {
        struct servo_point max_update;

        max_update = servo_multiply_veloctiy(maximum_speed, CONTROL_LOOP_UPDATE_PERIOD_MS / 1000.0f);

        driven_position = servo_update_clamped(driven_position, target_position, max_update);

        if (servo_point_equal(driven_position, target_position)) {
            at_target_position = true;
            // TODO: broadcast condition variable
        }
    }

    ret = xSemaphoreGive(position_mutex);
    assert(ret == pdTRUE);
}

static void run_control_loop(void *unused) {
    (void) unused;

    const TickType_t frequency = CONTROL_LOOP_UPDATE_PERIOD_MS / portTICK_PERIOD_MS;
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&last_wake_time, frequency);
        update_control_loop();
    }
}

// as defined in 3.5
void initialize_control_loop(void) {
    BaseType_t err;

    position_mutex = xSemaphoreCreateMutex();
    assert(position_mutex != NULL);

    err = xTaskCreate(run_control_loop, "control_loop", 400, NULL, 3, NULL);
    // TODO: consider other error handling techniques besides 'assert'
    assert(err == pdPASS);
}

/* [] END OF FILE */
