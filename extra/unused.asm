; CHS method of reading data
read_sector:
    mov ah, 2 ; Read sector command
    mov al, 1 ; Number of sectors to read
    mov ch, 0 ; Cylinder low eight bits
    mov cl, 2 ; Sector number
    mov dh, 0 ; Head number
    mov bx, buffer
    int 0x13

    jc .read_error
    ret

.read_error:
    mov si, read_error_message
    call print_string
    jmp $

print_char:
    mov ah, 0eh ; Print code
    int 0x10
    ret

print_string:
    lodsb ; mov al, [si] ; Set AL to current letter value (not address)
    cmp al, 0  ; Check for end of string
    je .end_print_string
    call print_char
    ; inc si ; Done by lodsb
    jmp print_string
.end_print_string:
    call new_line
    ret

new_line:
    ; Carriage return
    mov ah, 0eh 
    mov al, 13
    int 0x10
    ; Line feed   
    mov al, 10
    int 0x10
    ret

my_word db 'This works!', 0
my_word_2 db 'This works twice!', 0

read_error_message db 'Error reading file', 0