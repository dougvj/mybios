BITS 16
ORG 0xFFF70
%include "pcode.asm"
protect:
    push eax
    mov ax, 0xF000
    mov ds, ax
    lgdt[gdtr] ; ld doesn't like truncating this
    mov eax, cr0
    or al, 1
    mov cr0, eax
    PCODE 0xAA
    jmp dword 0x0008:._32main
BITS 32
._32main:
    PCODE 0x55
    ;Set up the stack to be the same as real mode
    mov ax, ss
    and eax, 0x0000FFFF
    shl eax, 4
    ;and esp, 0x0000FFFF
    ;and ebp, 0x0000FFFF
    add esp, eax
    add ebp, eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
;    mov fs, ax
;    mov gs, ax
    mov ss, ax
    ;PCODE 0x09
    pop eax
    jmp eax
gdt:
dd 0x00000000, 0x00000000
dd 0x0000ffff, 0x00Cf9a00
dd 0x0000ffff, 0x00Cf9200
; Used for switching to real mode, G = 0, DB=0
dd 0x0000ffff, 0x000f9a00
; Unused, 16-bit prot data
dd 0x0000ffff, 0x000f9200
gdtr:
dw gdtr - gdt - 1
dd gdt
