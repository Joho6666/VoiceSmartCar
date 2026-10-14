#include "safety.h"
#include "motion_controller.h"

void safety_uart_error(void) { motion_stop_immediate(CAR_FAULT_UART); }
void safety_overrun(void) { motion_stop_immediate(CAR_FAULT_QUEUE); }
void safety_stop_received(void) { motion_request_emergency_stop(); }
u8 safety_fault_active(void) { return motion_get_fault(); }
