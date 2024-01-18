SECTION .real_mode_text
BITS 16
CPU 486
default REL
global fill_dummy_ivt
global install_ivt

extern serial_write_string
extern serial_write_byte
extern serial_write_hex

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
    ret


msg:
  db "Interrupt Handler: ", 0
endl:
  db 0xa, 0xd, 0

print_handler:
    push ds
    xchg ax, bx
    mov ax, 0xF000
    mov ds, ax
    xchg ax, bx
    push esi
    mov esi, msg
    call serial_write_string
    call serial_write_hex
    mov esi, endl
    call serial_write_string
    pop esi
    pop ds
    pop eax
    mov bx, 0x1234
    mov ax, 0x4321
    iret

%macro interrupt_handler 1
int%1:
    push eax
    mov eax, %1
    jmp word print_handler
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

call_int_msg:
db "Call Interrupt: ", 0

global call_int
call_int:
    push ds
    push ax
    push esi
    mov ax, 0x0
    mov ds, ax
    mov esi, call_int_msg
    call serial_write_string
    mov ax, di
    call serial_write_hex
    pop ax
    shl di, 2
    pushf
    call far [ds:di]
    pop ds
    pop esi
    retf
