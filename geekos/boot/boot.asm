    org 07c00h
    mov ax, cs
    mov ds, ax
    mov es, ax
    call DisplayString
    jmp $
DisplayString:
    mov ax, BootMessage
    mov bp, ax
    mov cx, 21
    mov ax, 01301h
    mov bx, 000ch
    mov dl, 0
    int 10h
    ret
BootMessage:     db "Hello miao-yuyang OS!"
times 510-($-$$) db 0
dw 0xaa55
