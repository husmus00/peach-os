[BITS 32]

global _start
extern c_start
extern peachos_exit
; extern main

section .asm

; This _start symbol will call c_start() which will call main() which is defined to be at 0x400000
_start:
    push 50
    ; call main
    call c_start

    push 0 ; Exit code
    call peachos_exit ; Handles gracefully exiting the process
    
    ret

; global _start

; _start:
;     ; out 0x92, al ; General protection fault

;     ; push 20
;     ; push -10

;     ; ; Stack values will be retrieved and summed by the kernel command function
;     ; ; Remember that each push pushes 4 bytes to the stack (32 bits)

;     ; mov eax, 0 ; Command 0 sum
;     ; int 0x80

;     ; add esp, 8 ; Restore the stack to before the above pushes (using two pops also works)

;     ; Result will be in eax (should be 50)

;     ; Now to test the print function

;     push intro
;     mov eax, 1
;     int 0x80
;     add esp, 4

; getkey_loop:
;     call getkey

;     push eax ; Push the obtained key
;     mov eax, 2
;     int 0x80
;     add esp, 4

;     jmp getkey_loop

; getkey:
;     mov eax, 3 ; Command getkey
;     int 0x80

;     cmp eax, 0x00
;     je getkey ; Keep looping until a key is pressed

;     ret

; section .data
; intro: db "Running blank.elf", 0
; message: db "A key was detected by blank.asm"
; message: db 'BLANK, this actually works surprisingly enough.', 0 ; To help locate this program in the os.bin file