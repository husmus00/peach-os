[BITS 32]
global _start
extern kernel_main

global kernel_registers

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    ; SET UP SEGMENT REGISTERS
    ; DS, SS, ES, FS, GS
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ; Enable the A20 line
    in al, 0x92
    mov al, 2
    out 0x92, al

    ;
    ; Remap the master PIC (Programmable Interrupt Controller)
    mov al, 0010001b
    out 0x20, al ; Tell the master PIC

    mov al, 0x20 ; Interrupt 0x20 is where the master ISR should start
    out 0x21, al

    mov al, 00000001b ; Put the PIC in x86 mode
    out 0x21, al
    ; End remap the master PIC
    ;

    call kernel_main
    jmp $

; Set the segment registers to point to the kernel data segment
kernel_registers:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    ret

; To avoid any alignment issues with the C code that will come after this section
times 512-($$-$) db 0