#ifndef IO_H
#define IO_H

static inline void outb(unsigned short port, unsigned char value) {
    asm("out %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm("in %1, %0" : "=a" (ret) : "Nd" (port));
    return ret;
}

#endif
