SECTION .real_mode_text
BITS 16
%include "pcode.asm"

global init_video
init_video:
    push ds
    mov bx, 0xC000
    mov ds, bx
    PCODE 0x6
.search_video:
    mov ax, [ds:0x00]
    cmp ax, 0xAA55
    je .call_video
    mov bx, ds
    add bx, 0x80
    mov ds, bx
    cmp bx, 0xC800
    jne .search_video
    PCODE 0xEE
    pop ds
    ret
.call_video:
    PCODE 0x07
    ;Use BIOS RAM area for our jump vector for now
    mov ax, 0x0040
    mov ds, ax
    mov [ds:0x0002], bx
    mov bx, 0x0003
    mov [ds:0x0000], bx
    call far [ds:0x0000]
    pop ds
    ret
