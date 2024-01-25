#ifndef UTIL_H
#define UTIL_H
#include "types.h"
#include "interrupt.h"
#include "timer.h"
#include "dev.h"

typedef struct {
  u16 segment;
  u16 offset;
  u16 ax, bx, cx, dx, si, di;
} real_mode_call_params;

void real_mode_call(real_mode_call_params* params);

typedef struct {
  u8 int_num;
  u16 ax, bx, cx, dx, si;
} real_mode_int_params;

static void memcpy(void* dest, void* src, u32 len) {
  if ((len & 3) == 0) {
    u32* d = (u32*)dest;
    u32* s = (u32*)src;
    while (len > 0) {
      *d++ = *s++;
      len -= 4;
    }
  } else if ((len & 2) == 0) {
    u16* d = (u16*)dest;
    u16* s = (u16*)src;
    while (len > 0) {
      *d++ = *s++;
      len -= 2;
    }
  } else {
    u8* d = (u8*)dest;
    u8* s = (u8*)src;
    while (len > 0) {
      *d++ = *s++;
      len--;
    }
  }
}

static void* shadowed_call(void* func) {
  return func - 0xFFF00000;
}

void real_mode_int(real_mode_int_params* params);

void soft_reset();

#define assert(x) if (!(x)) { \
  serial_set_buffered(false); \
  printf("Assertion failed: %s\n", #x); \
  asm("ud2"); \
}

static void msleep(u32 ms) {
  unsigned int start = timer_get_ticks(dev_timer_primary);
  while ((timer_get_ticks(dev_timer_primary) - start) < ms) {
    asm("hlt");
  }
}

#endif
