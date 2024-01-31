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
  u16 es;
  u16 ds;
  union {
    u32 edi;
    u16 di;
  };
  union {
    u32 esi;
    u16 si;
  };
  union {
    u32 edx;
    union {
      u16 dx;
      struct {
        u8 dl;
        u8 dh;
      } packed;
    };
  };
  union {
    u32 ecx;
    union {
      u16 cx;
      struct {
        u8 cl;
        u8 ch;
      } packed;
    };
  };
  union {
    u32 ebx;
    union {
      u16 bx;
      struct {
        u8 bl;
        u8 bh;
      } packed;
    };
  };
  union {
    u32 eax;
    union {
      u16 ax;
      struct {
        u8 al;
        u8 ah;
      } packed;
    };
  };
  u32 esp;
  u32 ebp;
  u16 ss;
  u16 gs;
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
