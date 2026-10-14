#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

/* Profile IDs are passed by each Keil target. */
#define CLOCK_HZ        11059200UL
#define BAUD_RATE       9600UL
#define RX_QUEUE_SIZE   16
#define PWM_LEVELS      32
#define PWM_START       16
#define PWM_MIN         8
#define PWM_MAX         28
#define PWM_STEP        4
#define MOTION_TIMEOUT_MS 3000U
#define MAIN_TIMEOUT_MS  100U
#define REVERSAL_DEADTIME_MS 100U

/* Unknown PCB GPIO mapping. Set only after continuity measurements. */
#define BOARD_IO_MAPPING_CONFIRMED 0
#define BOARD_SAFE_LOGIC_RAIL_CONFIRMED 0
#define BOARD_RESET_MOTOR_OFF_CONFIRMED 0

#if defined(VOICECAR_HARDWARE)
#if !BOARD_IO_MAPPING_CONFIRMED
#error Hardware target blocked: verify MCU-to-L293D pin mapping first.
#endif
#if !BOARD_SAFE_LOGIC_RAIL_CONFIRMED
#error Hardware target blocked: verify the regulated MCU logic rail first.
#endif
#if !BOARD_RESET_MOTOR_OFF_CONFIRMED
#error Hardware target blocked: verify motor outputs stay off during reset.
#endif
#endif

#if defined(VOICECAR_6T)
#define TIMER1_BAUD_RELOAD 0xFA
#define TIMER2_RELOAD_H 0xFE
#define TIMER2_RELOAD_L 0x00
#else
#define TIMER1_BAUD_RELOAD 0xFD
#define TIMER2_RELOAD_H 0xFF
#define TIMER2_RELOAD_L 0x00
#endif

#endif
