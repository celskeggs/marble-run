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
#include "debug.h"

static struct servo_point last_written = {0, 0, 0, 0};

// as defined in 1.2.1
static int angle_to_compare(float angle) {
    if (angle < -90) {
        angle = -90;
    }
    if (angle > 90) {
        angle = 90;
    }
    double ms = 1.5 + (angle / 180.0);
    return (int) (100 * ms + 0.5);
}

// as defined in 1.2.2
void set_servos(struct servo_point angles) {
    last_written = angles;

    PWM_spin_SetCompare0(angle_to_compare(angles.arm_spin));
    PWM_left_SetCompare0(angle_to_compare(angles.arm_left));
    PWM_right_SetCompare0(angle_to_compare(angles.arm_right));
    PWM_grip_SetCompare0(angle_to_compare(angles.arm_grip));
}

// as defined in 1.2.3
void initialize_servos(void) {
    PWM_spin_Start();
    PWM_right_Start();
    PWM_left_Start();
    PWM_grip_Start();

    debug_text("servo_control(");
    debug_servo_point(&last_written);
    debug_text(") ");
}

/* [] END OF FILE */
