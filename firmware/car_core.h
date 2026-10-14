#ifndef CAR_CORE_H
#define CAR_CORE_H
#include "car_types.h"

void car_init(void);
void car_rx_isr_byte(u8 byte);
void car_rx_isr_error(void);
void car_timer_10ms_isr(void);
void car_pwm_isr(void);
void car_poll(void);
u16 car_now_ms(void);
u8 car_queue_size(void);

#endif
