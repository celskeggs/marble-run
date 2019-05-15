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

// text must be statically allocated
extern void debug_text(const char *text);
// do not nest these
extern void *debug_with_mutex(SemaphoreHandle_t mutex);
extern void debug_end_mutex(void *token);

extern void debug_boolean(const char *prodname, bool *variable);
extern void debug_integer(const char *prodname, unsigned int *variable, int digits);
extern void debug_ticktype(const char *prodname, TickType_t *variable);
extern void debug_float(const char *prodname, float *variable, int digits);
extern void debug_servo_point(const char *prodname, struct servo_point *variable);
extern void debug_servo_velocity(const char *prodname, struct servo_velocity *variable);
extern void debug_mutex_state(SemaphoreHandle_t *variable);
extern void debug_semaphore_state(const char *prodname, SemaphoreHandle_t *variable);

#define debug_start(name) debug_text(name "=")
#define debug_end() debug_text(" ")
#else
#define initialize_debugger() do {} while(0)
#define start_debugger() do {} while(0)

#define debug_text(text) do {} while(0)
#define debug_with_mutex(mutex) NULL
#define debug_end_mutex(token) do { (void) token; } while(0)

#define debug_boolean(prodname, variable) do {} while(0)
#define debug_integer(prodname, variable, digits) do {} while(0)
#define debug_ticktype(prodname, variable) do {} while(0)
#define debug_float(prodname, variable, digits) do {} while(0)
#define debug_servo_point(prodname, variable) do {} while(0)
#define debug_servo_velocity(prodname, variable) do {} while(0)
#define debug_mutex_state(variable) do {} while(0)
#define debug_semaphore_state(prodname, variable) do {} while(0)
#endif

#endif
/* [] END OF FILE */
