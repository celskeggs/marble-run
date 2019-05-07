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
#include "servo_control.h"
#include "control_loop.h"
#include "sensor_driver.h"
#include "sequencer.h"
#include "debug.h"
#include <FreeRTOS.h>
#include <task.h>

int main(void) {
    initialize_debugger();
    initialize_servos();
    initialize_control_loop();
    initialize_sensor_driver();
    initialize_sequencer();
    start_debugger();

    uart_send("starting scheduler\r\n");
    vTaskStartScheduler();

    // loop forever if we can't start the task scheduler
    for(;;) {}
}

/* [] END OF FILE */
