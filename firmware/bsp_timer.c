#include "bsp_timer.h"
#include "board_config.h"
#include "car_core.h"
#include "bsp_motor.h"
#include <REG52.H>

void bsp_timer_init(void)
{
    T2CON = 0x00U;
    RCAP2H = TIMER2_RELOAD_H;
    RCAP2L = TIMER2_RELOAD_L;
    TH2 = TIMER2_RELOAD_H;
    TL2 = TIMER2_RELOAD_L;
    ET2 = 1;
    TR2 = 1;
}

void bsp_timer2_isr(void) interrupt 5
{
    static u8 sub_ticks;
    TF2 = 0;
    car_pwm_isr();
    bsp_motor_refresh();
    sub_ticks++;
    if (sub_ticks >= 36) {
        sub_ticks = 0;
        car_timer_10ms_isr();
    }
}
