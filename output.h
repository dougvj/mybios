#ifndef OUTPUT_H
#define OUTPUT_H
#include "vga.h"
#include "types.h"


void putc(char c);
//printf that supports only some of the common formats, namely %i/%d, %u and %f
int printf(const char* format, ...);

void dumpmem(dword addr, dword len);

void set_vga_enabled(bool enabled);

#define cls vgaCls
#endif
