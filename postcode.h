#ifndef POSTCODE_H
#define POSTCODE_H
#include "io.h"

static void inline postCode(char val) {
    outb(0x80, val);
}

#endif
