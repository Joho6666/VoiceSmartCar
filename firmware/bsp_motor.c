#include "bsp_motor.h"
#include "board_config.h"

#if defined(VOICECAR_HARDWARE)
#error Hardware motor HAL is not implemented: add only measured L293D GPIO pins and verified output polarity.
#else
/* DryRun backend deliberately has no port writes and cannot drive a motor. */
void bsp_motor_init_safe(void) { }
void bsp_motor_refresh(void) { }
void bsp_motor_force_off(void) { }
#endif
