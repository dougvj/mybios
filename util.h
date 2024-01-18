#ifndef UTIL_H
#define UTIL_H
#include "types.h"

typedef struct {
  word segment;
  word offset;
  word ax, bx, cx, dx, si, di;
} real_mode_call_params;

void real_mode_call(real_mode_call_params* params);

typedef struct {
  byte int_num;
  word ax, bx, cx, dx, si;
} real_mode_int_params;

static void memcpy(void* dest, void* src, dword len) {
  if ((len & 3) == 0) {
    dword* d = (dword*)dest;
    dword* s = (dword*)src;
    while (len > 0) {
      *d++ = *s++;
      len -= 4;
    }
  } else if ((len & 2) == 0) {
    word* d = (word*)dest;
    word* s = (word*)src;
    while (len > 0) {
      *d++ = *s++;
      len -= 2;
    }
  } else {
    byte* d = (byte*)dest;
    byte* s = (byte*)src;
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
#endif
