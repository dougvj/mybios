BITS 16
CPU 486
%macro PCODE 1
push ax
mov al, %1
out 0x80, al
pop ax
%endmacro

extern wait_forever
extern fill_dummy_ivt
extern init_video
extern main
extern enter_protected_mode

SECTION .text
start:
    cli
    PCODE 0x00
    call wait_forever
    call wait_forever
    call wait_forever
    PCODE 0x01
    ;Wait for ram stabilitiy if ever
    mov ax, 0x0040
    mov ds, ax
    mov ax, 0xaaaa
    mov [ds:0x0000], ax
    mov bx, [ds:0x0000]
    cmp ax, bx
    ;If ram isn't stable, keep trying
    jne start
    mov ax, 0x0000
    mov ss, ax
    mov bp, 0x400
    PCODE 0x03
    call fill_dummy_ivt
    PCODE 0x04
    call init_video
    PCODE 0x08
    jmp enter_protected_mode

BITS 16
SECTION .enter_protected_mode
incbin "enter_protected_mode.bin"
BITS 32
call main
BITS 16
SECTION .reset
reset_vector:
    jmp dword start
