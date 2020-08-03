BITS 16
CPU 486
%include "pcode.asm"
extern wait_forever
extern fill_dummy_ivt
extern init_video
extern main
extern enter_protected_mode


%macro WAIT 1
    push ax
    mov ax, %1
    %%do_nop:
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        dec ax
        jnz %%do_nop
    pop ax
%endmacro

%macro DUMP_BYTE 3
PCODE %1
WAIT 0xFFFF
PCODE %2
WAIT 0xFFFF
PCODE %1
WAIT 0xFFFF
PCODE %3
WAIT 0xFFFF
%endmacro

SECTION .real_mode_text

entry_point:
    PCODE 0x00
    cli
    PCODE 0x01
    ;call wait_forever
    ;call wait_forever
    ;call wait_forever
    ;Wait for ram stabilitiy if ever
    mov si, 0x0000
    mov cx, 0x0000
    mov ds, cx
test_byte:
    mov cx, si
    mov al, 0xAA
    mov [ds:si], al
    mov bl, [ds:si]
    cmp al, bl
    ;If ram isn't stable, keep trying
    jne test_fail
    mov al, 0x55
    mov [ds:si], al
    mov bl, [ds:si]
    cmp al, bl
    jne test_fail 
    inc si
    jz init_runtime_16
    jmp test_byte
test_fail:
    mov cx, si
    WAIT 0xFFF
    DUMP_BYTE 0xFF, ch, cl
    DUMP_BYTE 0xFD, ah, al
    DUMP_BYTE 0xFE, bh, bl
    WAIT 0xFFF
    jmp test_fail
init_runtime_16:
    PCODE 0x02
    mov ax, 0x0000
    mov ss, ax
    mov bp, 0x400
    PCODE 0x03
    call fill_dummy_ivt
    PCODE 0x04
    call init_video
    PCODE 0x08
    jmp dword enter_protected_mode

SECTION .reset
BITS 16
global reset_vector
PCODE 0xCA
jmp dword init_runtime_16

SECTION .enter_protected_mode
BITS 16
incbin "enter_protected_mode.bin"
BITS 32
PCODE 0x10
call main
SECTION .signature
dd 0xDEADBEEF
