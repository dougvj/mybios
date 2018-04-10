#include "vga.h"
#include <stdarg.h>

void itoa(int value, char* buf, int base) {
    int len = 0;
    while (value != 0) {
        int d = value % base;
        value = value / base;
        char c;
        if (d < 10) {
            c = 0x30 + d;
        }
        else {
            c = 0x41 + (d - 10);
        }
        buf[len++] = c;
    }
    if (len == 0) {
        buf[0] = '0';
        buf[1] = '\0';
    }
    else {
        //Reverse str
        for (int i = 0; i < len/2; i++) {
            char tmp = buf[i];
            buf[i] = buf[len-i-1];
            buf[len-i-1] = tmp;
        }
    }
    buf[len] = '\0';
}

//printf that supports only some of the common formats, namely %i/%d, %u and %f
int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char *s = (char*)format;
     while (*s != '\0') {
         if (*s == '%') {
            s++;
            char* ins;
            char conv[10];
            int value;
             switch (*s) {
                case 'u':
                case 'd':
                case 'i':
                    value = va_arg(args, int);
                    itoa(value, conv, 10);
                    ins = conv;
                    break;
                case 'x':
                case 'X':
                    value = va_arg(args, int);
                    itoa(value, conv, 16);
                    ins = conv;
                    break;
                case 's':
                    ins = va_arg(args, char*);
                    break;
            }
            s++;
            while (*ins != '\0') {
             vgaPutChar(*ins++);
            }
        }
         else {
            vgaPutChar(*s);
            s++;
        }
    }
    va_end(args);
}

