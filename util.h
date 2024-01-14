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
  byte interrupt;
  word ax, bx, cx, dx, si;
} real_mode_int_params;

void real_mode_int(real_mode_int_params* params);
#endif
