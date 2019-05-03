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

// as defined in 3.2.1
struct servo_point target_position; // TODO: initialize

// as defined in 3.2.1.2
SemaphoreHandle_t position_mutex; // TODO: initialize

// as defined in 3.2.2
struct servo_point inferred_position; // TODO: initialize

// as defined in 3.2.3
bool at_target_position; // TODO: initialize
// TODO: condition variable

// as defined in 3.2.4
static struct servo_point driven_position; // TODO: initialize

// as defined in 3.3.1
static struct servo_velocity maximum_speed; // TODO: configure

// as defined in 3.4
static void update_control_loop(void) {
    // STUB
}

// as defined in 3.5
void initialize_control_loop(void) {
    // STUB
}

/* [] END OF FILE */
