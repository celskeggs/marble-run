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
#include "debug.h"
#include "sensor_driver.h"
#include <assert.h>
#include <FreeRTOS.h>
#include <task.h>

// as defined in 4.3.1
unsigned int marble_column = 0;
// TODO: condition variable

// as defined in 4.3.2
TickType_t marble_detected_at = 0;

// as defined in 4.3.1.2
SemaphoreHandle_t marble_column_mutex = NULL;

// debugging
static bool err_mutex_mishandle = false;

#define SENSOR_LOOP_UPDATE_PERIOD_MS 5

static unsigned int poll_first_sensor(void) {
    if (Cy_GPIO_Read(Detect_0_PORT, Detect_0_NUM)) {
        return 1;
    } else if (Cy_GPIO_Read(Detect_1_PORT, Detect_1_NUM)) {
        return 2;
    } else if (Cy_GPIO_Read(Detect_2_PORT, Detect_2_NUM)) {
        return 3;
    } else if (Cy_GPIO_Read(Detect_3_PORT, Detect_3_NUM)) {
        return 4;
    } else if (Cy_GPIO_Read(Detect_4_PORT, Detect_4_NUM)) {
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
    unsigned int sensor_active = poll_first_sensor();
    TickType_t detected = xTaskGetTickCount();

    if (xSemaphoreTake(marble_column_mutex, portMAX_DELAY) != pdTRUE) {
        err_mutex_mishandle = true;
        return;
    }

    if (marble_column != sensor_active) {
        marble_column = sensor_active;
        marble_detected_at = detected;
    }

    if (xSemaphoreGive(marble_column_mutex) != pdTRUE) {
        err_mutex_mishandle = true;
    }
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
    marble_column_mutex = xSemaphoreCreateMutex();
    if (marble_column_mutex != NULL) {
        uart_send("sensor driver NMUTEX\r\n");
        return;
    }

    debug_text("sensor_driver(MCM=");
    debug_mutex_state(&marble_column_mutex);
    debug_text(" ERRMX=");
    debug_boolean(&err_mutex_mishandle);
    debug_text(" ");
    void *token = debug_with_mutex(marble_column_mutex);
    debug_text(" MC=");
    debug_integer(&marble_column, 1);
    debug_text(" MDA=");
    debug_ticktype(&marble_detected_at);
    debug_end_mutex(token);
    debug_text(") ");

    if (xTaskCreate(run_sensor_loop, "sensor_poller", 400, NULL, 4, NULL) != pdPASS) {
        uart_send("sensor driver NTASK\r\n");
    }
}

/* [] END OF FILE */
