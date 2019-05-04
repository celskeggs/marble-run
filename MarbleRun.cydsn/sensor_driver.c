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
#include "sensor_driver.h"
#include <assert.h>
#include <FreeRTOS.h>
#include <task.h>

// as defined in 4.3.1
int marble_column = 0;
// TODO: condition variable

// as defined in 4.3.2
TickType_t marble_detected_at = 0;

// as defined in 4.3.1.2
SemaphoreHandle_t marble_column_mutex = NULL;

#define SENSOR_LOOP_UPDATE_PERIOD_MS 5

static int poll_first_sensor(void) {
    if (Cy_GPIO_Read(Detect_1_PORT, Detect_1_NUM)) {
        return 1;
    } else if (Cy_GPIO_Read(Detect_2_PORT, Detect_2_NUM)) {
        return 2;
    } else if (Cy_GPIO_Read(Detect_3_PORT, Detect_3_NUM)) {
        return 3;
    } else if (Cy_GPIO_Read(Detect_4_PORT, Detect_4_NUM)) {
        return 4;
    } else if (Cy_GPIO_Read(Detect_5_PORT, Detect_5_NUM)) {
        return 5;
    } else {
        return 0;
    }
}

#if ( INCLUDE_vTaskSuspend != 1)
#error INCLUDE_vTaskSuspend must be true in order to support blocking mutexes
#endif

// as defined in 4.4
static void poll_sensor_state(void) {
    int sensor_active = poll_first_sensor();
    TickType_t detected = xTaskGetTickCount();

    BaseType_t ret;
    ret = xSemaphoreTake(marble_column_mutex, portMAX_DELAY);
    assert(ret == pdTRUE);

    if (marble_column != sensor_active) {
        marble_column = sensor_active;
        marble_detected_at = detected;
    }

    ret = xSemaphoreGive(marble_column_mutex);
    assert(ret == pdTRUE);
}

static void run_sensor_loop(void *unused) {
    (void) unused;

    const TickType_t frequency = SENSOR_LOOP_UPDATE_PERIOD_MS / portTICK_PERIOD_MS;
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&last_wake_time, frequency);
        poll_sensor_state();
    }
}

// as defined in 4.5
void initialize_sensor_driver(void) {
    BaseType_t err;

    marble_column_mutex = xSemaphoreCreateMutex();
    assert(marble_column_mutex != NULL);

    err = xTaskCreate(run_sensor_loop, "sensor_poller", 400, NULL, 4, NULL);
    // TODO: consider other error handling techniques besides 'assert'
    assert(err == pdPASS);
}

/* [] END OF FILE */
