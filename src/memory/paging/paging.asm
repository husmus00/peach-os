[BITS 32]

section .asm

global paging_load_directory
global enable_paging_asm

paging_load_directory:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8] ; Move the passed directory pointer value into eax
    mov cr3, eax ; The cr3 register should contain the address to page directory

    pop ebp
    ret

enable_paging_asm:
    push ebp
    mov ebp, esp

    mov eax, cr0 ; cr0 cannot be manipulated directly
    or eax, 0x80000000 ; Enable bit 31
    mov cr0, eax

    pop ebp
    ret