SECTION .real_mode_text
BITS 16
CPU 486
default REL
global fill_dummy_ivt
global install_ivt

%include "pcode.asm"

extern serial_write_string
extern serial_write_byte
extern serial_write_hex
extern itr_real_mode_interrupt
extern itr_reload_idt

fill_dummy_ivt:
    xor bx, bx
    mov eax, int0
    mov cx, 0x100
.loop:
    call install_ivt
    inc bx
    add eax, int1-int0
    cmp bx, cx
    jl .loop
    ; Ignore timer interrupt to see if our problems go away
    mov bx, 32
    call install_ivt
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
    ;xchg bx, bx
    push bx
    push cx
    push dx
    push si
    push di
    push bp
    push ds
    push es
    push sp
    push ss
    push ax ; Has the interrupt number top of stack
    mov eax, ret
    jmp 0xF000:0xFF70
ret:
BITS 32
    call itr_real_mode_interrupt
    ;cli
    ; Restore real mode idt
    lidt [real_mode_idt]
    pop bx ; throw away the interrupt number
    xor ecx, ecx
    pop cx ; this will be the new ss
    xor ebx, ebx
    pop bx ; this will be the new sp
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
    ; Restore the stack then restore the registers
    mov esp, ebx ; for some asinine reason, the high word of esp is used
    mov ss, cx
    pop es
    pop ds
    pop bp
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ;xchg bx, bx
    iret

%macro interrupt_handler 1
int%1:
    cli
    push ax
    mov ax, %1
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
    mov dword [ds:bx], 0xF000;
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
