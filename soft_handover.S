BITS 16
ORG 0x70000
%include "pcode.asm"
soft_handover:
PCODE 0x98
; Copy 64k from 0x80000 to 0xF0000. This requires shadowing to be writable
; and the shadowing to be enabled.
mov ax, 0x8000
mov ds, ax
mov ax, 0xF000
mov es, ax
mov cx, 0x4000
;mov cx, 0x0
xor di, di
xor si, si
cld
rep movsd
mov ax, es:[0xFFFE]
mov bx, ds:[0xFFFE]
cmp ax, bx
jne .error
PCODE 0x99
; Reset to reset state
mov ax, 0x0000
mov es, ax
mov ds, ax
mov ss, ax
mov sp, 0x0000
mov bp, 0x0000
mov si, 0x0000
mov di, 0x0000
mov cx, 0x0000
mov dx, 0x0000
mov bx, 0x0000
mov ax, 0x0000
; Jump to the reset vector
jmp dword 0xF000:0xFFF0
;jmp dword 0xFFFF:0x0000
.error:
PCODE 0xEE
jmp $
