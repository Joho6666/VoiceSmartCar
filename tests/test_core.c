#include <stdio.h>
#include <stdlib.h>
#include "action_sequence.h"
#include "board_config.h"
#include "car_core.h"
#include "motion_controller.h"
#include "voice_protocol.h"

static unsigned int tests;
static unsigned int failures;

#define CHECK(expr) do { tests++; if (!(expr)) { \
    failures++; printf("FAIL line %d: %s\n", __LINE__, #expr); \
} } while (0)

static void advance_10ms(unsigned int count)
{
    unsigned int i;
    for (i = 0; i < count; i++) {
        car_timer_10ms_isr();
        car_poll();
    }
}

static void test_protocol(void)
{
    static const unsigned char valid[] = {0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xB1,0xB2};
    unsigned int i;
    for (i = 0; i < sizeof(valid); i++) CHECK(voice_is_valid(valid[i]));
    CHECK(!voice_is_valid(0x00));
    CHECK(!voice_is_valid('A'));
    CHECK(!voice_is_valid('1'));
}

static void test_motion_and_speed(void)
{
    car_init();
    car_rx_isr_byte(0xA4); car_poll(); CHECK(motion_get_state() == MOTION_STOP);
    CHECK(motion_get_duty_left() == 0);
    car_rx_isr_byte(0xA5); car_poll(); CHECK(motion_get_duty_left() == 0);
    car_rx_isr_byte(0xA1); car_poll();
    CHECK(motion_get_state() == MOTION_FORWARD);
    CHECK(motion_get_duty_left() == 16);
    CHECK(motion_get_duty_right() == 16);
    car_rx_isr_byte(0xA4); car_poll(); CHECK(motion_get_duty_left() == 20);
    car_rx_isr_byte(0xA4); car_poll(); car_rx_isr_byte(0xA4); car_poll();
    car_rx_isr_byte(0xA4); car_poll(); car_rx_isr_byte(0xA4); car_poll();
    car_rx_isr_byte(0xA4); car_poll();
    CHECK(motion_get_duty_left() == 28);
    car_rx_isr_byte(0xA5); car_poll(); CHECK(motion_get_duty_left() == 24);
    car_rx_isr_byte(0xA6); car_poll();
    CHECK(motion_get_state() == MOTION_LEFT);
    CHECK(motion_get_duty_left() == 0 && motion_get_duty_right() == 24);
    car_rx_isr_byte(0xA7); car_poll(); CHECK(motion_get_state() == MOTION_STOP);
    advance_10ms(10);
    CHECK(motion_get_state() == MOTION_RIGHT);
    CHECK(motion_get_duty_left() == 24 && motion_get_duty_right() == 0);
    car_rx_isr_byte(0xA2); car_poll(); CHECK(motion_get_state() == MOTION_STOP);
    advance_10ms(10); CHECK(motion_get_state() == MOTION_REVERSE);
}

static void test_stop_timeout_and_errors(void)
{
    car_init();
    car_rx_isr_byte(0xA1); car_poll();
    car_rx_isr_byte(0x00); car_poll();
    CHECK(motion_get_state() == MOTION_FORWARD);
    advance_10ms(299); CHECK(motion_get_state() == MOTION_FORWARD);
    advance_10ms(1); CHECK(motion_get_state() == MOTION_STOP);
    CHECK(motion_get_stop_latched());
    car_rx_isr_byte(0xA1); car_poll(); CHECK(motion_get_state() == MOTION_STOP);
    car_rx_isr_byte(0xA3); car_poll(); CHECK(!motion_get_stop_latched());
    car_rx_isr_byte(0xA1); car_poll(); CHECK(motion_get_state() == MOTION_FORWARD);
    car_rx_isr_error(); car_poll(); CHECK(motion_get_state() == MOTION_STOP);
    CHECK(motion_get_fault() == CAR_FAULT_UART);
    car_rx_isr_byte(0xA2); car_poll(); CHECK(motion_get_state() == MOTION_STOP);
    car_rx_isr_byte(0xA3); car_poll(); CHECK(motion_get_fault() == CAR_OK);
}

static void test_queue_stop_priority(void)
{
    unsigned int i;
    car_init();
    for (i = 0; i < RX_QUEUE_SIZE; i++) car_rx_isr_byte(0x00);
    CHECK(car_queue_size() == RX_QUEUE_SIZE);
    car_rx_isr_byte(0xA1);
    CHECK(motion_get_fault() == CAR_FAULT_QUEUE);
    CHECK(motion_get_state() == MOTION_STOP);
    car_rx_isr_byte(0xA3);
    car_poll();
    CHECK(!motion_get_stop_latched());
    CHECK(motion_get_state() == MOTION_STOP);
    car_rx_isr_byte(0xA1); car_poll(); CHECK(motion_get_state() == MOTION_FORWARD);
    for (i = 0; i < RX_QUEUE_SIZE; i++) car_rx_isr_byte(0x00);
    car_rx_isr_byte(0xA3);
    CHECK(motion_get_stop_latched());
    CHECK(car_queue_size() == 1);
    car_poll(); CHECK(motion_get_state() == MOTION_STOP);
    CHECK(motion_get_fault() == CAR_OK);
}

static void test_sequences(void)
{
    car_init();
    car_rx_isr_byte(0xB1); car_poll();
    CHECK(action_active() && motion_get_state() == MOTION_FORWARD);
    advance_10ms(99); CHECK(motion_get_state() == MOTION_FORWARD);
    advance_10ms(1); CHECK(action_active() && action_stage() == 1);
    CHECK(motion_get_state() == MOTION_LEFT);
    advance_10ms(49); CHECK(action_active());
    advance_10ms(1);
    CHECK(!action_active() && motion_get_state() == MOTION_STOP);
    car_init();
    car_rx_isr_byte(0xB2); car_poll(); CHECK(motion_get_state() == MOTION_REVERSE);
    advance_10ms(69); CHECK(motion_get_state() == MOTION_REVERSE);
    advance_10ms(1); CHECK(motion_get_state() == MOTION_STOP);
    advance_10ms(10); CHECK(motion_get_state() == MOTION_RIGHT);
    advance_10ms(49); CHECK(motion_get_state() == MOTION_RIGHT);
    advance_10ms(1); CHECK(motion_get_state() == MOTION_STOP);
}

static void test_sequence_interrupt_and_replacement(void)
{
    car_init();
    car_rx_isr_byte(0xB1); car_poll();
    advance_10ms(30);
    car_rx_isr_byte(0xA3); car_poll();
    CHECK(!action_active() && motion_get_state() == MOTION_STOP);
    car_rx_isr_byte(0xA1); car_poll(); CHECK(motion_get_state() == MOTION_FORWARD);
    car_rx_isr_byte(0xB2); car_poll(); CHECK(motion_get_state() == MOTION_STOP);
    CHECK(action_active() && action_stage() == 0);
    advance_10ms(10); CHECK(motion_get_state() == MOTION_REVERSE);
    car_rx_isr_byte(0xA6); car_poll();
    CHECK(!action_active() && motion_get_state() == MOTION_STOP);
    advance_10ms(10); CHECK(motion_get_state() == MOTION_LEFT);
}

static void test_direction_deadtime_and_stall(void)
{
    unsigned int i;
    car_init();
    car_rx_isr_byte(0xA2); car_poll();
    car_rx_isr_byte(0xA7); car_poll();
    CHECK(motion_get_state() == MOTION_STOP);
    advance_10ms(9); CHECK(motion_get_state() == MOTION_STOP);
    advance_10ms(1); CHECK(motion_get_state() == MOTION_RIGHT);
    CHECK(motion_get_duty_left() == 16);
    advance_10ms(49); CHECK(motion_get_state() == MOTION_RIGHT);
    advance_10ms(1); CHECK(motion_get_state() == MOTION_RIGHT);
    advance_10ms(99); CHECK(motion_get_state() == MOTION_RIGHT);
    advance_10ms(1); CHECK(motion_get_state() == MOTION_RIGHT);
    car_init();
    car_rx_isr_byte(0xA1); car_poll();
    for (i = 0; i < 10; i++) car_timer_10ms_isr();
    CHECK(motion_get_state() == MOTION_STOP);
    CHECK(motion_get_fault() == CAR_FAULT_MAIN_STALL);
    CHECK(motion_get_stop_latched());
    car_rx_isr_byte(0xA3); car_poll();
    car_rx_isr_byte(0xA1); car_poll();
    CHECK(motion_get_state() == MOTION_FORWARD);
}

static void test_elapsed_clock_wrap(void)
{
    unsigned int i;
    car_init();
    for (i = 0; i < 6500; i++) {
        car_timer_10ms_isr();
        car_poll();
    }
    car_rx_isr_byte(0xA1); car_poll();
    advance_10ms(299); CHECK(motion_get_state() == MOTION_FORWARD);
    advance_10ms(1); CHECK(motion_get_state() == MOTION_STOP);
}

int main(void)
{
    test_protocol();
    test_motion_and_speed();
    test_stop_timeout_and_errors();
    test_queue_stop_priority();
    test_sequences();
    test_sequence_interrupt_and_replacement();
    test_direction_deadtime_and_stall();
    test_elapsed_clock_wrap();
    printf("%u assertions, %u failures\n", tests, failures);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
