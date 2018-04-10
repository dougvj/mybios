extern char _bss_start, _bss_end, _data_start, _data_end, _text_end;

void initRuntime() {
    char *src = &_text_end;
    char *dst = &_data_start;
    while (dst < &_data_end)
        *dst++ = *src++;
    for (dst = &_bss_start; dst < &_bss_end; dst++)
        *dst = 0;
}
