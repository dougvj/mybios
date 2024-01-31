SECTION .real_mode_text
BITS 16
CPU 486
default REL
extern install_ivt
global install_vectors
%include "pcode.asm"

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


handle_keyboard:
    ; Do nothing for now
    push ax
    push ds
    mov ax, 0x20
    out 0x20, al
    pop ds
    pop ax
    iret

ignore:
    iret


%macro install_handler 2
    mov bx, %1
    mov eax, %2
    call install_ivt
%endmacro

install_vectors:
  ; This is a more efficient timer handler which doesn't do the full
  ; context switch.
  ; install_handler 0x08, handle_timer
  ; install_handler 0x09, handle_keyboard
  ; When I install this handler the infinit loop printing "Hello world"
  ; works, but when the protected mode handler is installed it breaks
  ; with invalid opcode after a couple of iterations.
  ;install_handler 0x16, ignore
  ret

