SECTION .real_mode_text
BITS 16
CPU 486
default REL
global fill_ivt
global install_ivt
global trampoline_to_32
%include "pcode.asm"

extern serial_write_string
extern serial_write_byte
extern serial_write_hex
extern itr_real_mode_interrupt
extern itr_reload_idt
extern install_vectors
extern printf

fill_ivt:
    xor bx, bx
    mov eax, int0
    mov cx, 0x100
.loop:
    call install_ivt
    inc bx
    add eax, int1-int0
    cmp bx, cx
    jl .loop
    ; Install special handlers written in real mode asm
    call install_vectors
    ret


msg:
  db "Interrupt Handler: ", 0
endl:
  db 0xa, 0xd, 0

stack_overflow_msg:
  db "Stack Overflow", 0

print_handler:
    push ds
    mov ax, 0xF000
    mov ds, ax
    push esi
    mov esi, msg
    call serial_write_string
    mov eax, ebx
    call serial_write_hex
    mov esi, endl
    call serial_write_string
    pop esi
    pop ds
    pop eax
    mov bx, 0x1234
    mov ax, 0x4321
    iret

trampoline_to_32:
    push gs ;
    push ebp ;
    mov ebp, 0x9c00; Segment of stack area, see rom_template.ld
    mov gs, bp; ds = 0
    mov [gs:0], esp
    mov [gs:4], ss
    ; Sentinel value
    mov dword [gs:8], 0x12345678
    mov ss, ebp
    ; Use interrup stack
    mov sp, 0x1000
    ; Copy ebp, gs, flags, cs, and ip, esp, and ss,
    ; to the interrupt stack in interrupt.c
    sub sp, 18
    push eax ; save eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ds
    push es
    push fs
    ;xchg bx, bx
    mov eax, ret
.enter_32:
    jmp 0xF000:0xFF70
ret:
BITS 32

    call itr_real_mode_interrupt
    ;cli
    ; Restore real mode idt
    lidt [real_mode_idt]
skip_call:
    ; Check that our stack is still valid
    mov eax, 0x9c008
    cmp dword [eax], 0x12345678
    jne .stack_overflow
    jmp dword 0x18:._16_prot
.stack_overflow:
    push stack_overflow_msg
    call printf
.hlt:
    hlt
    jmp .hlt
BITS 16
._16_prot:
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp dword 0xF000:(iret - 0xF0000)
iret:
    ; Restore real mode stack
    mov ax, 0x9c00
    mov gs, ax
    mov ss, ax
    shl eax, 4
    sub esp, eax
    pop fs
    pop es
    pop ds
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    ; Go back to real mode stack
    mov dword esp, [gs:0]
    mov word ss, [gs:4]
    pop ebp ; restore ebp
    pop gs ; restore gs, which was the second thing we pushed
    add sp, 2; throw away the interrupt number
    iret

%macro interrupt_handler 1
int%1:
    cli
    push word %1
    jmp word trampoline_to_32
%endmacro

%assign i 0
%rep 256
interrupt_handler i
%assign i i + 1
%endrep

;ax address of vector
;bx interrupt to install
install_ivt:
    push ds
    push dx
    push bx
    xor dx, dx
    mov ds, dx
    shl bx, 2
    mov [ds:bx], ax
    add bx, 2
    mov word [ds:bx], 0xF000;
    pop bx
    pop dx
    pop ds
    ret

global call_int
call_int:
    push ds
    push ax
    mov ax, 0x0
    mov ds, ax
    pop ax
    shl di, 2
    pushf
    call far [ds:di]
    pop ds
    retf
real_mode_idt:
    dw 0x3FF, 0x0
    dd 0x0
