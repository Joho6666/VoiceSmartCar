#ifndef BSP_MOTOR_H
#define BSP_MOTOR_H

/* Hardware backend remains intentionally disabled until PCB measurements exist. */
void bsp_motor_init_safe(void);
void bsp_motor_refresh(void);
void bsp_motor_force_off(void);

#endif
