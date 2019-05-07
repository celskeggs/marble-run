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
#ifndef DEBUG_H
#define DEBUG_H

// note: debugging code not covered by the specification, because it can be compiled out.
#include "coordinate_systems.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <stdbool.h>

#define MARBLE_DEBUG

#ifdef MARBLE_DEBUG
extern void initialize_debugger(void);
extern void start_debugger(void);

// only usable in other components during init
extern void uart_send(const char *message);

// text must be statically allocated
extern void debug_text(const char *text);
// do not nest these
extern void *debug_with_mutex(SemaphoreHandle_t mutex);
extern void debug_end_mutex(void *token);

extern void debug_boolean(bool *variable);
extern void debug_integer(unsigned int *variable, int digits);
extern void debug_ticktype(TickType_t *variable);
extern void debug_float(float *variable, int digits);
extern void debug_servo_point(struct servo_point *variable);
extern void debug_servo_velocity(struct servo_velocity *variable);
extern void debug_mutex_state(SemaphoreHandle_t *variable);
extern void debug_semaphore_state(SemaphoreHandle_t *variable);

#define debug_start(name) debug_text(name "=")
#define debug_end() debug_text(" ")
#else
#define initialize_debugger() do {} while(0)
#define start_debugger() do {} while(0)
    
#define uart_send(message) do {} while(0)

#define debug_text(text) do {} while(0)
#define debug_with_mutex(mutex) NULL
#define debug_end_mutex(token) do {} while(0)

#define debug_boolean(variable) do {} while(0)
#define debug_integer(variable, digits) do {} while(0)
#define debug_ticktype(variable) do {} while(0)
#define debug_float(variable) do {} while(0)
#define debug_servo_point(variable) do {} while(0)
#define debug_servo_velocity(variable) do {} while(0)
#define debug_mutex_state(variable) do {} while(0)
#define debug_semaphore_state(variable) do {} while(0)
#endif

#endif
/* [] END OF FILE */
