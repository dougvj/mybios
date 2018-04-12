OUTPUT_FORMAT("binary")
OUTPUT("bios.bin")
OUTPUT_ARCH("i386");
ENTRY("main")
MEMORY 
{
    RAM (rw) : ORIGIN = 0x9FC00, LENGTH = 1K
    ROM (rx) : ORIGIN = 0xF0000, LENGTH = 64k
}

SECTIONS
{
    .rodata 0xF0000: AT(0x0) {
        _top = .;
        *(.rodata*) 
        _rodata_end = .;
    }
    .data 0x9FC00: AT(SIZEOF(.rodata)) {
        _data_start = .;
        *(.data)
        _data_end = .;
    }
    .bss ADDR(.data) + SIZEOF(.data): AT(SIZEOF(.data) + SIZEOF(.rodata)){
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    }
    .text 0xF0000 + SIZEOF(.rodata) + SIZEOF(.data) + SIZEOF(.bss): AT(SIZEOF(.data) + SIZEOF(.bss) + SIZEOF(.rodata)) {
        *(.text*)
    }
    .enter_protected_mode 0xFFF70: AT(0xFF70) {
        enter_protected_mode = .;
        *(.enter_protected_mode);
    }
    .reset 0xFFFF0: AT(0xFFF0) {
        *(.reset);
    }
    .fill 0xFFFF0 + SIZEOF(.reset): AT(0xFFF0 + SIZEOF(.reset)) {
        . = 15 - SIZEOF(.reset);
        BYTE(0x00);
    }
}
