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

#include <stdbool.h>

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

// not in spec
static inline bool servo_point_equal(struct servo_point a, struct servo_point b) {
    return a.arm_spin == b.arm_spin
        && a.arm_grip == b.arm_grip
        && a.arm_left == b.arm_left
        && a.arm_right == b.arm_right;
}

// not in spec
static inline struct servo_point servo_multiply_veloctiy(struct servo_velocity velocity, float interval_sec) {
    return (struct servo_point) {
        .arm_spin = interval_sec * velocity.arm_spin,
        .arm_grip = interval_sec * velocity.arm_grip,
        .arm_left = interval_sec * velocity.arm_left,
        .arm_right = interval_sec * velocity.arm_right,
    };
}

#endif

/* [] END OF FILE */
