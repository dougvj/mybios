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

enum {
  IRQ0 = 32,
  IRQ1,
  IRQ2,
  IRQ3,
  IRQ4,
  IRQ5,
  IRQ6,
  IRQ7,
  IRQ8,
  IRQ9,
  IRQ10,
  IRQ11,
  IRQ12,
  IRQ13,
  IRQ14,
  IRQ15
};

typedef void(*interrupt_handler)(interrupt_frame_t* frame);

void interrupts_set_interrupt_handler(uint8 vector, interrupt_handler handler);
void interrupts_set_trap_handler(uint8 vector, interrupt_handler handler);
void interrupts_clear_handler(uint8 vector);

void interrupts_enable_watchdog(void);
void interrupts_disable_watchdog(void);
void interrupts_watchdog_reset(void);
unsigned int  interrupts_timer_ticks(void);

typedef unsigned int(*timer_callback)(unsigned int ticks);
int interrupts_register_timer_callback(timer_callback callback, unsigned int ticks);
int interrupts_unregister_timer_callback(int id);
bool interrupts_enabled(void);
#endif // __INTERRUPTS_H__
