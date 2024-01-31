#include "keyboard.h"
#include "dev.h"
#include "interrupt.h"
#include "io.h"
#include "output.h"
#include "bda.h"

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

#define KB_STATE_LEFT_SHIFT 0x02
#define KB_STATE_RIGHT_SHIFT 0x01
#define KB_STATE_CTRL 0x04
#define KB_STATE_ALT 0x08
#define KB_STATE_SCROLL_LOCK 0x10
#define KB_STATE_NUM_LOCK 0x20
#define KB_STATE_CAPS_LOCK 0x40
#define KB_STATE_INSERT 0x80

#define KB_STATE_LEFT_CTRL 0x01
#define KB_STATE_LEFT_ALT 0x02
#define KB_STATE_RIGHT_CONTROL 0x04
#define KB_STATE_RIGHT_ALT 0x08
#define KB_STATE_SCROLL_LOCK_PRESSED 0x10
#define KB_STATE_NUM_LOCK_PRESSED 0x20
#define KB_STATE_CAPS_LOCK_PRESSED 0x40
#define KB_STATE_SYS_REQ_PRESSED 0x80


#define KB_STATE_LEFT_SHIFT 0x02
#define KB_STATE_RIGHT_SHIFT 0x01
#define KB_STATE_CTRL 0x04
#define KB_STATE_ALT 0x08
#define KB_STATE_SCROLL_LOCK 0x10
#define KB_STATE_NUM_LOCK 0x20
#define KB_STATE_CAPS_LOCK 0x40
#define KB_STATE_INSERT 0x80

#define KB_STATE_LEFT_CTRL 0x01
#define KB_STATE_LEFT_ALT 0x02
#define KB_STATE_RIGHT_CONTROL 0x04
#define KB_STATE_RIGHT_ALT 0x08
#define KB_STATE_SCROLL_LOCK_PRESSED 0x10
#define KB_STATE_NUM_LOCK_PRESSED 0x20
#define KB_STATE_CAPS_LOCK_PRESSED 0x40
#define KB_STATE_SYS_REQ_PRESSED 0x80

u8 keyboard_read_data() {
  u32 time = timer_get_ticks(dev_timer_primary);
  u32 timeout = time + 18;
  while ((inb(KEYBOARD_STATUS_PORT) & OUTPUT_BUFFER_FULL) == 0) {
    if (timer_get_ticks(dev_timer_primary) > timeout) {
      printf("Keyboard read timeout\n");
      return 0;
    }
    // printf("Reading keybboard status: %x data %x\n",
    // inb(KEYBOARD_STATUS_PORT), inb(KEYBOARD_DATA_PORT));
    // controller_cmd(0x60);
    // keyboard_send_data(0x00);
  }
  return inb(KEYBOARD_DATA_PORT);
}

u8 keyboard_transact(u8 data);

void update_leds() {
  u8 leds = 0;
  if (bda.keyboard_flags_xt & KB_STATE_SCROLL_LOCK) {
    leds |= 0x01;
  }
  if (bda.keyboard_flags_xt & KB_STATE_NUM_LOCK) {
    leds |= 0x02;
  }
  if (bda.keyboard_flags_xt & KB_STATE_CAPS_LOCK) {
    leds |= 0x04;
  }
  keyboard_transact(0xED);
  keyboard_transact(leds);
}

u8 keyboard_process_keystroke() {
  u8 last_kb_flags = bda.keyboard_flags_xt;
  u8 last_xt_flags = bda.keyboard_flags_at;
  u8 scancode = keyboard_read_data();
  if (scancode & 0x80) {
    switch (scancode & 0x7F) {
    case KB1_SC_LEFT_SHIFT:
      bda.keyboard_flags_xt &= ~KB_STATE_LEFT_SHIFT;
      break;
    case KB1_SC_RIGHT_SHIFT:
      bda.keyboard_flags_xt &= ~KB_STATE_RIGHT_SHIFT;
      break;
    case KB1_SC_LEFT_CTRL:
      bda.keyboard_flags_xt &= ~KB_STATE_CTRL;
      bda.keyboard_flags_at &= ~KB_STATE_LEFT_CTRL;
      break;
    case KB1_SC_LEFT_ALT:
      bda.keyboard_flags_xt &= ~KB_STATE_ALT;
      bda.keyboard_flags_at &= ~KB_STATE_LEFT_ALT;
      break;
    case KB1_SC_SCROLLLOCK:
      bda.keyboard_flags_at &= ~KB_STATE_SCROLL_LOCK_PRESSED;
      break;
    case KB1_SC_NUMLOCK:
      bda.keyboard_flags_at &= ~KB_STATE_NUM_LOCK_PRESSED;
      break;
    case KB1_SC_CAPSLOCK:
      bda.keyboard_flags_at &= ~KB_STATE_CAPS_LOCK_PRESSED;
      break;
    case KB1_SC_INSERT:
      bda.keyboard_flags_xt &= ~KB_STATE_INSERT;
      break;
    }
  } else {
    switch (scancode) {
    case KB1_SC_LEFT_SHIFT:
      bda.keyboard_flags_xt |= KB_STATE_LEFT_SHIFT;
      break;
    case KB1_SC_RIGHT_SHIFT:
      bda.keyboard_flags_xt |= KB_STATE_RIGHT_SHIFT;
      break;
    case KB1_SC_LEFT_CTRL:
      bda.keyboard_flags_xt |= KB_STATE_CTRL;
      bda.keyboard_flags_at |= KB_STATE_LEFT_CTRL;
      break;
    case KB1_SC_LEFT_ALT:
      bda.keyboard_flags_xt |= KB_STATE_ALT;
      bda.keyboard_flags_at |= KB_STATE_LEFT_ALT;
      break;
    case KB1_SC_SCROLLLOCK:
      bda.keyboard_flags_xt ^= KB_STATE_SCROLL_LOCK;
      bda.keyboard_flags_at |= KB_STATE_SCROLL_LOCK_PRESSED;
      update_leds();
      break;
    case KB1_SC_NUMLOCK:
      bda.keyboard_flags_xt ^= KB_STATE_NUM_LOCK;
      bda.keyboard_flags_at |= KB_STATE_NUM_LOCK_PRESSED;
      update_leds();
      break;
    case KB1_SC_CAPSLOCK:
      bda.keyboard_flags_xt ^= KB_STATE_CAPS_LOCK;
      bda.keyboard_flags_at |= KB_STATE_CAPS_LOCK_PRESSED;
      update_leds();
      break;
    case KB1_SC_INSERT:
      bda.keyboard_flags_xt ^= KB_STATE_INSERT;
      break;
    }
  }
  if (last_kb_flags != bda.keyboard_flags_xt) {
    printf("Keyboard flags: %x\n", bda.keyboard_flags_xt);
  }
  if (last_xt_flags != bda.keyboard_flags_at) {
    printf("Keyboard flags: %x\n", bda.keyboard_flags_at);
  }
  if (scancode < 0x80) {
    bda.keyboard_buffer[bda.keyboard_buffer_head] = scancode;
    bda.keyboard_buffer_head = (bda.keyboard_buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
  }
  return scancode;
}

void keyboard_irq_handler(enum itr_irq unused irq, void *unused _) {
  u8 scancode = keyboard_process_keystroke();
  printf("Got scancode from interrupt: %x\n", scancode);
};

void controller_cmd(u8 cmd) {
  outb(KEYBOARD_CMD_PORT, cmd);
  while ((inb(KEYBOARD_STATUS_PORT) & COMMAND_DATA) == 0)
    ;
}

void keyboard_send_data(u8 data) {
  int start_ticks = timer_get_ticks(dev_timer_primary);
  while ((inb(KEYBOARD_STATUS_PORT) & INPUT_BUFFER_FULL) != 0) {
    printf("Ticks: %d\n", timer_get_ticks(dev_timer_primary));
    if (timer_get_ticks(dev_timer_primary) - start_ticks > 100) {
      printf("Keyboard send data timeout. Keyboard error?\n");
      return;
    }
  };
  outb(KEYBOARD_DATA_PORT, data);
}

u8 keyboard_read_output_port() {
  controller_cmd(0xD0);
  return keyboard_read_data();
}

void keyboard_write_output_port(u8 data) {
  controller_cmd(0xD1);
  keyboard_send_data(data);
}

void controller_config_write(u8 data) {
  controller_cmd(0x60);
  keyboard_send_data(data);
}

u8 controller_config_read() {
  controller_cmd(0x20);
  return keyboard_read_data();
}

u8 keyboard_transact(u8 data) {
  printf("Keyboard transact: ->%x\n", data);
  int timer_ticks = timer_get_ticks(dev_timer_primary);
  do {
    keyboard_send_data(data);
    data = keyboard_read_data();
    printf("Keyboard transact: <-%x\n", data);
    if (timer_get_ticks(dev_timer_primary) - timer_ticks > 18) {
      printf("Keyboard transact timeout\n");
      return 0;
    }
  } while (data != 0xFA);
  printf("Keyboard transact: <-%x\n", data);
  return data;
}

u8 keyboard_reset() {
  keyboard_transact(0xFF);
  u8 result = keyboard_read_data();
  return result;
}

bool keyboard_init() {
  printf("Keyboard init\n");
  // Disable devices
  controller_cmd(0xAD);
  // controller_cmd(0xA7);
  printf("Test keyboard controller\n");
  // Test controller
  controller_cmd(0xAA);
  u8 status = keyboard_read_data();
  if (status != 0x55) {
    printf("Keyboard controller test failed\n");
    return false;
  }
  // Flash LEDs
  keyboard_transact(0xED);
  keyboard_transact(0x07);
  u8 config = controller_config_read();
  // Enable interrupts
  config |= 0x01;
  printf("Config: %x\n", config);
  controller_config_write(config);
  printf("Enable keyboard controller\n");
  // Enable devices
  controller_cmd(0xAE);
  // controller_cmd(0xA8);
  printf("Reset keyboard\n");
  // Reset devices
  bool reset = false;
  for (int i = 0; i < 10; i++) {
    u8 result = keyboard_reset();
    if (result == 0xAA) {
      reset = true;
      break;
    }
  }
  if (!reset) {
    printf("Keyboard reset failed\n");
    return false;
  }
  // Set scan code set 1
  keyboard_transact(0xF0);
  keyboard_transact(0x01);
  // See if the keyboard set the scan code set
  keyboard_transact(0xF0);
  keyboard_transact(0x00);
  u8 set = keyboard_read_data();
  printf("Keyboard set scan code set: %x %x\n", set);
  if (set != 0x1) {
    printf("Keyboard set scan code set failed, switching to 2 and enabling translation\n");
    keyboard_transact(0xF0);
    keyboard_transact(0x02);
    controller_cmd(0x20);
    status = keyboard_read_data();
    status |= 0x40;
    controller_cmd(0x60);
    keyboard_send_data(status);
  } else {
    printf("Keyboard set scan code set succeeded, disabling translation\n");
    controller_cmd(0x20);
    status = keyboard_read_data();
    status &= ~0x40;
    controller_cmd(0x60);
    keyboard_send_data(status);
  }
  printf("Keyboard interrupts enabled\n");
  //  Enable keyboard interrupts
  itr_set_irq_handler(KEYBOARD_IRQ, keyboard_irq_handler, NULL);
  // Unflash LEDs
  keyboard_transact(0xED);
  keyboard_transact(0x00);
  bda.keyboard_buffer_head = 0;
  bda.keyboard_buffer_tail = 0;
  bda.keyboard_flags_xt = 0;
  bda.keyboard_flags_at = 0;
  return true;
}

bool keyboard_available() {
  return bda.keyboard_buffer_head != bda.keyboard_buffer_tail ||
         (inb(KEYBOARD_STATUS_PORT) & OUTPUT_BUFFER_FULL) != 0;
}

u8 keyboard_get_scancode() {
  if (itr_enabled()) {
    while (!keyboard_available()) {
      asm volatile("hlt");
    }
    u8 scancode = bda.keyboard_buffer[bda.keyboard_buffer_tail];
    return scancode;
  } else {
    if (bda.keyboard_buffer_head != bda.keyboard_buffer_tail) {
      u8 scancode = bda.keyboard_buffer[bda.keyboard_buffer_tail];
      return scancode;
    }
    if (!keyboard_available()) {
      return 0;
    }
    u8 scancode = keyboard_process_keystroke();
    printf("Got scancode from polling: %x\n", scancode);
    return keyboard_get_scancode();
  }
}

u8 keyboard_pop_scancode() {
  u8 scancode = keyboard_get_scancode();
  bda.keyboard_buffer_tail = (bda.keyboard_buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
  return scancode;
}

u8 keyboard_map(u8 scancode) {
  if (scancode < 0x80) {
    if ((bda.keyboard_flags_xt & (KB_STATE_LEFT_SHIFT | KB_STATE_RIGHT_SHIFT)) ^
        (bda.keyboard_flags_xt & KB_STATE_CAPS_LOCK)) {
      return kb_scancodes_set1_shift_map[scancode];
    } else {
      return kb_scancodes_set1_noshift_map[scancode];
    }
  } else {
    return 0;
  }
}
