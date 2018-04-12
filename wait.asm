BITS 16
global wait_forever
wait_forever:
    push ax
    push bx
    push dx
    mov ax, 0
    mov bx, 0
    mov dx, 0xFFFF
.loop:
    inc ax
    cmp ax, dx
    jne .loop
    inc bx
    cmp ax, dx
    jne .loop
    pop dx
    pop bx
    pop ax
    ret
