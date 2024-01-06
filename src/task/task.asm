[BITS 32]

section .asm

global task_return
global restore_general_purpose_registers
global user_registers

; void task_return(struct registers* regs);
task_return:
    ; We are essentially falsifying an interrupt stack frame
    ; Calling iret will then restore these values and go to user land
    ; Execution will then start from the 'ip' value in the provided struct
    mov ebp, esp
    ; PUSH THE DATA SEGMENT (SS WILL BE FINE)
    ; PUSH THE STACK ADDRESS
    ; PUSH THE FLAGS
    ; PUSH THE CODE SEGMENT
    ; PUSH THE IP

    ; Accessing the struct passed to this routine
    mov ebx, [ebp+4]
    ; Push the data/stack selector (SS)
    push dword [ebx+44]
    ; Push the stack pointer
    push dword [ebx+40]
    ; Push the flags
    pushf ; Push flags then pop into eax
    pop eax
    or eax, 0x200 ; This bit will reenable flags upon iret
    push eax ; Finally push the flags (with the interrupt bit) onto the stack
    ; Push the code segment
    push dword [ebx+32]
    ; Push the IP to execute (expects a virtual address because paging is enabled)
    push dword [ebx+28]

    ; Setup some segment registers
    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; No need to manually handle the stack segment, it will be handled by the CPU upon iret

    ; Now restore the registers
    push dword [ebp+4] ; push the struct address to the next stack frame
    call restore_general_purpose_registers
    add esp, 4 ; Because we pushed 4 bytes (the struct address) onto the stack before calling restore_general_purpose_registers

    ; Leave kernel land and execute in user land!
    iretd

; void restore_general_purpose_registers(struct registers* regs);
restore_general_purpose_registers:
    push ebp
    mov ebp, esp
    mov ebx, [ebp+8] ; Starting point for the struct from the C call
    
    ; Now each register will be populated from a an offset into the registers struct
    mov edi, [ebx]
    mov esi, [ebx+4]
    mov ebp, [ebx+8]
    mov edx, [ebx+16]
    mov ecx, [ebx+20]
    mov eax, [ebx+24]
    mov ebx, [ebx+12] ; ebx is last to avoid overwriting the registers struct address

    ; pop ebp ; This was a bug, we already restored ebp above. This would overwrite it.
    ; This restores the correct stack address without overwriting ebp:
    add esp, 4
    ret

; void user_registers();
user_registers:
    ; Change all the segment registers to the user data segment registers
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret