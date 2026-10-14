#ifndef MOTION_CONTROLLER_H
#define MOTION_CONTROLLER_H
#include "car_types.h"

void motion_init(void);
void motion_command(u8 command, u16 now_ms);
void motion_tick_10ms(u16 now_ms);
void motion_stop_immediate(car_fault_t fault);
void motion_auto_stop(void);
void motion_request_emergency_stop(void);
void motion_pwm_tick(void);
u8 motion_has_pending_motion(void);
u8 motion_left_pwm(void);
u8 motion_right_pwm(void);
void motion_clear_fault(void);
motion_t motion_get_state(void);
u8 motion_get_duty_left(void);
u8 motion_get_duty_right(void);
u8 motion_get_fault(void);
u8 motion_get_stop_latched(void);

#endif
