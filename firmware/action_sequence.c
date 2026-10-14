#include "action_sequence.h"
#include "motion_controller.h"

#define MOTION_STOP 0
#define MOTION_FORWARD 1
#define MOTION_REVERSE 2
#define MOTION_LEFT 3
#define MOTION_RIGHT 4

static const u16 s_duration[2][3] = {{1000, 500, 0}, {700, 500, 0}};
static const u8 s_motion[2][3] = {
    {MOTION_FORWARD, MOTION_LEFT, MOTION_STOP},
    {MOTION_REVERSE, MOTION_RIGHT, MOTION_STOP}
};
static u8 s_sequence;
static u8 s_stage;
static u8 s_active;
static u16 s_stage_end;

void action_init(void)
{
    s_sequence = 0;
    s_stage = 0;
    s_active = 0;
    s_stage_end = 0;
}

void action_abort(void)
{
    s_active = 0;
    s_stage = 0;
}

void action_start(u8 sequence, u16 now_ms)
{
    action_abort();
    if (sequence != 1 && sequence != 2) return;
    s_sequence = (u8)(sequence - 1);
    s_stage = 0;
    s_active = 1;
    s_stage_end = (u16)(now_ms + s_duration[s_sequence][0]);
    if (s_motion[s_sequence][0] == MOTION_FORWARD)
        motion_command(0xA1, now_ms);
    else
        motion_command(0xA2, now_ms);
}

void action_tick(u16 now_ms)
{
    u8 next;
    if (!s_active || (u16)(now_ms - s_stage_end) >= 0x8000U) return;
    next = (u8)(s_stage + 1);
    if (next >= 2) {
        motion_auto_stop();
        action_abort();
        return;
    }
    s_stage = next;
    if (s_motion[s_sequence][s_stage] == MOTION_LEFT)
        motion_command(0xA6, now_ms);
    else if (s_motion[s_sequence][s_stage] == MOTION_RIGHT)
        motion_command(0xA7, now_ms);
    s_stage_end = (u16)(now_ms + s_duration[s_sequence][s_stage]);
    if (motion_get_state() == MOTION_STOP && motion_has_pending_motion())
        s_stage_end = (u16)(s_stage_end + 100U);
}

u8 action_active(void) { return s_active; }
u8 action_stage(void) { return s_stage; }
