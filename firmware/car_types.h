#ifndef CAR_TYPES_H
#define CAR_TYPES_H

typedef unsigned char u8;
/* unsigned short is 16-bit in both Keil C51 and the Windows host harness. */
typedef unsigned short u16;
typedef unsigned long u32;

typedef enum {
    MOTION_STOP = 0,
    MOTION_FORWARD,
    MOTION_REVERSE,
    MOTION_LEFT,
    MOTION_RIGHT
} motion_t;

typedef enum {
    CAR_OK = 0,
    CAR_FAULT_UART,
    CAR_FAULT_QUEUE,
    CAR_FAULT_MAIN_STALL
} car_fault_t;

#endif
