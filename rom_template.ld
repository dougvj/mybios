OUTPUT_FORMAT("elf32-i386")
OUTPUT("bios.bin")
OUTPUT_ARCH("i386");
ENTRY("reset_vector")

#define ROM_START 0x100000000 - ROM_SIZE

#ifdef LOWER_ROM_128K
#define LOWER_ROM_START 0xE0000
#define LOWER_ROM_SIZE 128K
#else
#define LOWER_ROM_START 0xF0000
#define LOWER_ROM_SIZE 64K
#endif


MEMORY
{
    RAM (rw) : ORIGIN = 0x9d000, LENGTH = 12K
    STACK (rw) : ORIGIN = 0x9c000, LENGTH = 4K
    BDA (rw) : ORIGIN = 0x400, LENGTH = 0x100
    LOWER_ROM (rx) : ORIGIN = LOWER_ROM_START, LENGTH = LOWER_ROM_SIZE
    UPPER_ROM (rx) : ORIGIN = ROM_START, LENGTH = ROM_SIZE
}

SECTIONS
{
    .signature ROM_START : {
        *(.signature*)
    }
    .data  : AT(ADDR(.signature) + SIZEOF(.signature)) {
        _data_start = .;
        *(.data)
        _data_end = .;
    } > RAM
    _data_load_start = LOADADDR(.data);
    #define LOWER_ROM_LOCATION \
        (MAX(LOWER_ROM_START, LOWER_ROM_START + (SIZEOF(.signature) + SIZEOF(.data) - (ROM_SIZE - LOWER_ROM_SIZE))))
    .rodata LOWER_ROM_LOCATION: AT(LOWER_ROM_LOCATION + 0xFFF00000) {
        *(.rodata*)
    }
    .text . :  {
        *(.text*)
    }
    .real_mode_text . :  {
        *(.real_mode_text*);
    }
    .call_real_mode 0xFFF00 : AT(0xFFFFFF00)  {
        call_real_mode = .;
        *(.call_real_mode*);
    }
    .enter_protected_mode 0xFFF70 : AT(0xFFFFFF70)  {
        enter_protected_mode = .;
        *(.enter_protected_mode*);
    }
    .reset 0xFFFF0 : AT(0xFFFFFFF0) {
        reset_vector = .;
        *(.reset*);
    }
    .fill 0xFFFFFFFF : AT(0xFFFFFFFF) {
        BYTE(0x0)
    }
    .bss (NOLOAD) : ALIGN(0x4) {
        _bss_start = .;
        *(.bss*)
        _bss_end = .;
    } > RAM
    .bda (NOLOAD) : ALIGN(0x4) {
        _bda_start = .;
        *(.bda*)
        _bda_end = .;
    } > BDA
    .stack (NOLOAD) : ALIGN(0x4) {
        _stack_start = .;
        *(.stack*)
        _stack_end = .;
    } > STACK
}
