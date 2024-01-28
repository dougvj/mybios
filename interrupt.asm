SECTION .real_mode_text
BITS 16
CPU 486
default REL
global fill_ivt
global install_ivt
%include "pcode.asm"

extern serial_write_string
extern serial_write_byte
extern serial_write_hex
extern itr_real_mode_interrupt
extern itr_reload_idt
extern install_vectors

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
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    push ds
    push es
    push esp
    push ss
    push fs
    push gs
    push ax ; Has the interrupt number top of stack
    mov eax, ret
    jmp 0xF000:0xFF70
ret:
BITS 32

    call itr_real_mode_interrupt
    ;cli
    ; Restore real mode idt
    lidt [real_mode_idt]
    jmp dword 0x18:._16_prot
BITS 16
._16_prot:
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    jmp dword 0xF000:(.iret - 0xF0000)
.iret:
    pop bx ; throw away the interrupt number
    pop gs
    pop fs
    ; Atomically pop the SP and SS
    pop bx ; ss
    pop eax; esp
    mov ss, bx
    mov esp, eax
    pop es
    pop ds
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    lidt [esp]
    add esp, 6
    lgdt [esp]
    add esp, 6
    xchg bx, bx
    iret

%macro interrupt_handler 1
int%1:
    xchg bx, bx
    cli
    sub esp, 6
    sgdt [esp]
    sub esp, 6
    sidt [esp]
    push eax
    mov ax, %1
    jmp word trampoline_to_32
%endmacro

%assign i 0
%rep 256
interrupt_handler i
%assign i i + 1
%endrep

handle_timer:
    push ax
    push ds
    mov ax, 0x40
    mov ds, ax
    inc dword [ds:0x6c]
    mov ax, 0x20
    out 0x20, al
    pop ds
    pop ax
    iret

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
