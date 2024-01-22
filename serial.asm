SECTION .real_mode_text
BITS 16
CPU 486
default REL
global init_serial
global serial_write_byte
global serial_write_string
global serial_write_hex

%define SERIAL_PORT 0x3f8
%define SERIAL_DATA 0x0
%define SERIAL_INT_ENABLE 0x1
%define SERIAL_INT_ID 0x2
%define SERIAL_FIFO_CTRL 0x2
%define SERIAL_LINE_CTRL 0x3
%define SERIAL_MODEM_CTRL 0x4
%define SERIAL_LINE_STATUS 0x5
%define SERIAL_MODEM_STATUS 0x6
%define SERIAL_SCRATCH 0x7

%include "pcode.asm"

%macro SERIAL_WRITE_REG 2
    mov dx, SERIAL_PORT + %0
    mov al, %1
    out dx, al
    mov di, 0x100
%%.loop:
    dec di
    cmp di, 0
    jne %%.loop
%endmacro

%macro SERIAL_READ_REG 1
    mov dx, SERIAL_PORT + %0
    in al, dx
%endmacro

init_serial:
%ifdef ENABLE_EARLY_SERIAL
    push ax
    push dx
    push di
    SERIAL_WRITE_REG SERIAL_LINE_CTRL, 0x80
    SERIAL_WRITE_REG SERIAL_DATA, 115200 / 115200
    SERIAL_WRITE_REG SERIAL_INT_ENABLE, 0x0
    SERIAL_WRITE_REG SERIAL_LINE_CTRL, 0x3
    SERIAL_WRITE_REG SERIAL_MODEM_CTRL, 0x3
    pop di
    pop dx
    pop ax
%endif
    ret

serial_write_byte:
%ifdef ENABLE_EARLY_SERIAL
    push dx
    push ax
    mov dx, 0x3f8
    add dx, 0x5
.loop:
    in al, dx
    and al, 0x20
    cmp al, 0x0
    jz .loop
    pop ax
    sub dx, 0x5
    out dx, al
    pop dx
%endif
    ret

serial_write_string:
    push eax
    cmp byte [si], 0
    je .done
.loop:
    mov al, [si]
    call serial_write_byte
    inc si
    cmp byte [si], 0
    jne .loop
.done:
    pop eax
    ret


serial_write_hex:
    cmp eax, 0x10
    jl .one
    push eax
    shr eax, 4
    call serial_write_hex
    pop eax
    push eax
    and eax, 0x0F
    call serial_write_hex
    pop eax
    ret
.one:
    cmp eax, 0x9
    jle .num
    sub eax, 0x0A
    add eax, 0x41
    jmp .write
.num:
    add eax, 0x30
.write:
    call serial_write_byte
    ret
