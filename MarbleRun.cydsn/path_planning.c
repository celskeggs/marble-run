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
struct servo_point graph1[] = {{0, -70, 175, 60}}; // drop
struct servo_point graph2[] = {{0, -70, 175, -40}}; // drop waypoint
struct servo_point graph3[] = {{0, -70, 130, -40}}; // drop waypoint
struct servo_point graph4[] = {{0, -70, 30, -40}}; // drop waypoint
struct servo_point graph5[] = {{0, -70, -50, 165}}; // pickup waypoint
struct servo_point graph6[] = {{30, -60, 30, 175}, {0, -60, 30, 175}, {-20, -60, 30, 175}}; // pickup positions
struct servo_point *graph[] = {graph1, graph2, graph3, graph4, graph5, graph6};
// DEBUG at T3121707   : uart(OVR=false) servo_control(neglim=120 poslim=180 lwrite=(S 0.0000 G-70.00 L-50.00 R165.00)) control_loop(PM=-unowned RUNFQ=20.00 ERRMX=false MAXSV=(s 40.000 g150.00 l70.000 r130.00) MAXHV=(s 360.00 g300.00 l300.00 r300.00) +{ DPOS=(S 0.0000 G-70.00 L-50.00 R165.00) IPOS=(S 0.0000 G-70.00 L-50.00 R165.00) TPOS=(S 0.0000 G-70.00 L-50.00 R165.00) ATP=true  SM=false ATPN=0   *+) sensor_driver(MCM=-unowned ERRMX=false D0=false D1=false D2=false S0=1067     S1=133      S2=478      +{ MC=0 SLD=false MDA=T1858436    MCN=0   *+) path_planning(W1=(S 0.0000 G-70.00 L175.00 R60.000) W2=(S 0.0000 G-70.00 L175.00 R-40.00) W3=(S 0.0000 G-70.00 L130.00 R-40.00) W4=(S 0.0000 G-70.00 L30.000 R-40.00) W5=(S 0.0000 G-70.00 L-50.00 R165.00) W60=(S 30.000 G-60.00 L30.000 R175.00) W61=(S 0.0000 G-60.00 L30.000 R175.00) W62=(S -20.00 G-60.00 L30.000 R175.00)) sequencer(drop=0.500 pickup=2.000 stop=false)

void initialize_graph(void) {
    debug_text("path_planning(");
    debug_text("W1=");
    debug_servo_point("w1", &graph1[0]);
    debug_text(" W2=");
    debug_servo_point("w2", &graph2[0]);
    debug_text(" W3=");
    debug_servo_point("w3", &graph3[0]);
    debug_text(" W4=");
    debug_servo_point("w4", &graph4[0]);
    debug_text(" W5=");
    debug_servo_point("w5", &graph5[0]);
    debug_text(" W60=");
    debug_servo_point("w60", &graph6[0]);
    debug_text(" W61=");
    debug_servo_point("w61", &graph6[1]);
    debug_text(" W62=");
    debug_servo_point("w62", &graph6[2]);
    debug_text(") ");
}

/* [] END OF FILE */
