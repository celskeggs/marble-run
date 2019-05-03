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
#ifndef PATH_PLANNING_H
#define PATH_PLANNING_H

#include "coordinate_systems.h"

// as defined in 7.1
struct graph_node {
    struct servo_point position;
    struct graph_node *marble;
    struct graph_node *no_marble;
    int node_order;
};

// as defined in 7.2
extern struct graph_node drop_off, drop_off_waypoint, pick_up_waypoint;
extern struct graph_node pick_ups[];

#endif
/* [] END OF FILE */
