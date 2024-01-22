#include "vga.h"
#include <stdarg.h>
#include "io.h"
#include "serial.h"
#include "types.h"
#include "interrupts.h"

static bool vga_enabled = false;
void putc(char c)
{
    if (vga_enabled)
      vgaPutChar(c);
#ifdef ENABLE_SERIAL
    /*if (c == '\n') {
        serial_write('\r');
    }*/
    if (serial_enabled()) {
      // We only want to use the buffered serial port if interrupts are enabled
      if (interrupts_enabled()) {
          serial_write(c);
      }
      else {
          serial_write_unbuffered(c);
      }
    }
#endif
    // This is a bochs thing?
#ifdef BOCHS
    outb(0xE9, c);
#endif
}

void set_vga_enabled(bool enabled) {
    vga_enabled = enabled;
}

void itoa(dword value, char* buf, int base) {
    int len = 0;
    while (value != 0) {
        dword d = value % base;
        value = value / base;
        byte c;
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
    if (len == 0)
        buf[len++] = '0';
    buf[len] = '\0';
}

//printf that supports only some of the common formats, namely %i,%n
void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char *s = (char*)format;
     while (*s != '\0') {
         if (*s == '%') {
            s++;
            char* ins;
            char conv[10];
            dword value;
             switch (*s) {
                case 'u':
                case 'd':
                case 'i':
                    value = va_arg(args, dword);
                    itoa(value, conv, 10);
                    ins = conv;
                    break;
                case 'x':
                case 'X':
                    value = va_arg(args, dword);
                    itoa(value, conv, 16);
                    ins = conv;
                    break;
                case 's':
                    ins = va_arg(args, char*);
                    break;
                default:
                    va_arg(args, char*);
                    ins = "";
            }
            s++;
            while (*ins != '\0') {
                putc(*ins++);
//                outb(0x402, *ins);
            }
        }
         else {
            putc(*s);
//            outb(0x402, *s);
            s++;
        }
    }
    va_end(args);
}

void dumpmem(dword addr, dword len) {
  for (dword i = 0; i < len; i++) {
    if ((i & 0x7) == 0) {
      printf("\n%X: ", addr + i);
    }
    printf("%X ", (dword)(*(byte*)(addr + i)));
  }
  printf("\n");
}
