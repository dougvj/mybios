OUTPUT_FORMAT("binary")
OUTPUT("bios.bin")
OUTPUT_ARCH("i386");
ENTRY("main")
MEMORY 
{
    RAM_DATA (rw) : ORIGIN = 0x2400, LENGTH = 3072
    RAM_BSS (rw) : ORIGIN = 0x400, LENGTH = 3072
    ROM (rx) : ORIGIN = 0xF0000, LENGTH = 64k
}

SECTIONS
{
    .text.startup 0xF0000: AT(0x0) {
        *(.text.startup*)
    }
    .text SIZEOF(.text.startup) + 0xF0000: AT(SIZEOF(.text.startup)) {
        *(.text*)
        *(.rodata*) 
        _text_end = .;
    }
    .data 0x2400: AT(ADDR(.text) + SIZEOF(.text)){
        _data_start = .;
        *(.data)
        _data_end = .;
    }
    .bss 0x400: AT(ADDR(.data) + SIZEOF(.data)){
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    }
}
