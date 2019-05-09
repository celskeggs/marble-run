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
#include "project.h"
#include "control_loop.h"
#include "servo_control.h"
#include "debug.h"
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

// as defined in 3.2.3.4
SemaphoreHandle_t at_target_position_notify = NULL;

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
    .arm_spin = 30.0,
    .arm_grip = 50.0,
    .arm_left = 50.0,
    .arm_right = 25.0,
    // TODO: calibrate
};

// debugging
static bool err_mutex_mishandle = false;

#define CONTROL_LOOP_UPDATE_PERIOD_MS 20

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
    if (xSemaphoreTake(position_mutex, portMAX_DELAY) != pdTRUE) {
        err_mutex_mishandle = true;
        return;
    }

    // per 3.4.3
    inferred_position = driven_position;

    // per 3.4.4
    if (!servo_point_equal(driven_position, target_position)) {
        struct servo_point max_update;

        max_update = servo_multiply_veloctiy(maximum_speed, CONTROL_LOOP_UPDATE_PERIOD_MS / 1000.0f);

        driven_position = servo_update_clamped(driven_position, target_position, max_update);
        set_servos(driven_position);

        if (servo_point_equal(driven_position, target_position)) {
            at_target_position = true;
            // we don't care if this fails, because that just means the last notification is still pending
            (void) xSemaphoreGive(at_target_position_notify);
        }
    }

    if (xSemaphoreGive(position_mutex) != pdTRUE) {
        err_mutex_mishandle = true;
    }
}

static float actual_frequency = 0;

static void run_control_loop(void *unused) {
    (void) unused;

    uart_send("control loop start\r\n");

    const TickType_t frequency = CONTROL_LOOP_UPDATE_PERIOD_MS / portTICK_PERIOD_MS;
    TickType_t last_wake_time = xTaskGetTickCount();
    TickType_t first_wake_time = last_wake_time;

    int n = 0;

    for (;;) {
        Cy_GPIO_Write(Indicator_PORT, Indicator_NUM, 1);
        vTaskDelayUntil(&last_wake_time, frequency);
        Cy_GPIO_Write(Indicator_PORT, Indicator_NUM, 0);
        
        update_control_loop();
        actual_frequency = (xTaskGetTickCount() - first_wake_time) * portTICK_PERIOD_MS / (++n);
    }
}

// as defined in 3.5
void initialize_control_loop(void) {
    position_mutex = xSemaphoreCreateMutex();
    if (position_mutex == NULL) {
        uart_send("control loop NMUTEX\r\n");
        return;
    }

    at_target_position_notify = xSemaphoreCreateBinary();
    if (at_target_position_notify == NULL) {
        uart_send("control loop NSEMAPHORE\r\n");
        return;
    }

    debug_text("control_loop(PM=");
    debug_mutex_state(&position_mutex);
    debug_text(" RUNFQ=");
    debug_float(&actual_frequency, 5);
    debug_text(" ERRMX=");
    debug_boolean(&err_mutex_mishandle);
    debug_text(" MAXV=");
    debug_servo_velocity(&maximum_speed);
    debug_text(" ");
    void *token = debug_with_mutex(position_mutex);
    debug_text(" DPOS=");
    debug_servo_point(&driven_position);
    debug_text(" IPOS=");
    debug_servo_point(&inferred_position);
    debug_text(" TPOS=");
    debug_servo_point(&target_position);
    debug_text(" ATP=");
    debug_boolean(&at_target_position);
    debug_text(" ATPN=");
    debug_semaphore_state(&at_target_position_notify);
    debug_end_mutex(token);
    debug_text(") ");

    set_servos(driven_position);

    if (xTaskCreate(run_control_loop, "control_loop", 400, NULL, 3, NULL) != pdPASS) {
        uart_send("control loop NTASK\r\n");
    }
}

/* [] END OF FILE */
