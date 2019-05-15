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
#include "debug.h"

// as defined in 7.2
// TODO: initialize
struct graph_node drop_off, drop_off_waypoint, pick_up_waypoint;
struct servo_point * pick_ups;
struct servo_point graph1[] = {{0, -70, 175, 30}}; // drop
struct servo_point graph2[] = {{0, -70, 130, -40}}; // drop waypoint
struct servo_point graph3[] = {{0, -70, -50, 165}}; // pickup waypoint
struct servo_point graph4[] = {{-10, -70, 0, 175}, {10, -70, 0, 175}, {30, -70, 0, 175}}; // pickup positions; only center is unestimated
struct servo_point *graph[] = {graph1, graph2, graph3, graph4};
// (W1=(S 0.0000 G-70.00 L175.00 R30.000) W2=(S 0.0000 G-70.00 L130.00 R-40.00) W3=(S 0.0000 G-70.00 L-50.00 R165.00) W40=(S 10.000 G-70.00 L0.0000 R175.00) W41=(S 10.000 G-70.00 L0.0000 R175.00) W42=(S 10.000 G-70.00 L0.0000 R175.00)
void initialize_graph(void) {
    debug_text("path_planning(");
    debug_text("W1=");
    debug_servo_point("w1", &graph1[0]);
    debug_text(" W2=");
    debug_servo_point("w2", &graph2[0]);
    debug_text(" W3=");
    debug_servo_point("w3", &graph3[0]);
    debug_text(" W40=");
    debug_servo_point("w40", &graph4[0]);
    debug_text(" W41=");
    debug_servo_point("w41", &graph4[1]);
    debug_text(" W42=");
    debug_servo_point("w42", &graph4[2]);
    debug_text(") ");
}

/* [] END OF FILE */
