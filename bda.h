#ifndef BDA_H
#define BDA_H
#include "types.h"

struct bda {
  u16 com1_io;   // 0x400
  u16 com2_io;   // 0x402
  u16 com3_io;   // 0x404
  u16 com4_io;   // 0x406
  u16 lpt1_io;   // 0x408
  u16 lpt2_io;   // 0x40a
  u16 lpt3_io;   // 0x40c
  u16 ebda_base; // 0x40e
  u16 equipment; // 0x410
  u8 _pad1;
  u16 memory_kb; // 0x413
  u8 _pad2[2];
  u8 keyboard_flags_xt;
  u8 keyboard_flags_at;
  u8 _pad3;
  u16 keyboard_buffer_head_offset;
  u16 keyboard_buffer_tail_offset;
  u8 keyboard_buffer[32];
  u8 _pad4[11];
  u8 display_mode;
  u16 display_columns;
  u8 _pad5[23];
  u16 display_io;
  u8 _pad6[7];
  u32 irq0_counter;
  u8 flag_24hr;
  u8 bios_break_key_flag;
  u16 soft_reset_flag;
  u8 last_hdd_op;
  u8 num_hdd; // Left off here
  u8 _pad7[10];
  u16 keyboard_buffer_head;
  u16 keyboard_buffer_tail;
  u8 _pad8[19];
  u8 last_keyboard;
} packed;

#define offset_of(type, member) ((u32)(&((type *)0)->member))

#define IRQ0_COUNTER_OFFSET offset_of(struct bda, irq0_counter)

#define CHECK_OFFSET(field, offset)                                            \
  static_assert(!(offset_of(struct bda, field) + 0x400 > offset),              \
                "field offset is too large");                                  \
  static_assert(!(offset_of(struct bda, field) + 0x400 < offset),              \
                "field offset is too small");

// COnfirm that when we fiddle with the struct addding/removing fields, we don't
// break the offsets
//
// Got this from https://stanislavs.org/helppc/bios_data_area.html
CHECK_OFFSET(com1_io, 0x400);
CHECK_OFFSET(com2_io, 0x402);
CHECK_OFFSET(com3_io, 0x404);
CHECK_OFFSET(com4_io, 0x406);
CHECK_OFFSET(lpt1_io, 0x408);
CHECK_OFFSET(lpt2_io, 0x40a);
CHECK_OFFSET(lpt3_io, 0x40c);
CHECK_OFFSET(ebda_base, 0x40e);
CHECK_OFFSET(equipment, 0x410);
CHECK_OFFSET(memory_kb, 0x413);
CHECK_OFFSET(keyboard_flags_xt, 0x417);
CHECK_OFFSET(keyboard_flags_at, 0x418);
CHECK_OFFSET(keyboard_buffer_head_offset, 0x41a);
CHECK_OFFSET(keyboard_buffer_tail_offset, 0x41c);
CHECK_OFFSET(keyboard_buffer, 0x41e);
CHECK_OFFSET(display_mode, 0x449);
CHECK_OFFSET(display_columns, 0x44a);
CHECK_OFFSET(display_io, 0x463);
CHECK_OFFSET(irq0_counter, 0x46c);
CHECK_OFFSET(flag_24hr, 0x470);
CHECK_OFFSET(bios_break_key_flag, 0x471);
CHECK_OFFSET(soft_reset_flag, 0x472);
CHECK_OFFSET(last_hdd_op, 0x474);
CHECK_OFFSET(num_hdd, 0x475);
CHECK_OFFSET(keyboard_buffer_head, 0x480);
CHECK_OFFSET(keyboard_buffer_tail, 0x482);
CHECK_OFFSET(last_keyboard, 0x497);


// The section designator tells the linker to put this at 0x400 in RAM. Neat!
extern struct bda bda section(".bda");

#endif
