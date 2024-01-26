#include "keyboard.h"
#include "interrupt.h"
#include "io.h"
#include "output.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_CMD_PORT 0x64
#define KEYBOARD_IRQ IRQ1
#define KEYBOARD_BUFFER_SIZE 128

#define OUTPUT_BUFFER_FULL 0x01
#define INPUT_BUFFER_FULL 0x02
#define SYSTEM_FLAG 0x04
#define COMMAND_DATA 0x08
#define KEYBOARD_LOCKED 0x10
#define AUXILIARY_OUTPUT_BUFFER_FULL 0x20
#define TIMEOUT_ERROR 0x40
#define PARITY_ERROR 0x80

static struct keyboard_buffer {
  u8 buffer[KEYBOARD_BUFFER_SIZE];
  u8 head;
  u8 tail;
} keyboard_buffer;

u8 keyboard_press_state = 0;
u8 keyboard_extended_press_state = 0;

u8 keyboard_read_data() {
  while ((inb(KEYBOARD_STATUS_PORT) & OUTPUT_BUFFER_FULL) == 0) {
    printf("Reading keybboard status: %x data %x\n", inb(KEYBOARD_STATUS_PORT), inb(KEYBOARD_DATA_PORT));
    //keyboard_cmd(0x60);
    //keyboard_send_data(0x00);
  }
  return inb(KEYBOARD_DATA_PORT);
}

void keyboard_irq_handler(enum itr_irq unused irq, void* unused _) {
    u8 scancode = keyboard_read_data();
    printf("Got scancode from interrupt: %x\n", scancode);
    if (scancode & 0x80) {
      switch (scancode & 0x7F) {
        // TODO setup keyboard press state
      }
    } else {
      switch (scancode) {
        // TODO setup keyboard press state
        default:
          keyboard_buffer.buffer[keyboard_buffer.head] = scancode;
          keyboard_buffer.head = (keyboard_buffer.head + 1) % KEYBOARD_BUFFER_SIZE;
      }
    }
};

void keyboard_cmd(u8 cmd) {
  outb(KEYBOARD_CMD_PORT, cmd);
  while ((inb(KEYBOARD_STATUS_PORT) & COMMAND_DATA) == 0);
}

void keyboard_send_data(u8 data) {
  while ((inb(KEYBOARD_STATUS_PORT) & INPUT_BUFFER_FULL) != 0);
  outb(KEYBOARD_DATA_PORT, data);
}

u8 keyboard_read_output_port() {
  keyboard_cmd(0xD0);
  return keyboard_read_data();
}

void keyboard_write_output_port(u8 data) {
  keyboard_cmd(0xD1);
  keyboard_send_data(data);
}

u8 keyboard_reset() {
  keyboard_send_data(0xFF);
  return keyboard_read_data();
}

bool keyboard_init() {
  printf("Keyboard init\n");
  // Disable devices
  keyboard_cmd(0xAD);
  //keyboard_cmd(0xA7);
  printf("Test keyboard controller\n");
  // Test controller
  keyboard_cmd(0xAA);
  u8 status = keyboard_read_data();
  if (status != 0x55) {
    printf("Keyboard controller test failed\n");
    return false;
  }
  // Enable interrupts
  keyboard_cmd(0x20);
  u8 config = keyboard_read_data();
  config |= 0x01;
  keyboard_cmd(0x60);
  printf("Config: %x\n", config);
  keyboard_send_data(config);
  printf("Enable keyboard controller\n");
  // Enable devices
  keyboard_cmd(0xAE);
  //keyboard_cmd(0xA8);
  printf("Reset keyboard\n");
  // Reset devices
  bool reset = false;
  for (int i = 0; i < 10; i++) {
    u8 status = keyboard_reset();
    if (status == 0xFA) {
      reset = true;
      break;
    }
  }
  if (!reset) {
    printf("Keyboard reset failed\n");
    return false;
  }
  printf("Keyboard interrupts enabled\n");
  // Enable keyboard interrupts
  itr_set_irq_handler(KEYBOARD_IRQ, keyboard_irq_handler, NULL);
  // Enable keyboard

  return true;
}

bool keyboard_available() {
    return keyboard_buffer.head != keyboard_buffer.tail || (inb(KEYBOARD_STATUS_PORT) & OUTPUT_BUFFER_FULL) != 0;
}

u8 keyboard_get_scancode() {
  if (itr_enabled()) {
    while (!keyboard_available()) {
      asm volatile("hlt");
    }
    u8 scancode = keyboard_buffer.buffer[keyboard_buffer.tail];
    return scancode;
  } else {
    if (keyboard_buffer.head != keyboard_buffer.tail) {
      u8 scancode = keyboard_buffer.buffer[keyboard_buffer.tail];
      return scancode;
    }
    if ((inb(KEYBOARD_STATUS_PORT) & OUTPUT_BUFFER_FULL) == 0) {
      return 0;
    }
    u8 scancode = inb(KEYBOARD_DATA_PORT);
    u8 ascii = keyboard_map(scancode);
    printf("Got scancode from poll: %x, '%x'\n", scancode, ascii);
    // Put scancode in buffer
    keyboard_buffer.buffer[keyboard_buffer.head] = scancode;
    keyboard_buffer.head = (keyboard_buffer.head + 1) % KEYBOARD_BUFFER_SIZE;
    return scancode;
  }
}

u8 keyboard_pop_scancode() {
  u8 scancode = keyboard_get_scancode();
  keyboard_buffer.tail = (keyboard_buffer.tail + 1) % KEYBOARD_BUFFER_SIZE;
  return scancode;
}

u8 keyboard_map(u8 scancode) {
  if (scancode < 0x80) {
    return kb_scancodes_set1_noshift_map[scancode];
  } else {
    return 0;
  }
}

