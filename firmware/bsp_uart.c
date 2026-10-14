#include "bsp_uart.h"
#include "board_config.h"
#include <REG52.H>

void bsp_uart_init(void)
{
    TMOD = (TMOD & 0x0FU) | 0x20U;
    TH1 = TIMER1_BAUD_RELOAD;
    TL1 = TIMER1_BAUD_RELOAD;
    PCON &= 0x7FU;
    SCON = 0x50U;
    TR1 = 1;
    ES = 1;
}

void bsp_uart_service(void) { }
