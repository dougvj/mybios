OUTPUT_FORMAT("elf32-i386")
OUTPUT("bios.bin")
OUTPUT_ARCH("i386");
ENTRY("reset_vector")

/* Define the size of the ROM */
#define ROM_SIZE 64k

#define ROM_START 0x100000000 - ROM_SIZE
MEMORY
{
    RAM (rw) : ORIGIN = 0x80000, LENGTH = 128K
    RAM32 (rw) : ORIGIN = 0x100000, LENGTH = 1M
    LOWER_ROM (rx) : ORIGIN = 0xF0000, LENGTH = 64K
    UPPER_ROM (rx) : ORIGIN = ROM_START, LENGTH = ROM_SIZE
}

SECTIONS
{
    .signature ROM_START : {
        *(.signature*)
    }
    .text .  :  {
        *(.text*)
    }
    .rodata . : {
        *(.rodata*)
    }
    .data : AT(ADDR(.rodata) + SIZEOF(.rodata)) {
        _data_start = .;
        *(.data)
        _data_end = .;
    } > RAM32
    _data_load_start = LOADADDR(.data);
    #define REAL_MODE_TEXT_LOCATION \
        (MAX(0xFFFF0000, 0xFFFF0000 + (SIZEOF(.signature) + SIZEOF(.data) + SIZEOF(.rodata) + SIZEOF(.text)) - (ROM_SIZE - 64K)))
    .real_mode_text REAL_MODE_TEXT_LOCATION - 0xFFF00000 : AT(REAL_MODE_TEXT_LOCATION)  {
        *(.real_mode_text*);
    }
    .call_real_mode 0xFFF20 : AT(0xFFFFFF20)  {
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
    }
}
