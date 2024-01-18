#include "interrupts.h"
#include "stdio.h"
#include "io.h"

typedef struct {
  uint16 offset_low;
  uint16 selector;
  uint8 zero;
  uint8 type_attr;
  uint16 offset_high;
} __attribute__((packed)) idt_entry_t;

idt_entry_t idt[256];

void interrupt empty_exception_handler(interrupt_frame_t* frame) {
  printf("Interrupt!\n");
  printf("ip: %x\n", frame->ip);
  printf("cs: %x\n", frame->cs);
  printf("flags: %x\n", frame->flags);
  printf("sp: %x\n", frame->sp);
  printf("ss: %x\n", frame->ss);
}


typedef struct {
  uint16 limit;
  uint32 base;
} __attribute__((packed)) idt_ptr_t;

enum interrupt_gate_type {
  INTERRUPT_GATE_TYPE_TASK = 0x5,
  INTERRUPT_GATE_TYPE_INTERRUPT = 0xE,
  INTERRUPT_GATE_TYPE_TRAP = 0xF,
};

void interrupts_set_handler(uint8 vector, interrupt_handler handler, enum interrupt_gate_type type) {
  uint32 handler_int = (uint32)handler;
  idt[vector].offset_low = handler_int & 0xFFFF;
  idt[vector].selector = 0x08;
  idt[vector].zero = 0;
  idt[vector].type_attr = 0x80 | type;
  idt[vector].offset_high = (handler_int >> 16) & 0xFFFF;
}

void interrupts_set_interrupt_handler(uint8 vector, interrupt_handler handler) {
  interrupts_set_handler(vector, handler, INTERRUPT_GATE_TYPE_INTERRUPT);
}

void interrupts_set_trap_handler(uint8 vector, interrupt_handler handler) {
  interrupts_set_handler(vector, handler, INTERRUPT_GATE_TYPE_TRAP);
}

static void print_interrupt_frame(interrupt_frame_t* frame) {
  printf("ip: %x\n", frame->ip);
  printf("cs: %x\n", frame->cs);
  printf("flags: %x\n", frame->flags);
  printf("sp: %x\n", frame->sp);
  printf("ss: %x\n", frame->ss);
}

void interrupt empty_interrupt_handler(interrupt_frame_t* frame) {
  printf("Interrupt!\n");
  print_interrupt_frame(frame);
}

void interrupt handle_divide_by_zero(interrupt_frame_t* frame) {
  printf("Divide by zero!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_debug(interrupt_frame_t* frame) {
  printf("Debug!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_nmi(interrupt_frame_t* frame) {
  printf("NMI!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_breakpoint(interrupt_frame_t* frame) {
  printf("Breakpoint!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_overflow(interrupt_frame_t* frame) {
  printf("Overflow!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_bound_range_exceeded(interrupt_frame_t* frame) {
  printf("Bound range exceeded!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_invalid_opcode(interrupt_frame_t* frame) {
  printf("Invalid opcode!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_device_not_available(interrupt_frame_t* frame) {
  printf("Device not available!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_double_fault(interrupt_frame_t* frame) {
  printf("Double fault!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_coprocessor_segment_overrun(interrupt_frame_t* frame) {
  printf("Coprocessor segment overrun!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_invalid_tss(interrupt_frame_t* frame) {
  printf("Invalid TSS!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_segment_not_present(interrupt_frame_t* frame) {
  printf("Segment not present!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_stack_segment_fault(interrupt_frame_t* frame) {
  printf("Stack segment fault!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_general_protection_fault(interrupt_frame_t* frame) {
  printf("General protection fault!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_page_fault(interrupt_frame_t* frame, uint32 error_code) {
  printf("Page fault!\n");
  print_interrupt_frame(frame);
  printf("error code: %x\n", error_code);
  asm volatile("hlt");
}

void interrupt handle_x87_floating_point_exception(interrupt_frame_t* frame) {
  printf("x87 floating point exception!\n");
  print_interrupt_frame(frame);
  asm volatile("hlt");
}

void interrupt handle_alignment_check(interrupt_frame_t* frame, uint32 error_code) {
  printf("Alignment check!\n");
  print_interrupt_frame(frame);
  printf("error code: %x\n", error_code);
  asm volatile("hlt");
}

void interrupt handle_machine_check(interrupt_frame_t* frame) {
  printf("Machine check!\n");
  print_interrupt_frame(frame);
}

void interrupt handle_simd_floating_point_exception(interrupt_frame_t* frame) {
  printf("SIMD floating point exception!\n");
  print_interrupt_frame(frame);
}

void interrupt handle_virtualization_exception(interrupt_frame_t* frame) {
  printf("Virtualization exception!\n");
  print_interrupt_frame(frame);
}

void interrupt handle_security_exception(interrupt_frame_t* frame) {
  printf("Security exception!\n");
  print_interrupt_frame(frame);
}

int watchdog_counter = 0;
int watchdog_enabled = 0;

void interrupt handle_timer(interrupt_frame_t* frame) {
  if (watchdog_enabled) {
    watchdog_counter++;
    if (watchdog_counter > 100) {
      printf("Watchdog!\n");
      print_interrupt_frame(frame);
      asm volatile("hlt");
    }
    if (watchdog_counter % 10 == 0) {
      printf(".");
    }
  }
  // Acknowledge the interrupt
  outb(0x20, 0x20);
}

void interrupts_enable_watchdog(void) {
  watchdog_enabled = 1;
}

void interrupts_disable_watchdog(void) {
  watchdog_enabled = 0;
}

void interrupts_watchdog_reset(void) {
  watchdog_counter = 0;
}

void interrupts_init(void) {
  // Set up the IDT pointer:
  idt_ptr_t idt_ptr;
  idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
  idt_ptr.base = (uint32)&idt;

  for (int i = 0; i < 256; i++) {
    interrupts_set_interrupt_handler(i, empty_interrupt_handler);
  }
  interrupts_set_trap_handler(0, handle_divide_by_zero);
  interrupts_set_trap_handler(1, handle_debug);
  interrupts_set_interrupt_handler(2, handle_nmi);
  interrupts_set_trap_handler(3, handle_breakpoint);
  interrupts_set_trap_handler(4, handle_overflow);
  interrupts_set_trap_handler(5, handle_bound_range_exceeded);
  interrupts_set_trap_handler(6, handle_invalid_opcode);
  interrupts_set_trap_handler(7, handle_device_not_available);
  interrupts_set_interrupt_handler(8, handle_double_fault);
  interrupts_set_trap_handler(9, handle_coprocessor_segment_overrun);
  interrupts_set_trap_handler(10, handle_invalid_tss);
  interrupts_set_trap_handler(11, handle_segment_not_present);
  interrupts_set_trap_handler(12, handle_stack_segment_fault);
  interrupts_set_trap_handler(13, handle_general_protection_fault);
  interrupts_set_trap_handler(14, (void*)handle_page_fault);
  interrupts_set_trap_handler(16, handle_x87_floating_point_exception);
  interrupts_set_trap_handler(17, (void*)handle_alignment_check);
  interrupts_set_interrupt_handler(18, (void*)handle_machine_check);
  interrupts_set_trap_handler(19, handle_simd_floating_point_exception);
  interrupts_set_trap_handler(20, handle_virtualization_exception);
  interrupts_set_trap_handler(30, handle_security_exception);
  // Setup the PIT timer to interrupt every 10ms
  outb(0x43, 0x36);
  int divisor = 1193180 / 100;
  outb(0x40, divisor & 0xFF);
  outb(0x40, (divisor >> 8) & 0xFF);

  interrupts_set_interrupt_handler(32, handle_timer);
  // Points the processor's internal register to the new IDT
  // Initialize the PIC
  outb(0x20, 0x11); //begin initialization
  outb(0xA0, 0x11);
  outb(0x21, 0x20); // master offset
  outb(0xA1, 0x28); // slave offset
  outb(0x21, 0x04); // master/slave wiring
  outb(0xA1, 0x02);
  outb(0x21, 0x01); //8088
  outb(0xA1, 0x01);
  asm volatile("lidt %0" : : "m"(idt_ptr));
}

void interrupts_clear_handler(uint8 vector) {
  interrupts_set_interrupt_handler(vector, empty_interrupt_handler);
}

void interrupts_enable(void) {
  asm volatile("sti");
}

void interrupts_disable(void) {
  asm volatile("cli");
}
