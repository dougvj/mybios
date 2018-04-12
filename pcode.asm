%macro PCODE 1
push ax
mov al, %1
out 0x80, al
pop ax
%endmacro
