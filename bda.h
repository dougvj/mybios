#ifndef BDA_H
#define BDA_H
#include "types.h"

struct bda {
  u16 com1_io;// 0x400
  u16 com2_io;// 0x402
  u16 com3_io;// 0x404
  u16 com4_io;// 0x406
  u16 lpt1_io;// 0x408
  u16 lpt2_io;// 0x40a
  u16 lpt3_io;// 0x40c
  u16 ebda_base;// 0x40e
  u16 equipment;// 0x410
  u16 memory_kb;// 0x413
  // TODO more padding
  u16 keyboard_flags;
  u8 keyboard_buffer[32];
  u8 display_mode;
  u16 display_columns;
  u16 display_io;
  u16 irq0_counter;
  u8 num_hdd;
  u16 kb_buffer_start;
  u16 kb_buffer_end;
  u8 last_keyboard;
} packed;

extern struct bda bda section(".bda");

#endif
