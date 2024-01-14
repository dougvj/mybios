#ifndef SERIAL_H
#define SERIAL_H
#include "io.h"

void serial_init(int baudrate);
void serial_write(char c);
char serial_read();


#endif // SERIAL_H
