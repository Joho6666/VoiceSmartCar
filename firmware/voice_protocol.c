#include "voice_protocol.h"

voice_command_t voice_decode(u8 byte)
{
    switch (byte) {
    case 0xA1: return VOICE_FORWARD;
    case 0xA2: return VOICE_REVERSE;
    case 0xA3: return VOICE_STOP;
    case 0xA4: return VOICE_FASTER;
    case 0xA5: return VOICE_SLOWER;
    case 0xA6: return VOICE_LEFT;
    case 0xA7: return VOICE_RIGHT;
    case 0xB1: return VOICE_SEQUENCE_1;
    case 0xB2: return VOICE_SEQUENCE_2;
    default: return VOICE_INVALID;
    }
}

u8 voice_is_valid(u8 byte) { return (voice_decode(byte) != VOICE_INVALID) ? 1 : 0; }
u8 voice_is_stop(u8 byte) { return (byte == 0xA3) ? 1 : 0; }
