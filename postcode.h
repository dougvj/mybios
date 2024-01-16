#ifndef POSTCODE_H
#define POSTCODE_H
#include "io.h"

inline static void postCode(char val) {
    outb(0x80, val);
}

#endif
