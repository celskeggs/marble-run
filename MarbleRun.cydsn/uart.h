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
#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdbool.h>

#define UART_BUFLEN 128

extern void initialize_uart(void);

extern bool uart_input_overrun;

// uart is not threadsafe; may only be called from debug task or during init

extern size_t uart_get_line(void);
extern const char *uart_get_buffer(void);
extern void uart_chew_line(void);

extern void uart_send(const char *message);

#endif
/* [] END OF FILE */
