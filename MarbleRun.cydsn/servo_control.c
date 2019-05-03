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
    PWMArmSpin_SetCompare0(angle_to_compare(angles.arm_spin));
    PWMArmRaiseL_SetCompare0(angle_to_compare(angles.arm_left));
    PWMArmRaiseR_SetCompare0(angle_to_compare(angles.arm_right));
    PWMArmGrip_SetCompare0(angle_to_compare(angles.arm_grip));
}

// as defined in 1.2.3
void initialize_servos(void) {
    PWMArmSpin_Start();
    PWMArmRaiseR_Start();
    PWMArmRaiseL_Start();
    PWMArmGrip_Start();
}

/* [] END OF FILE */
