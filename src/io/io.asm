section .asm

global insb
global insw

global outb
global outw

; ins byte
insb:
    push ebp
    mov ebp, esp

    xor eax, eax ; zero the eax register
    mov edx, [ebp + 8] ; Move the 'port' parameter into dx
    in al, dx

    ; When C and assembly interoperate, the eax register contains the return value
    ; So, al will contain the return byte for the insb function in C 

    pop ebp
    ret

; ins word
insw:
    push ebp
    mov ebp, esp

    xor eax, eax ; zero the eax register
    mov edx, [ebp + 8] ; Move the 'port' parameter into dx
    in ax, dx

    ; When C and assembly interoperate, the eax register contains the return value
    ; So, ax will contain the return word for the insw function in C 

    pop ebp
    ret

; out byte
outb:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp+8]  ; Move the 'port' parameter into dx
    mov eax, [ebp+12] ; Move the 'val' parameter into al
    out dx, al 

    ; We don't return any value to the calling C function

    pop ebp
    ret

outw:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp+8]  ; Move the 'port' parameter into dx
    mov eax, [ebp+12] ; Move the 'val' parameter into ax
    out dx, ax

    ; We don't return any value to the calling C function

    pop ebp
    ret
