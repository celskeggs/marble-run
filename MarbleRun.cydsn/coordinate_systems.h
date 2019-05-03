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
#ifndef COORDINATE_SYSTEMS_H
#define COORDINATE_SYSTEMS_H

// as defined in 2.1
// measured in the range -90.0 degrees to 90.0 degrees
struct servo_point {
    float arm_spin;
    float arm_grip;
    float arm_left;
    float arm_right;
};

// as defined in 2.2
// measured in degrees per second
struct servo_velocity {
    float arm_spin;
    float arm_grip;
    float arm_left;
    float arm_right;
};

#endif

/* [] END OF FILE */
