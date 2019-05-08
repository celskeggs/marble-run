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
static float drop_interval = 0.25; // TODO: configure

// as defined in 5.4.2
static float pickup_end = 2.0; // TODO: configure

//for pickup to return both the column and the time
typedef struct {
    int col;
    TickType_t col_at;
} sensor_out;

const TickType_t drop_delay;
const TickType_t pickup_delay;

// as defined in 5.10
static void path_plan(int start_pos, int end_pos, int pickup_index) {
    //selecting the direction to go
    int sign = -1;
    if(start_pos < end_pos){
        sign = 1;
    }
    int current = start_pos;
    //looping until the target position is the same as the goal
    while(current != end_pos){
        BaseType_t ret;
        current += sign;
        ret = xSemaphoreTake(position_mutex, portMAX_DELAY);
        assert(ret == pdTRUE);
        if(current != 3){
            target_position = *graph[current];
        } else {
            target_position = graph[3][pickup_index];
        }
        if (!servo_point_equal(target_position, inferred_position)) {
            at_target_position = false;
        }
        while (!at_target_position) {
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
    path_plan(0, 2, 0);
}

// as defined in 5.6
static sensor_out wait_for_marble(void) {
    BaseType_t ret;
    ret = xSemaphoreTake(marble_column_mutex, portMAX_DELAY);
    assert(ret == pdTRUE);
    while (marble_column == 0) {
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
    path_plan(2, 3, col);
    vTaskDelayUntil(&col_at, pickup_delay);
}

// as defined in 5.8
static void to_drop(void) {
    path_plan(3, 0, 0);
}

// as defined in 5.9
static void drop_marble(void) {
    target_position.arm_grip = 45; //TODO: set actual values
    vTaskDelay(drop_delay);
    target_position.arm_grip = 0; //TODO: set actual values
}

// as defined in 5.3
static void run_path_loop(void * unused) {
    (void) unused;
    uart_send("sequencer start\r\n");
    for(;;){
        pickup_standby();
        sensor_out so = wait_for_marble();
        pickup_marble(so.col, so.col_at);
        to_drop();
        drop_marble();
    }
}

// as defined in 5.11
void initialize_sequencer(void) {
    const TickType_t drop_delay = drop_interval * 1000 / portTICK_PERIOD_MS;
    const TickType_t pickup_delay = pickup_end * 1000 / portTICK_PERIOD_MS;

    if (xTaskCreate(run_path_loop, "sequencer", 400, NULL, 2, NULL) != pdPASS) {
        uart_send("sequencer NTASK\r\n");
    }
    debug_text("sequencer(no debugging info)");
}

/* [] END OF FILE */
