#include "car_core.h"
#include "bsp_timer.h"
#include "bsp_uart.h"
#include "bsp_motor.h"

#if defined(VOICECAR_HOST_TEST)
int main(void)
{
    car_init();
    while (1) {
        car_poll();
    }
}
#else

#include <REG52.H>
#include "board_config.h"
#include "safety.h"

static void timers_uart_init(void)
{
    bsp_uart_init();
    bsp_timer_init();
    ES = 1;
    EA = 1;
}

void serial_isr(void) interrupt 4
{
    u8 byte;
    if (RI) {
        byte = SBUF;
        RI = 0;
        if (RB8) car_rx_isr_byte(byte);
        else safety_uart_error();
    }
    if (TI) TI = 0;
}

void main(void)
{
    bsp_motor_init_safe();
    car_init();
    timers_uart_init();
    while (1) {
        car_poll();
        bsp_motor_refresh();
    }
}
#endif
