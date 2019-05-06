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
#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <stdint.h>
#include <FreeRTOS.h>
#include <semphr.h>

// as defined in 4.3.1
extern unsigned int marble_column;
// TODO: condition variable

// as defined in 4.3.2
extern TickType_t marble_detected_at;

// as defined in 4.3.1.2
extern SemaphoreHandle_t marble_column_mutex;

// as defined in 4.5
extern void initialize_sensor_driver(void);

#endif
/* [] END OF FILE */
