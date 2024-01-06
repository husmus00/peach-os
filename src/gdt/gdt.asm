section .asm

global gdt_load

gdt_load:
    ; Set the C-supplied GDT struture address
    mov eax, [esp+4]
    mov [gdt_descriptor + 2], eax
    ; Set the C-supplied GDT size
    mov ax, [esp + 8]
    mov [gdt_descriptor], ax
    ; Load the GDT
    lgdt [gdt_descriptor]
    ret

section .data

; The below values are populated by the gdt_load routine (called from C)
gdt_descriptor:
    dw 0x00 ; Size
    dd 0x00 ; GDT start address