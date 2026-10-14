#include "motion_controller.h"

#define CMD_FORWARD 0xA1
#define CMD_REVERSE 0xA2
#define CMD_STOP    0xA3
#define CMD_FASTER  0xA4
#define CMD_SLOWER  0xA5
#define CMD_LEFT    0xA6
#define CMD_RIGHT   0xA7

static motion_t s_motion;
static car_fault_t s_fault;
static u8 s_duty;
static u8 s_pwm_phase;
static u8 s_duty_left;
static u8 s_duty_right;
static u8 s_stop_latched;
static u8 s_emergency_request;
static u16 s_motion_deadline;
static u16 s_deadtime_deadline;
static motion_t s_pending_motion;
static u8 s_deadtime_active;
static u8 s_pwm_left;
static u8 s_pwm_right;

static void set_motion(motion_t next)
{
    s_motion = next;
    s_duty_left = 0;
    s_duty_right = 0;
    if (next == MOTION_FORWARD || next == MOTION_REVERSE) {
        s_duty_left = s_duty;
        s_duty_right = s_duty;
    } else if (next == MOTION_LEFT) {
        s_duty_right = s_duty;
    } else if (next == MOTION_RIGHT) {
        s_duty_left = s_duty;
    }
}

static u8 direction_reverses(motion_t from, motion_t to)
{
    if (from == MOTION_FORWARD && to == MOTION_REVERSE) return 1;
    if (from == MOTION_REVERSE && to == MOTION_FORWARD) return 1;
    if (from == MOTION_LEFT && to == MOTION_RIGHT) return 1;
    if (from == MOTION_RIGHT && to == MOTION_LEFT) return 1;
    if (from == MOTION_REVERSE && to == MOTION_RIGHT) return 1;
    if (from == MOTION_RIGHT && to == MOTION_REVERSE) return 1;
    if (from == MOTION_REVERSE && to == MOTION_LEFT) return 1;
    if (from == MOTION_LEFT && to == MOTION_REVERSE) return 1;
    return 0;
}

void motion_init(void)
{
    s_motion = MOTION_STOP;
    s_fault = CAR_OK;
    s_duty = 16;
    s_pwm_phase = 0;
    s_duty_left = 0;
    s_duty_right = 0;
    s_stop_latched = 0;
    s_emergency_request = 0;
    s_motion_deadline = 0;
    s_deadtime_deadline = 0;
    s_pending_motion = MOTION_STOP;
    s_deadtime_active = 0;
    s_pwm_left = 0;
    s_pwm_right = 0;
}

void motion_request_emergency_stop(void)
{
    s_stop_latched = 1;
    s_emergency_request = 1;
    s_pwm_left = 0;
    s_pwm_right = 0;
}

void motion_stop_immediate(car_fault_t fault)
{
    s_stop_latched = 1;
    s_fault = fault;
    s_emergency_request = 1;
    s_motion = MOTION_STOP;
    s_duty_left = 0;
    s_duty_right = 0;
    s_pwm_left = 0;
    s_pwm_right = 0;
}

void motion_auto_stop(void)
{
    s_motion = MOTION_STOP;
    s_duty_left = 0;
    s_duty_right = 0;
    s_deadtime_active = 0;
    s_pending_motion = MOTION_STOP;
}

void motion_clear_fault(void)
{
    s_fault = CAR_OK;
}

void motion_command(u8 command, u16 now_ms)
{
    if (command == CMD_STOP) {
        s_stop_latched = 0;
        s_emergency_request = 0;
        s_fault = CAR_OK;
        s_motion_deadline = now_ms;
        s_deadtime_active = 0;
        s_pending_motion = MOTION_STOP;
        set_motion(MOTION_STOP);
        return;
    }
    if (s_stop_latched || s_fault != CAR_OK) {
        return;
    }
    if (command == CMD_FASTER) {
        if (s_duty < 28) {
            s_duty += 4;
            if (s_duty > 28) s_duty = 28;
        }
        if (s_motion != MOTION_STOP && !s_deadtime_active) set_motion(s_motion);
    } else if (command == CMD_SLOWER) {
        if (s_duty > 8) {
            s_duty -= 4;
            if (s_duty < 8) s_duty = 8;
        }
        if (s_motion != MOTION_STOP && !s_deadtime_active) set_motion(s_motion);
    } else if (command == CMD_FORWARD || command == CMD_REVERSE ||
               command == CMD_LEFT || command == CMD_RIGHT) {
        motion_t requested = MOTION_STOP;
        if (command == CMD_FORWARD) requested = MOTION_FORWARD;
        else if (command == CMD_REVERSE) requested = MOTION_REVERSE;
        else if (command == CMD_LEFT) requested = MOTION_LEFT;
        else requested = MOTION_RIGHT;
        if (s_deadtime_active) {
            s_pending_motion = requested;
        } else if (direction_reverses(s_motion, requested)) {
            s_motion = MOTION_STOP;
            s_duty_left = 0;
            s_duty_right = 0;
            s_pending_motion = requested;
            s_deadtime_deadline = (u16)(now_ms + 100U);
            s_deadtime_active = 1;
        } else {
            set_motion(requested);
        }
    } else {
        return;
    }
    if (s_motion != MOTION_STOP) {
        s_motion_deadline = (u16)(now_ms + 3000U);
    }
}

void motion_tick_10ms(u16 now_ms)
{
    if (s_stop_latched) {
        if (s_emergency_request) {
            s_motion = MOTION_STOP;
            s_duty_left = 0;
            s_duty_right = 0;
            s_emergency_request = 0;
        }
        return;
    }
    if (s_deadtime_active && (u16)(now_ms - s_deadtime_deadline) < 0x8000U) {
        motion_t target = s_pending_motion;
        s_deadtime_active = 0;
        s_pending_motion = MOTION_STOP;
        set_motion(target);
        s_motion_deadline = (u16)(now_ms + 3000U);
    }
    if (s_motion != MOTION_STOP && (u16)(now_ms - s_motion_deadline) < 0x8000U) {
        motion_stop_immediate(CAR_OK);
    }
}

void motion_pwm_tick(void)
{
    s_pwm_phase++;
    if (s_pwm_phase >= 32) s_pwm_phase = 0;
    if (s_stop_latched || s_fault != CAR_OK || s_motion == MOTION_STOP) {
        s_pwm_left = 0;
        s_pwm_right = 0;
    } else {
        s_pwm_left = (s_pwm_phase < s_duty_left) ? 1 : 0;
        s_pwm_right = (s_pwm_phase < s_duty_right) ? 1 : 0;
    }
}

motion_t motion_get_state(void) { return s_motion; }
u8 motion_get_duty_left(void) { return s_duty_left; }
u8 motion_get_duty_right(void) { return s_duty_right; }
u8 motion_get_fault(void) { return (u8)s_fault; }
u8 motion_get_stop_latched(void) { return s_stop_latched; }
u8 motion_has_pending_motion(void) { return (s_motion != MOTION_STOP || s_deadtime_active) ? 1 : 0; }
u8 motion_left_pwm(void) { return s_pwm_left; }
u8 motion_right_pwm(void) { return s_pwm_right; }
