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
#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

#include "coordinate_systems.h"
#include <stdbool.h>
#include <FreeRTOS.h>
#include <semphr.h>

// as defined in 3.2.1
extern struct servo_point target_position;

// as defined in 3.2.1.2
extern SemaphoreHandle_t position_mutex;

// as defined in 3.2.2
extern struct servo_point inferred_position;

// as defined in 3.2.3
extern bool at_target_position;
// TODO: condition variable

// as defined in 3.5
extern void initialize_control_loop(void);

#endif
/* [] END OF FILE */
