BITS 16
ORG 0xF0000
c_start:
incbin "c_code.bin"

%macro PCODE 1
push ax
mov al, %1
out 0x80, al
pop ax
%endmacro

%macro HANDLER 1
handler_%1:
mov al, %1
out 0x80, al
out 0x80, al
out 0x80, al
iret
%endmacro

HelloWorldStr:
db "Hello World! I think it works!", 0
ProtectedStr:
db "Now we are in protected mode, yay!", 0


;Descriptor table for flat addressing
gdt:
dd 0x00000000, 0x00000000
dd 0x0000ffff, 0x00Cf9a00
dd 0x0000ffff, 0x00Cf9200

gdtr:
dw gdtr - gdt - 1
dd gdt

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


print_string:
    push es
    mov bx, 0xB800
    mov es, bx
    mov si, ax
    mov di, 0x0
.start:
    mov al, [ds:si]
    test al, al
    jnz .write
    pop es
    ret
.write:
    mov [es:di], al
    inc di
    mov al, 0x1f
    mov [es:di], al
    inc si
    inc di
    jmp .start

init_video:
    push ds
    mov bx, 0xC000
    mov ds, bx
    PCODE 0x6
.search_video:
    mov ax, [ds:0x00]
    cmp ax, 0xAA55
    je .call_video
    mov bx, ds
    add bx, 0x80
    mov ds, bx
    cmp bx, 0xC800
    jne .search_video
    PCODE 0xEE
    hlt
.call_video:
    PCODE 0x07
    ;Use BIOS RAM area for our jump vector for now
    mov ax, 0x0040
    mov ds, ax
    mov [ds:0x0002], bx
    mov bx, 0x0003
    mov [ds:0x0000], bx
    call far [ds:0x0000]
    pop ds
    ret

dump_sig:
    mov bx, 0xC000
    mov ds, bx
    mov bx, 0x0
    mov al, [ds:bx]
    out 0x80, al
    call wait_forever
    inc bx
    mov al, [ds:bx]
    out 0x80, al
    call 0xC000:0x0003
    PCODE 0xEE
    hlt


HANDLER 0x1
HANDLER 0x2
HANDLER 0x3
HANDLER 0x4
HANDLER 0x5
HANDLER 0x6
HANDLER 0x7
HANDLER 0x8
HANDLER 0x9
HANDLER 0xA
HANDLER 0xB
HANDLER 0xC
HANDLER 0xD
HANDLER 0xE
HANDLER 0xF
HANDLER 0x10
HANDLER 0x11
HANDLER 0x12
HANDLER 0x13
HANDLER 0x14
HANDLER 0x15
HANDLER 0x16
HANDLER 0x17
HANDLER 0x18
HANDLER 0x19
HANDLER 0x1A
HANDLER 0x1B

IVT:
dw  handler_0x1,0xF000
dw  handler_0x2,0xF000
dw  handler_0x3,0xF000
dw  handler_0x4,0xF000
dw  handler_0x5,0xF000
dw  handler_0x6,0xF000
dw  handler_0x7,0xF000
dw  handler_0x8,0xF000
dw  handler_0x9,0xF000
dw  handler_0xA,0xF000
dw  handler_0xB,0xF000
dw  handler_0xC,0xF000
dw  handler_0xD,0xF000
dw  handler_0xE,0xF000
dw  handler_0xF,0xF000
dw  handler_0x10,0xF000
dw  handler_0x11,0xF000
dw  handler_0x12,0xF000
dw  handler_0x13,0xF000
dw  handler_0x14,0xF000
dw  handler_0x15,0xF000
dw  handler_0x16,0xF000
dw  handler_0x17,0xF000
dw  handler_0x18,0xF000
dw  handler_0x19,0xF000
dw  handler_0x1A,0xF000
dw  handler_0x1B,0xF000
IVT_end:


fill_vec:
    push es
    push ds
    mov cx, IVT_end - IVT
    xor ax, ax
    mov es, ax
    mov di, ax
    mov ax, cs
    mov ds, ax
    mov si, IVT
    rep movsb
    pop ds
    pop es
    ret


start:
    cli
    PCODE 0x00
;    call wait_forever
;    call wait_forever
;    call wait_forever
;    call wait_forever
;    call wait_forever
    call wait_forever
    call wait_forever
    call wait_forever
    PCODE 0x01
    ;Wait for ram stabilitiy if ever
    mov ax, 0x0040
    mov ds, ax
    mov ax, 0xaaaa
    mov [ds:0x0000], ax
    mov bx, [ds:0x0000]
    cmp ax, bx
    ;If ram isn't stable, keep trying
    jne start
    mov ax, 0x0000
    mov ss, ax
    mov bp, 0x400
    PCODE 0x03
    call fill_vec
    PCODE 0x04
    call init_video
    PCODE 0x08
    mov ax, 0xf000
    mov ds, ax
    mov ax, HelloWorldStr
    call print_string
    ;Now let's try to enter 32-bit mode
protect:
    mov ax, 0xF000
    mov ds, ax
    lgdt[gdtr]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp dword 0x0008:_32main
BITS 32
_32main:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x2000
    mov esp, 0x2000
    PCODE 0x09
    mov ecx, 0
    mov esi, ProtectedStr
    mov edi, 0xB80A0
    mov al, [esi]
.loop:
    mov [edi], al
    inc edi
    mov [edi], byte 0x1f
    inc edi
    inc esi
    mov al, [esi]
    test al, al
    jnz .loop
    cli
    jmp c_start

BITS 16
TIMES 0xFFF0 - ($ - $$) db 0
entry:
    jmp dword start
TIMES 0x10000 - ($ - $$) db 0
