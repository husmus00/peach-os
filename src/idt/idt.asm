section .asm

global idt_load
global enable_interrupts
global disable_interrupts

extern interrupt_handler
global interrupt_pointer_table

global int0Dh
extern int0Dh_handler

global int20h
extern int20h_handler

global int21h
extern int21h_handler

global no_interrupt
extern no_interrupt_handler

global isr80h_wrapper
extern isr80h_handler

; NOTE: interrupts don't need to cli and sti because the interrupt attribute means
; that interrupts will be disabled automatically anyways

; Call from C after loading interrupt table
enable_interrupts:
    sti
    ret

; Can call from C to disable interrupts
disable_interrupts:
    cli
    ret

; Call from C code to load the IDT 
idt_load:
    push ebp
    mov ebp, esp
    
    mov ebx, [ebp+8] ; ebp is the base pointer, ebp + 4 is the callee's return address, ebp + 8 is the idt_table
    lidt [ebx]

    pop ebp
    ret

; Timer interrupt
int0Dh:
    pushad
    call int0Dh_handler
    popad
    iret

; ; Timer interrupt
; int20h:
;     pushad
;     call int20h_handler
;     popad
;     iret

; Keyboard interrupt
int21h:
    pushad
    call int21h_handler
    popad
    iretd

; For when an interupt is not implemented (set the unimplemented interrupts to this)
no_interrupt:
    pushad
    call no_interrupt_handler
    popad
    iret


; Macro for generating an interrupt
%macro interrupt 1
    global int%1
    int%1:
        ; INTERRUPT FRAME START
        ; ALREADY PUSHED TO US BY THE PROCESSOR UPON ENTRY TO THIS INTERRUPT
        ; uint32_t ip
        ; uint32_t cs
        ; uint32_t flags
        ; uint32_t sp
        ; uint32_t ss
        ; ^^^ All these are automatically pushed
        pushad
        ; INTERRUPT FRAME ENDS

        ; Push the stack pointer so we can use it to point to the interrupt frame
        ; (If we don't change it now, it'll get corrupted essentially)
        push esp ; struct interrupt_frame* frame

        ; Push the command value (which specifies which command/interrupt the user program is requesting)
        ; to be used by isr80h_handler
        push dword %1 ; int interrupt

        call interrupt_handler ; C code, will return a void pointer

        ; Add 2 words (8 bytes) to the stack pointer to return it to the state it was in 
        ; before we pushed eax and esp above
        ; (We do this so 'popad' below works correctly)
        add esp, 8

        ; Restore the general purpose registers for user land
        popad

        ; Interrupt return, will restore the registers pushed by the processor at the beginning of this routine
        iretd
%endmacro


; Assign counter
%assign i 0
; Loop and generate 512 interrupts from the above macro
%rep 512
    interrupt i
%assign i i+1
%endrep


isr80h_wrapper:
    ; INTERRUPT FRAME START
    ; ALREADY PUSHED TO US BY THE PROCESSOR UPON ENTRY TO THIS INTERRUPT
    ; uint32_t ip
    ; uint32_t cs
    ; uint32_t flags
    ; uint32_t sp
    ; uint32_t ss
    ; ^^^ All these are automatically pushed
    pushad ; Then we push the general purpose registers to the stack
    ; INTERRUPT FRAME ENDS

    ; Push the stack pointer so we can use it to point to the interrupt frame
    ; (If we don't change it now, it'll get corrupted essentially)
    push esp ; struct interrupt_frame* frame
    
    ; Push the command value held in eax (which specifies which command/interrupt the user program is requesting)
    ; to be used by isr80h_handler
    push eax ; int command

    call isr80h_handler ; C code, will return a void pointer
    mov dword[tmp_res], eax ; The C return/result value will be stored in eax
    
    ; Add 2 words (8 bytes) to the stack pointer to return it to the state it was in 
    ; before we pushed eax and esp above
    ; (We do this so 'popad' below works correctly)
    add esp, 8

    ; Restore the general purpose registers for user land
    popad 

    ; Restore the value returned by the C code (isr80h_handler) since eax may have been changed
    mov eax, [tmp_res]

    ; Interrupt return, will restore the registers pushed by the processor at the beginning of this routine
    iretd


section .data

; Inside here is stored the return result from the isr80h_handler
tmp_res: dd 0

; Macro for generating an interrupt entry in the table
%macro interrupt_array_entry 1
    dd int%1 ; 32 bit address of an interrupt routine described in a macro above
%endmacro

; The interrupt pointers array for use by idt.c
interrupt_pointer_table:
%assign i 0
%rep 512
    interrupt_array_entry i
%assign i i+1
%endrep
