#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include "types.h"

void itr_init(void);
void itr_enable(void);
void itr_disable(void);

typedef struct {
  // TODO add full state
  u32 ip;
  u32 cs;
  u32 flags;
  u32 sp;
  u32 ss;
} packed itr_frame;

enum itr_irq {
  IRQ0 = 0,
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


typedef void (*itr_handler_irq)(enum itr_irq irq, void* data);

void itr_set_irq_handler(enum itr_irq irq, itr_handler_irq handler, void *data);
void itr_clear_irq_handler(enum itr_irq irq);

typedef void (*itr_handler)(u8 vector, itr_frame *frame, void* data);

void itr_set_handler(u8 vector, itr_handler handler, void *data);
void itr_clear_handler(u8 vector);

typedef struct {
  u16 fs;
  u16 gs;
  u16 ss;
  u32 esp;
  u16 es;
  u16 ds;
  u32 ebp;
  u32 edi;
  u32 esi;
  union {
    u32 edx;
    u16 dx;
  };
  union {
    u32 ecx;
    u16 cx;
  };
  union {
    u32 ebx;
    u16 bx;
  };
  union {
    u32 eax;
    u16 ax;
  };
  u8 idt[6];
  u8 gdt[6];
  u16 ip;
  u16 cs;
  u16 flags;
} packed itr_frame_real_mode;

typedef void (*itr_handler_real_mode)(u8 vector, itr_frame_real_mode *frame, void* data);
void itr_set_real_mode_handler(u8 vector, itr_handler_real_mode handler,
                               void *data);
void itr_setup_real_mode(void);
bool itr_enabled(void);
void itr_reload_idt(void);
#endif // __INTERRUPTS_H__
