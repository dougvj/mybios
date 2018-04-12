#ifndef OUTPUT_H
#define OUTPUT_H
#include "vga.h"
//printf that supports only some of the common formats, namely %i/%d, %u and %f
int printf(const char* format, ...);
#define cls vgaCls
#endif
