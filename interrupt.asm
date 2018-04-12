BITS 16
CPU 486
default REL
global fill_dummy_ivt
global install_ivt

fill_dummy_ivt:
    mov eax, dummy_handler
    mov cx, 0x100
    xor bx, bx
.loop:
    call install_ivt
    inc bx
    cmp bx, cx
    jl .loop
    ret

dummy_handler:
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
    mov dword [ds:bx], 0xF000;
    pop bx
    pop dx
    pop ds
    ret

