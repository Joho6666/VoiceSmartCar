#include "car_core.h"
#include "action_sequence.h"
#include "board_config.h"
#include "motion_controller.h"
#include "voice_protocol.h"

#ifdef VOICECAR_HOST_TEST
static u8 enter_critical(void) { return 0; }
static void leave_critical(u8 previous) { (void)previous; }
#else
#include <REG52.H>
static u8 enter_critical(void)
{
    u8 previous = EA ? 1 : 0;
    EA = 0;
    return previous;
}
static void leave_critical(u8 previous) { if (previous) EA = 1; }
#endif

static volatile u8 s_rx_data[RX_QUEUE_SIZE];
static volatile u8 s_rx_head;
static volatile u8 s_rx_tail;
static volatile u8 s_rx_count;
static volatile u8 s_rx_overrun;
static volatile u16 s_now_ms;
static volatile u8 s_main_stale_ticks;

void car_init(void)
{
    s_rx_head = 0;
    s_rx_tail = 0;
    s_rx_count = 0;
    s_rx_overrun = 0;
    s_now_ms = 0;
    s_main_stale_ticks = 0;
    motion_init();
    action_init();
}

void car_rx_isr_byte(u8 byte)
{
    if (voice_is_stop(byte)) {
        motion_request_emergency_stop();
        s_rx_head = 0;
        s_rx_tail = 0;
        s_rx_count = 0;
        s_rx_overrun = 0;
    }
    if (s_rx_count >= RX_QUEUE_SIZE) {
        motion_stop_immediate(CAR_FAULT_QUEUE);
        s_rx_head = 0;
        s_rx_tail = 0;
        s_rx_count = 0;
        s_rx_overrun = 1;
        return;
    }
    s_rx_data[s_rx_head] = byte;
    s_rx_head++;
    if (s_rx_head >= RX_QUEUE_SIZE) s_rx_head = 0;
    s_rx_count++;
}

void car_rx_isr_error(void)
{
    motion_stop_immediate(CAR_FAULT_UART);
    s_rx_head = 0;
    s_rx_tail = 0;
    s_rx_count = 0;
    s_rx_overrun = 1;
}

void car_timer_10ms_isr(void)
{
    s_now_ms = (u16)(s_now_ms + 10U);
    motion_tick_10ms(s_now_ms);
    if (s_main_stale_ticks < 255) s_main_stale_ticks++;
    if (s_main_stale_ticks >= 10 && motion_has_pending_motion())
        motion_stop_immediate(CAR_FAULT_MAIN_STALL);
}

void car_pwm_isr(void) { motion_pwm_tick(); }

static u8 queue_pop(u8 *out)
{
    u8 previous = enter_critical();
    if (s_rx_count == 0) {
        leave_critical(previous);
        return 0;
    }
    *out = s_rx_data[s_rx_tail];
    s_rx_tail++;
    if (s_rx_tail >= RX_QUEUE_SIZE) s_rx_tail = 0;
    s_rx_count--;
    leave_critical(previous);
    return 1;
}

void car_poll(void)
{
    u8 byte;
    u16 now;
    u8 previous = enter_critical();
    now = s_now_ms;
    s_main_stale_ticks = 0;
    leave_critical(previous);
    if (s_rx_overrun) {
        s_rx_overrun = 0;
        action_abort();
    }
    while (queue_pop(&byte)) {
        voice_command_t decoded = voice_decode(byte);
        if (decoded == VOICE_INVALID) continue;
        if (decoded == VOICE_STOP) {
            action_abort();
            motion_command(byte, now);
        } else if (decoded == VOICE_SEQUENCE_1) {
            action_start(1, now);
        } else if (decoded == VOICE_SEQUENCE_2) {
            action_start(2, now);
        } else if (decoded == VOICE_FORWARD || decoded == VOICE_REVERSE ||
                   decoded == VOICE_LEFT || decoded == VOICE_RIGHT) {
            action_abort();
            motion_command(byte, now);
        } else {
            motion_command(byte, now);
        }
    }
    action_tick(now);
}

u16 car_now_ms(void) { return s_now_ms; }
u8 car_queue_size(void) { return s_rx_count; }
