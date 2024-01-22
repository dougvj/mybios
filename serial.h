#ifndef SERIAL_H
#define SERIAL_H
#include "io.h"
#include "types.h"

bool serial_init(int baudrate);
void serial_write(char c);
void serial_write_unbuffered(char c);
char serial_read();
void serial_set_buffered(bool b);
bool serial_enabled();
#endif // SERIAL_H
