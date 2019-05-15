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
#ifndef CONSOLE_H
#define CONSOLE_H
#include <stddef.h>

extern void initialize_console(void);

struct console_def {
    const char *name;
    unsigned int argcount;
    // can call uart functions
    void (*cb)(void);
    struct console_def *next;
};

extern void console_register(struct console_def *def);

extern const char *console_get_string(unsigned int arg);

extern int console_get_int(unsigned int arg);

// must be able to call uart functions
extern void console_perform(const char *input, size_t length);

#endif
/* [] END OF FILE */
