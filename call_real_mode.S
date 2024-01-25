BITS 32
ORG 0xFFF00
real_mode_call:
    cli
    ; eax has the real mode segment, save it
    push eax
    jmp dword 0x18:._16prot
BITS 16
._16prot:
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    jmp dword 0xF000:(._16call - 0xF0000)
._16call:
    push ds
    mov ax, 0xF000
    mov ds, ax
    lidt [_idt]
    pop ds
    pop eax
    push bp
    mov bp, sp
    xchg bx, bx
    sti
    call far [ss:bp+6]
    cli
    pop bp
    ; Renenter protected mode at ret
    push eax
    mov eax, ret
    jmp 0xF000:0xFF70
ret:
BITS 32
    sti
    pop eax
    ret
_idt:
    dw 0x3FF, 0x0
    dd 0x0
