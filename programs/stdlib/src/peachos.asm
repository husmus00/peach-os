[BITS 32]

section .asm

global peachos_exit:function
global print:function
global peachos_putchar:function
global peachos_getkey:function
global peachos_malloc:function
global peachos_free:function
global peachos_process_load_start:function
global peachos_system:function
global peachos_process_get_arguments:function

; Command 0 void peachos_exit(int exit_code)
peachos_exit:
    push ebp
    mov ebp, esp

    push dword[ebp+8] ; Exit code
    mov eax, 0 ; Command 0 process exit
    int 0x80

    add esp, 4
    pop ebp
    ret

; command 1 void print(const char* filename)
print:
    push ebp

    mov ebp, esp
    push dword[ebp+8] ; the 'filename' pointer will exist at base pointer (ebp) +4 (go past ebp that we pushed) +4 (go past pushed return address)
    mov eax, 1 ; Command print
    int 0x80
    add esp, 4 ; Restore the stack pointer

    pop ebp
    ret

; command 3 int getkey()
peachos_getkey:
    push ebp
    mov ebp, esp

    mov eax, 3 ; Command getkey
    int 0x80
    ; Due to how C works, eax will contain the return value (from interrupt_handler -> isr80h_handler)
    pop ebp
    ret

; command 2 void peachos_putchar(char c)
peachos_putchar:
    push ebp
    mov ebp, esp

    push dword[ebp+8] ; Variable 'c'
    mov eax, 2 ; Command putchar
    int 0x80
    add esp, 4 ; Restore the stack pointer

    pop ebp
    ret


; Command 4 void* peachos_malloc(size_t size)
peachos_malloc:
    push ebp
    mov ebp, esp

    push dword[ebp+8] ; Variable 'size' that the caller pushed (explanation in 'print' above)
    mov eax, 4 ; Command malloc
    int 0x80
    add esp, 4 ; Restore the stack pointer for the 'size' push we just performed
    ; Due to how C works, eax will contain the return value (from interrupt_handler -> isr80h_handler)
    pop ebp
    ret

; Command 5 void  peachos_free(void* ptr)
peachos_free:
    push ebp
    mov ebp, esp

    push dword[ebp+8] ; Variable 'ptr' that the caller pushed (explanation in 'print' above)
    mov eax, 5 ; Command 5 free
    int 0x80
    add esp, 4 ; Restore the stack pointer for the 'ptr' pushed above

    pop ebp
    ret

; Command 6 void* peachos_process_load_start(const char* filename)
peachos_process_load_start:
    push ebp
    mov ebp, esp

    push dword[ebp+8] ; Variable 'filename'
    mov eax, 6 ; Command 6 process load start
    int 0x80
    ; Execution flow will be cut off here until returning from the loaded process
    add esp, 4
    pop ebp

    ret

; Runs a program with its supplied arguments
; Command 7 int peachos_system(struct command_argument* arguments)
peachos_system:
    push ebp
    mov ebp, esp

    ; We provide the arguments var (contains the program name and links to
    ; the rest of the program arguments) to the system to run the program
    push dword[ebp+8] ; 'arguments' variable
    mov eax, 7 ; Command 7 invoke system command
    int 0x80

    ; Keep in mind that if we return after succesfully running a program,
    ; eax will contain the value 7 rather than the return value of int80h command 7
    ; because we never actually returned from that function due to execution flow change 

    add esp, 4
    pop ebp
    ret

; Gets argc and argv
; Command 8 void peachos_process_get_arguments(struct process_arguments* arguments)
peachos_process_get_arguments:
    push ebp
    mov ebp, esp
    
    ; We provide this routine with an address (of a 'process_arguments' struct)
    ; into which the system command will push argc and argv
    push dword[ebp+8] ; 'arguments' variable
    mov eax, 8 ; Command 8 process get arguments
    int 0x80

    add esp, 4
    pop ebp
    ret