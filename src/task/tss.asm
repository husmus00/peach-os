section .asm

global tss_load

tss_load:
    push ebp
    mov ebp, esp

    mov ax, [ebp+8]
    ltr ax ; Loads in the task switch segment (TSS)
    
    pop ebp
    ret