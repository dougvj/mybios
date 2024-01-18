#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include "types.h"

#define interrupt __attribute__((interrupt))
void interrupts_init(void);
void interrupts_enable(void);
void interrupts_disable(void);


typedef struct {
    uint32 ip;
    uint32 cs;
    uint32 flags;
    uint32 sp;
    uint32 ss;
} interrupt_frame_t;


typedef void(*interrupt_handler)(interrupt_frame_t* frame);

void interrupts_set_interrupt_handler(uint8 vector, interrupt_handler handler);
void interrupts_set_trap_handler(uint8 vector, interrupt_handler handler);
void interrupts_clear_handler(uint8 vector);

void interrupts_enable_watchdog(void);
void interrupts_disable_watchdog(void);
void interrupts_watchdog_reset(void);

#endif // __INTERRUPTS_H__
