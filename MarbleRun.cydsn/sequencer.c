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
#include "path_planning.h"
#include "control_loop.h"
#include "sensor_driver.h"
#include <assert.h>
#include <FreeRTOS.h>
#include <task.h>
#include "debug.h"

// as defined in 5.2.1
//static struct graph_node; // TODO: initialize

// as defined in 5.4.1
static float drop_interval = 0.5; // TODO: configure

// as defined in 5.4.2
static float pickup_end = 2.0; // TODO: configure

// debugging
static bool stop_sequence = false;

//for pickup to return both the column and the time
typedef struct {
    int col;
    TickType_t col_at;
} sensor_out;

// debugging
static int vary_position = 0;

// as defined in 5.10
static void path_plan(int start_pos, int end_pos, int pickup_index) {
    //selecting the direction to go
    int sign = -1;
    if(start_pos < end_pos){
        sign = 1;
    }
    int current = start_pos;
    //looping until the target position is the same as the goal
    while(current != end_pos && !stop_sequence){
        BaseType_t ret;
        current += sign;
        ret = xSemaphoreTake(position_mutex, portMAX_DELAY);
        assert(ret == pdTRUE);
        if(current != 5){
            target_position = *graph[current];
            if (current == 0) {
                target_position.arm_spin += vary_position;
                vary_position += 5;
                if (vary_position > 20) {
                    vary_position = -20;
                }
            }
        } else {
            target_position = graph[5][pickup_index-1];
        }
        if (!servo_point_equal(target_position, inferred_position)) {
            at_target_position = false;
        }
        while (!at_target_position && !stop_sequence) {
            ret = xSemaphoreGive(position_mutex);
            assert(ret == pdTRUE);

            ret = xSemaphoreTake(at_target_position_notify, portMAX_DELAY);
            assert(ret == pdTRUE);

            ret = xSemaphoreTake(position_mutex, portMAX_DELAY);
            assert(ret == pdTRUE);
        }
        ret = xSemaphoreGive(position_mutex);
        assert(ret == pdTRUE);
    }
}

// as defined in 5.5
static void pickup_standby(void) {
    path_plan(0, 4, 0);
}

// as defined in 5.6
static sensor_out wait_for_marble(void) {
    BaseType_t ret;
    ret = xSemaphoreTake(marble_column_mutex, portMAX_DELAY);
    assert(ret == pdTRUE);
    while (marble_column == 0 && !stop_sequence) {
        ret = xSemaphoreGive(marble_column_mutex);
        assert(ret == pdTRUE);

        ret = xSemaphoreTake(marble_column_notify, portMAX_DELAY);
        assert(ret == pdTRUE);

        ret = xSemaphoreTake(marble_column_mutex, portMAX_DELAY);
        assert(ret == pdTRUE);
    }
    sensor_out so = {marble_column, marble_detected_at};
    ret = xSemaphoreGive(marble_column_mutex);
    assert(ret == pdTRUE);
    return so;
}

// as defined in 5.7
static void pickup_marble(int col, TickType_t col_at) {
    path_plan(4, 5, col);
    vTaskDelayUntil(&col_at, pickup_end * 1000 / portTICK_PERIOD_MS);
    if (stop_sequence) {
        return;
    }

    BaseType_t ret;
    ret = xSemaphoreTake(position_mutex, portMAX_DELAY);
    assert(ret == pdTRUE);
    slow_mode = true;
    ret = xSemaphoreGive(position_mutex);
    assert(ret == pdTRUE);
}

// as defined in 5.8
static void to_drop(void) {
    path_plan(5, 0, 0);
}

// as defined in 5.9
static void drop_marble(void) {
    BaseType_t ret;

    ret = xSemaphoreTake(marble_column_mutex, portMAX_DELAY);
    assert(ret == pdTRUE);
    marble_column = 0;
    ret = xSemaphoreGive(marble_column_mutex);
    assert(ret == pdTRUE);

    ret = xSemaphoreTake(position_mutex, portMAX_DELAY);
    assert(ret == pdTRUE);
    slow_mode = false;
    target_position.arm_grip = -20; //TODO: set actual values
    ret = xSemaphoreGive(position_mutex);
    assert(ret == pdTRUE);
    vTaskDelay(drop_interval * 1000 / portTICK_PERIOD_MS);
    // don't bother explicitly setting the gripper back, because that'll be implicitly done by the next move
}

// as defined in 5.3
static void run_path_loop(void * unused) {
    (void) unused;
    uart_send("sequencer start\r\n");
    for(;;){
        while (stop_sequence) {
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
        pickup_standby();
        if (stop_sequence) {
            continue;
        }
        sensor_out so = wait_for_marble();
        if (stop_sequence) {
            continue;
        }
        pickup_marble(so.col, so.col_at);
        if (stop_sequence) {
            continue;
        }
        to_drop();
        if (stop_sequence) {
            continue;
        }
        drop_marble();
    }
}

// as defined in 5.11
void initialize_sequencer(void) {
    if (xTaskCreate(run_path_loop, "sequencer", 400, NULL, 2, NULL) != pdPASS) {
        uart_send("sequencer NTASK\r\n");
    }
    debug_text("sequencer(drop=");
    debug_float("drop", &drop_interval, 5);
    debug_text(" pickup=");
    debug_float("pickup", &pickup_end, 5);
    debug_text(" stop=");
    debug_boolean("stop", &stop_sequence);
    debug_text(")");
}

/* [] END OF FILE */
