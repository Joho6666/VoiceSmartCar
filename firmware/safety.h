#ifndef SAFETY_H
#define SAFETY_H
#include "car_types.h"

void safety_uart_error(void);
void safety_overrun(void);
void safety_stop_received(void);
u8 safety_fault_active(void);

#endif
