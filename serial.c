#include "serial.h"
#include "interrupt.h"
#include "util.h"
#include "output.h"

#define SERIAL_PORT 0x3f8
#define SERIAL_IRQ IRQ4

#define DATA_REGISTER 0
#define INTERRUPT_ENABLE_REGISTER 1
#define INTERRUPT_IDENTIFICATION_REGISTER 2
#define LINE_CONTROL_REGISTER 3
#define MODEM_CONTROL_REGISTER 4
#define LINE_STATUS_REGISTER 5
#define MODEM_STATUS_REGISTER 6
#define SCRATCH_REGISTER 7

#define serial_reg_write(reg, val) outb(SERIAL_PORT + reg, val)
#define serial_reg_read(reg) inb(SERIAL_PORT + reg)

static bool buffered = false;
static bool enabled = false;
bool serial_init(int baudrate) {
  // Detect if serial port is present
  serial_reg_write(INTERRUPT_ENABLE_REGISTER, 0x00);
  serial_reg_write(LINE_CONTROL_REGISTER, 0x80);
  serial_reg_write(LINE_CONTROL_REGISTER, 0x00);
  serial_reg_write(INTERRUPT_ENABLE_REGISTER, 0x01);
  if (serial_reg_read(INTERRUPT_IDENTIFICATION_REGISTER) == 0xFF) {
    return false;
  }
  buffered = false;
  serial_reg_write(LINE_CONTROL_REGISTER, 0x80);
  serial_reg_write(DATA_REGISTER, 115200 / baudrate);
  serial_reg_write(INTERRUPT_ENABLE_REGISTER, 0x00);
  serial_reg_write(LINE_CONTROL_REGISTER, 0x03); // 8 bits, no parity, 1 stop bit
  serial_reg_write(MODEM_CONTROL_REGISTER, 0x0B); // IRQs enabled, RTS/DSR set
  enabled = true;
  return true;
}
static volatile char buf[1024];
static volatile int buf_head = 0;
static volatile int buf_tail = 0;

void serial_write_unbuffered(char c) {
  // Wait until the transmit buffer is empty
  while ((serial_reg_read(LINE_STATUS_REGISTER) & 0x20) == 0);
  serial_reg_write(DATA_REGISTER, c);
}

void serial_write_available_handler(u8 vector, itr_frame* frame, void* data) {
  // beep
  //outb(0x61, inb(0x61) | 0x03);
  if (buffered) {
    //printf("%x %x\n", buf_head, buf_tail);
    if (buf_head != buf_tail) {
      if (serial_reg_read(LINE_STATUS_REGISTER) & 0x20) {
        serial_reg_write(DATA_REGISTER, buf[buf_tail]);
        buf_tail = (buf_tail + 1) & (1024 - 1);
      }
    } else {
      serial_reg_write(INTERRUPT_ENABLE_REGISTER, 0x00);
    }
  }
  outb(0x20, 0x20);
}

void serial_set_buffered(bool b) {
  buffered = b;
  if (buffered) {
    serial_reg_write(INTERRUPT_ENABLE_REGISTER, 0x02);
    itr_set_handler(IRQ4, serial_write_available_handler, NULL);
  }
  else {
    serial_reg_write(INTERRUPT_ENABLE_REGISTER, 0x00);
  }
}

void serial_write(char c) {
  if (!buffered) {
    serial_write_unbuffered(c);
  }
  else {
    // Buffer is full, wait until there is space
    while (((buf_head + 1) & (1024 - 1)) == buf_tail) {
      //outb(0x61, inb(0x61) | 0x03);
    }
    buf[buf_head] = c;
    buf_head = (buf_head + 1) & (1024 - 1);
    serial_reg_write(INTERRUPT_ENABLE_REGISTER, 0x02);
  }
}

char serial_read() {
  // Wait until the receive buffer is full
  while ((serial_reg_read(LINE_STATUS_REGISTER) & 0x01) == 0);
  return serial_reg_read(DATA_REGISTER);
}

bool serial_enabled() {
  return enabled;
}

