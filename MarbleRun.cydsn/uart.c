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

#include "uart.h"
#include "project.h"
#include <string.h>
#include <assert.h>

bool uart_input_overrun = false; // TODO: add debug for this
static char uart_nextchar;
static char uart_buffer[UART_BUFLEN];
static size_t uart_collected_length = 0;
static size_t uart_line_length = 0;

void uart_callback(uint32_t event) {
    if (event == CY_SCB_UART_RECEIVE_DONE_EVENT) {
        if (uart_line_length == 0) {
            assert(uart_collected_length < UART_BUFLEN - 1);
            if (uart_nextchar == '\n' || uart_nextchar == '\r') {
                uart_buffer[uart_collected_length] = '\0';
                uart_line_length = uart_collected_length;
                uart_collected_length = 0;
            } else if (uart_nextchar == '\b' || uart_nextchar == 127) {
                if (uart_collected_length > 0) {
                    uart_collected_length--;
                }
            } else {
                uart_buffer[uart_collected_length++] = uart_nextchar;
                if (uart_collected_length >= UART_BUFLEN - 1) {
                    uart_buffer[uart_collected_length] = '\0';
                    uart_line_length = UART_BUFLEN - 1;
                    uart_collected_length = 0;
                }
            }
        } else {
            uart_input_overrun = true;
        }
        UART_1_Receive(&uart_nextchar, 1);
    }
}

void initialize_uart(void) {
    UART_1_Init(&UART_1_config);

    /* Hook interrupt service routine and enable interrupt */
    Cy_SysInt_Init(&UART_1_SCB_IRQ_cfg, &UART_1_Interrupt);
    NVIC_EnableIRQ(UART_1_SCB_IRQ_cfg.intrSrc);

    UART_1_RegisterCallback(uart_callback);

    UART_1_Enable();

    UART_1_Receive(&uart_nextchar, 1);
}

void uart_send(const char *string) {
    cy_en_scb_uart_status_t err;
    size_t len = strlen(string);

    err = UART_1_Transmit((void *) string, len);
    if (err != CY_SCB_UART_TRANSMIT_BUSY) {
        return;
    }
    
    int irqs_disabled = __get_PRIMASK();
    if (irqs_disabled) {
        // TODO: something more reliable?
        __enable_irq();
    }
    while (err == CY_SCB_UART_TRANSMIT_BUSY) {
        err = UART_1_Transmit((void *) string, len);
    }
    if (irqs_disabled) {
        __disable_irq();
    }
}

size_t uart_get_line(void) {
    return uart_line_length;
}

const char *uart_get_buffer(void) {
    return uart_buffer;
}

void uart_chew_line(void) {
    memset(uart_buffer, '~', UART_BUFLEN - 1);
    uart_buffer[UART_BUFLEN - 1] = '\0';
    uart_line_length = 0;
}


/* [] END OF FILE */
