#include "output.h"
extern char _bss_start, _bss_end, _data_start, _data_load_start, _data_end;

void initRuntime() {
    //printf("Initializing runtime areas\n");
    char *src = &_data_load_start;
    char *dst = &_data_start;
    //printf(".data: %d b\n", &_data_end - &_data_start);
    while (dst < &_data_end)
        *dst++ = *src++;
    dst = &_bss_start;
    //printf(".bss: %d b\n", &_bss_end - &_bss_start);
    while (dst < &_bss_end) {
        *dst++ = 0;
    }
}
