#include "output.h"
extern char _bss_start, _bss_end, _data_start, _data_load_start, _data_end;

void initRuntime() {
    vgaCls(0);
    printf("Initializing runtime areas from %x\n", &_data_load_start);
    char *src = &_data_load_start;
    char *dst = &_data_start;
    printf(".data: %d bytes at 0x%x\n", &_data_end - &_data_start, &_data_start);
    while (dst < &_data_end)
        *dst++ = *src++;
    dst = &_bss_start;
    printf(".bss: %d bytes at 0x%x\n", &_bss_end - &_bss_start, &_bss_start);
    while (dst < &_bss_end) {
        *dst++ = 0;
    }
}
