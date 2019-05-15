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

extern void initialize_uart(void);

// only usable in components besides debug during init
extern void uart_send(const char *message);

#endif
/* [] END OF FILE */
