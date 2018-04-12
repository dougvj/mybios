BITS 16
ORG 0xFFF70
%include "pcode.asm"
protect:
    cli
    mov ax, 0xF000
    mov ds, ax
    lgdt[gdtr] ; ld doesn't like truncating this
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp dword 0x0008:._32main
BITS 32
._32main:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x2000
    mov esp, 0x2000
    PCODE 0x09
    jmp end
gdt:
dd 0x00000000, 0x00000000
dd 0x0000ffff, 0x00Cf9a00
dd 0x0000ffff, 0x00Cf9200
gdtr:
dw gdtr - gdt - 1
dd gdt
end:
