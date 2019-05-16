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

// as defined in 4.3.2
TickType_t marble_detected_at = 0;

// as defined in 4.3.1.2
SemaphoreHandle_t marble_column_mutex = NULL;

// as defined in 4.3.1.3
SemaphoreHandle_t marble_column_notify = NULL;

// debugging
static bool err_mutex_mishandle = false;
static bool sensor_loop_disable = false;
static bool debug_detect_0, debug_detect_1, debug_detect_2;
static unsigned int debug_seen_0, debug_seen_1, debug_seen_2;

#define SENSOR_LOOP_UPDATE_PERIOD_MS 5

static unsigned int poll_first_sensor(void) {
    debug_detect_0 = Cy_GPIO_Read(Detect_0_PORT, Detect_0_NUM);
    if (debug_detect_0) {
        debug_seen_0++;
    }
    debug_detect_1 = Cy_GPIO_Read(Detect_1_PORT, Detect_1_NUM);
    if (debug_detect_1) {
        debug_seen_1++;
    }
    debug_detect_2 = Cy_GPIO_Read(Detect_2_PORT, Detect_2_NUM);
    if (debug_detect_2) {
        debug_seen_2++;
    }
    if (debug_detect_0) {
        return 1;
    } else if (debug_detect_1) {
        return 2;
    } else if (debug_detect_2) {
        return 3;
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

    if (sensor_active == 0) {
        // no need to notify
        return;
    }

    if (xSemaphoreTake(marble_column_mutex, portMAX_DELAY) != pdTRUE) {
        err_mutex_mishandle = true;
        return;
    }

    if (marble_column != sensor_active) {
        marble_column = sensor_active;
        marble_detected_at = detected;
        // we don't care if this fails, because that just means the last notification is still pending
        (void) xSemaphoreGive(marble_column_notify);
    }

    if (xSemaphoreGive(marble_column_mutex) != pdTRUE) {
        err_mutex_mishandle = true;
    }
}

static void run_sensor_loop(void *unused) {
    (void) unused;

    uart_send("sensor driver start\r\n");

    const TickType_t frequency = SENSOR_LOOP_UPDATE_PERIOD_MS / portTICK_PERIOD_MS;
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&last_wake_time, frequency);
        if (!sensor_loop_disable) {
            poll_sensor_state();
        }
    }
}

// as defined in 4.5
void initialize_sensor_driver(void) {
    marble_column_mutex = xSemaphoreCreateMutex();
    if (marble_column_mutex == NULL) {
        uart_send("sensor driver NMUTEX\r\n");
        return;
    }

    marble_column_notify = xSemaphoreCreateBinary();
    if (marble_column_notify == NULL) {
        uart_send("sensor driver NSEMAPHORE\r\n");
        return;
    }

    debug_text("sensor_driver(MCM=");
    debug_mutex_state(&marble_column_mutex);
    debug_text(" ERRMX=");
    debug_boolean("sensor_errmx", &err_mutex_mishandle);
    debug_text(" D0=");
    debug_boolean(NULL, &debug_detect_0);
    debug_text(" D1=");
    debug_boolean(NULL, &debug_detect_1);
    debug_text(" D2=");
    debug_boolean(NULL, &debug_detect_2);
    debug_text(" S0=");
    debug_integer("s0", &debug_seen_0, 8);
    debug_text(" S1=");
    debug_integer("s1", &debug_seen_1, 8);
    debug_text(" S2=");
    debug_integer("s2", &debug_seen_2, 8);
    debug_text(" ");
    void *token = debug_with_mutex(marble_column_mutex);
    debug_text(" MC=");
    debug_integer("mc", &marble_column, 1);
    debug_text(" SLD=");
    debug_boolean("sld", &sensor_loop_disable);
    debug_text(" MDA=");
    debug_ticktype("mda", &marble_detected_at);
    debug_text(" MCN=");
    debug_semaphore_state("mcn", &marble_column_notify);
    debug_end_mutex(token);
    debug_text(") ");

    if (xTaskCreate(run_sensor_loop, "sensor_poller", 400, NULL, 4, NULL) != pdPASS) {
        uart_send("sensor driver NTASK\r\n");
    }
}

/* [] END OF FILE */
