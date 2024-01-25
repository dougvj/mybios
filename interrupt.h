#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include "types.h"

void itr_init(void);
void itr_enable(void);
void itr_disable(void);

typedef struct {
  u32 ip;
  u32 cs;
  u32 flags;
  u32 sp;
  u32 ss;
} packed itr_frame;

enum itr_number_vectors {
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

typedef void (*itr_handler)(u8 vector, itr_frame *frame, void* data);

void itr_set_handler(u8 vector, itr_handler handler, void *data);
void itr_clear_handler(u8 vector);

typedef struct {
  u16 ss;
  u16 sp;
  u16 es;
  u16 ds;
  u16 bp;
  u16 di;
  u16 si;
  u16 dx;
  u16 cx;
  u16 bx;
  u16 ax;
  u16 ip;
  u16 cs;
  u16 flags;
} packed itr_frame_real_mode;

typedef void (*itr_handler_real_mode)(u8 vector, itr_frame_real_mode *frame, void* data);
void itr_set_real_mode_handler(u8 vector, itr_handler_real_mode handler,
                               void *data);
bool itr_enabled(void);
void itr_reload_idt(void);
#endif // __INTERRUPTS_H__
