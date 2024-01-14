#include "serial.h"

#define SERIAL_PORT 0x3f8

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

void serial_init(int baudrate) {
  serial_reg_write(LINE_CONTROL_REGISTER, 0x80);
  serial_reg_write(DATA_REGISTER, 115200 / baudrate);
  serial_reg_write(INTERRUPT_ENABLE_REGISTER, 0x00);
  serial_reg_write(LINE_CONTROL_REGISTER, 0x03); // 8 bits, no parity, 1 stop bit
  serial_reg_write(MODEM_CONTROL_REGISTER, 0x03); // DTR and RTS
}

void serial_write(char c) {
  // Wait until the transmit buffer is empty
  while ((serial_reg_read(LINE_STATUS_REGISTER) & 0x20) == 0);
  serial_reg_write(DATA_REGISTER, c);
}

char serial_read() {
  // Wait until the receive buffer is full
  while ((serial_reg_read(LINE_STATUS_REGISTER) & 0x01) == 0);
  return serial_reg_read(DATA_REGISTER);
}


