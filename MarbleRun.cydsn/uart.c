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

void initialize_uart(void) {
    UART_1_Init(&UART_1_config);

    /* Hook interrupt service routine and enable interrupt */
    Cy_SysInt_Init(&UART_1_SCB_IRQ_cfg, &UART_1_Interrupt);
    NVIC_EnableIRQ(UART_1_SCB_IRQ_cfg.intrSrc);

    UART_1_Enable();
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

/* [] END OF FILE */
