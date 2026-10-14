#ifndef VOICE_PROTOCOL_H
#define VOICE_PROTOCOL_H
#include "car_types.h"

typedef enum {
    VOICE_INVALID = 0,
    VOICE_FORWARD,
    VOICE_REVERSE,
    VOICE_STOP,
    VOICE_FASTER,
    VOICE_SLOWER,
    VOICE_LEFT,
    VOICE_RIGHT,
    VOICE_SEQUENCE_1,
    VOICE_SEQUENCE_2
} voice_command_t;

voice_command_t voice_decode(u8 byte);
u8 voice_is_valid(u8 byte);
u8 voice_is_stop(u8 byte);

#endif
