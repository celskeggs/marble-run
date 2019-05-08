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

#include "path_planning.h"

// as defined in 7.2
// TODO: initialize
struct graph_node drop_off, drop_off_waypoint, pick_up_waypoint;
struct servo_point * pick_ups;
struct servo_point graph1[] = {{10, 10, 10, 10}};
struct servo_point graph2[] = {{20, 20, 20, 20}};
struct servo_point graph3[] = {{30, 30, 30, 30}};
struct servo_point graph4[] = {{40, 0, 0, 0}, {40, 10, 10, 10}, {40, 20, 20, 20}, {40, 30, 30, 30}, {40, 40, 40, 40}};
struct servo_point *graph[] = {graph1, graph2, graph3, graph4};

/* [] END OF FILE */
