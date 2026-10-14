#ifndef ACTION_SEQUENCE_H
#define ACTION_SEQUENCE_H
#include "car_types.h"

void action_init(void);
void action_start(u8 sequence, u16 now_ms);
void action_tick(u16 now_ms);
void action_abort(void);
u8 action_active(void);
u8 action_stage(void);

#endif
