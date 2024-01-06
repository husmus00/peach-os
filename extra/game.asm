ORG 0
BITS 16

;
; Initialise
;

read_sector:
    mov ah, 2 ; Read sector command
    mov al, 1 ; Number of sectors to read
    mov ch, 0 ; Cylinder low eight bits
    mov cl, 2 ; Sector number
    mov dh, 0 ; Head number
    mov bx, buffer
    int 0x13

    jc .read_error

    jmp game_loop

.read_error:
    mov si, read_error_message
    call print_string
    jmp $
                   
; ; Set video mode
; mov AL, 03h
; mov AH, 0
; int 10h

; ; Set cursor position
; mov DH, 1
; mov DL, 1
; mov BH, 0
; mov AH, 2
; int 10h

; ; Set attribute to white foreground, black background
; mov BL, 0Fh

;
; ----------
;

game_loop:
    ; Main loop body
    call print_grid
    call print_current_player
    call print_message
    call get_input
    call set_chosen_position
    call check_victory
    
    ; Check if any player has won (player_has_won variable will be 1)
    cmp byte[player_has_won], 1
    je player_won
    
    ; Switching current player
    cmp byte[current_player], 0 ; 0 if X, 1 if O
    je is_x_gl
    
    ; is_o_gl:
        mov byte[current_player], 0     
        jmp game_loop
    
    is_x_gl:
        mov byte[current_player], 1
        jmp game_loop

; Sets the correct attribute and prints the character
; print_char:
                                                         
;     mov AH, 09h ; Set interrupt to print
;     mov CX, 1 ; So that the character is only printed once
    
;     ; Set the correct attribute
;     cmp AL, "X"
;     je is_x_pc
;     cmp AL, "O"
;     je is_o_pc
;     ; is something other than X or O
;     ; Set attribute to white foreground, black background
;     mov BL, 0Fh
;     jmp print_char_interrupt
    
;     is_x_pc:
;         mov BL, 09h ; 9h is the light blue attribute
;         jmp print_char_interrupt
        
;     is_o_pc:
;         mov BL, 0Ch ; Ch is the light red attribute
    
;     print_char_interrupt:
;     int 10h
    
;     inc DL ; Incrementing the cursor position horizontally
;     mov AH, 2h ; Set interrupt to set cursor position
;     int 10h
      
;     ret

print_char:
    mov ah, 0eh ; Print code
    int 0x10
    ret

; carriage_return:
;     inc DH ; Incrementing the cursor position vertically
;     mov DL, 1 ; Resetting horizontal cursor position
;     mov AH, 2h ; Set interrupt to set cursor position
;     int 10h
    
;     ret

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
    mov al, 13
    call print_char
    ; Line feed   
    mov al, 10
    call print_char
    ret

; Prints the grid to the screen
print_grid:
    
    ; Set video mode to clear screen
    mov AL, 03h
    mov AH, 0
    int 10h
    
    ; reset cursor position
    ; mov DL, 1
    ; mov DH, 1
    ; mov AH, 2h ; Set interrupt to set cursor position
    ; int 10h
    
    ; Row 1
    
    mov AL, pos[0]
    call print_char
    
    mov AL, '|'
    call print_char
    
    mov AL, pos[1]
    call print_char
    
    mov AL, '|'
    call print_char
    
    mov AL, pos[2]
    call print_char
    
    call new_line
    
    ; Row 2
    
    mov AL, '-'
    call print_char
    
    mov AL, '+'
    call print_char
    
    mov AL, '-'
    call print_char
    
    mov AL, '+'
    call print_char
    
    mov AL, '-'
    call print_char
    
    call new_line
    
    ; Row 3
    
    mov AL, pos[3]
    call print_char
    
    mov AL, '|'
    call print_char
    
    mov AL, pos[4]
    call print_char
    
    mov AL, '|'
    call print_char
    
    mov AL, pos[5]
    call print_char
    
    call new_line
    
    ; Row 4
    
    mov AL, '-'
    call print_char
    
    mov AL, '+'
    call print_char
    
    mov AL, '-'
    call print_char
    
    mov AL, '+'
    call print_char
    
    mov AL, '-'
    call print_char
    
    call new_line
    
    ; Row 5
    
    mov AL, pos[6]
    call print_char
    
    mov AL, '|'
    call print_char
    
    mov AL, pos[7]
    call print_char
    
    mov AL, '|'
    call print_char
    
    mov AL, pos[8]
    call print_char
    
    call new_line
    call new_line
    
    ret
                         
; Print X or O with styling depending on turn                         
; print_current_player:
;     mov AH, 09h
    
;     cmp byte[current_player], 0 ; 0 if X, 1 if O
;     je is_x
    
;     ; is O
;         mov AL, "O"
;         mov BL, 0Ch ; C is the light red colour attribute
;         jmp end_pcp
    
;     is_x:
;         mov AL, "X"
;         mov BL, 09h ; 9 is the light blue colour attribute
    
;     end_pcp:
;         int 10h
        
;         inc DL ; Incrementing the cursor position horizontally
;         mov AH, 2h ; Set interrupt to set cursor position
;         int 10h
        
;         ret

print_current_player:
    cmp byte[current_player], 0 ; 0 if X, 1 if O
    je is_x

    is_o:
        mov al, "O"
        jmp end_print_current_player

    is_x:
        mov al, "X"

    end_print_current_player:
        call print_char

    ret


print_message:
    ; BL will be used to as the index into the string
    ; mov BX, 0
    
    ; text_loop:
    ;     mov AH, 0Ah ; Write character without attribute
    ;     mov AL, message[BX]
    ;     cmp AL, "$" ; Check for sentinel
    ;     je end_text_loop ; End if sentinel is found
    ;     int 10h
    ;     inc BX
        
    ;     inc DL ; Incrementing the cursor position horizontally
    ;     mov AH, 2h ; Set interrupt to set cursor position
    ;     int 10h
        
    ;     jmp text_loop
        
    ; end_text_loop:
    ;     call new_line
    ;     ret
    mov si, message
    call print_string
    ret

get_input:
    mov AH, 1 ; DOS interrupt to get character from keyboard
    int 0x21
    
    ; AL will contain the inputted character
    
    ret

        
set_chosen_position:
    ; AL will contain the inputted value
    ; But we must subtract 31h to convert the ASCII input into a numerical value between 1 and 9, (31h and not 30h to account for 0-index)
    ; (assuming that the inputted value was between 1 and 9)
    
    SUB AL, 31h
    mov BL, AL ; Because we must use BX to index pos
    mov BH, 0
    
    cmp byte[current_player], 0
    je is_x_scp
    
    ; is_O
        mov byte pos[BX], "O"
        jmp end_scp
    
    is_x_scp:
        mov byte pos[BX], "X"
    
    end_scp:
        ret


check_victory:
    ; First, we load the current player's letter (X or O) into AL.
    ; Then, for each row, column or diagonal to check, compare each of the 3 positions with AL.
    ; If all the comparisons are true, then we have a winner. This will be based on the current player since, if player X plays and
    ; a victory condition is found, it must have been X
    ; If the first position is a space " ", abort immediately since the line can't be a victory condition
    
    ; First, load AL with X or O depending on current turn
    cmp byte[current_player], 0
    je is_x_cv
    
    ; is_o_cv
        mov AL, "O"
        jmp continue_check_victory
        
    is_x_cv:
        mov AL, "X"
    
    continue_check_victory:
    
    ; Row 1, 2, 3
    cmp byte pos[0], " "
    je continue_456 ; Check for " " empty position
    
    cmp AL, pos[0]
    jne continue_456
    cmp AL, pos[1]
    jne continue_456
    cmp AL, pos[2]
    jne continue_456   
    jmp victory
    
    ; Row 4, 5, 6
    continue_456:
    cmp byte pos[3], " "
    je continue_789 ; Check for " " empty position
    
    cmp AL, pos[3]
    jne continue_789
    cmp AL, pos[4]
    jne continue_789
    cmp AL, pos[5]
    jne continue_789    
    jmp victory
    
times 510-($-$$) db 0
dw 0xAA55

buffer:

    ; Row 7, 8, 9
    continue_789:
    cmp byte pos[6], " "
    je continue_147 ; Check for " " empty position
    
    cmp AL, pos[6]
    jne continue_147
    cmp AL, pos[7]
    jne continue_147
    cmp AL, pos[8]
    jne continue_147    
    jmp victory
    
    ; Column 1, 4, 7
    continue_147:
    cmp byte pos[0], " "
    je continue_258 ; Check for " " empty position
    
    cmp AL, pos[0]
    jne continue_258
    cmp AL, pos[3]
    jne continue_258
    cmp AL, pos[6]
    jne continue_258    
    jmp victory
    
    ; Column 2, 5, 8
    continue_258:
    cmp byte pos[1], " "
    je continue_369 ; Check for " " empty position
    
    cmp AL, pos[1]
    jne continue_369
    cmp AL, pos[4]
    jne continue_369
    cmp AL, pos[7]
    jne continue_369    
    jmp victory
    
    ; Column 3, 6, 9
    continue_369:
    cmp byte pos[2], " "
    je continue_159 ; Check for " " empty position
    
    cmp AL, pos[2]
    jne continue_159
    cmp AL, pos[5]
    jne continue_159
    cmp AL, pos[8]
    jne continue_159    
    jmp victory
    
    ; Diagonal 1, 5, 9
    continue_159:
    cmp byte pos[0], " "
    je continue_357 ; Check for " " empty position
    
    cmp AL, pos[0]
    jne continue_357
    cmp AL, pos[4]
    jne continue_357
    cmp AL, pos[8]
    jne continue_357    
    jmp victory
    
    ; Diagonal 3, 5, 7
    continue_357:
    cmp byte pos[2], " "
    je no_victory ; Check for " " empty position
    
    cmp AL, pos[2]
    jne no_victory
    cmp AL, pos[4]
    jne no_victory
    cmp AL, pos[6]
    jne no_victory    
    jmp victory
    
    victory:
    mov byte[player_has_won], 1
    
    no_victory:
    ret

print_victory_message:
    ; BL will be used to as the index into the string
    ; mov BX, 0
    
    ; victory_text_loop:
    ;     mov AH, 0Ah ; Write character without attribute
    ;     mov AL, victory_message[BX]
    ;     cmp AL, "$" ; Check for sentinel
    ;     je end_victory_text_loop ; End if sentinel is found
    ;     int 10h
    ;     inc BX
        
    ;     inc DL ; Incrementing the cursor position horizontally
    ;     mov AH, 2h ; Set interrupt to set cursor position
    ;     int 10h
        
    ;     jmp victory_text_loop
        
    ; end_victory_text_loop:
    ;     call new_line
    ;     ret
    mov si, victory_message
    call print_string
    ret


player_won:
    call print_grid
    call print_current_player
    call print_victory_message

end_program:
    jmp $

; 20H is the space character, duplicate 20H 9 times (for 9 positions)  
pos db 9 DUP(0x20) ; Array for the 9 grid positions

message db ", choose position", 0 ; $ is the terminating character (sentinel character)

current_player db 0 ; 0 for X and 1 for O

player_has_won db 0 ; 0 if a player has not yet won, 1 if otherwise

victory_message db " has won!", 0 ; $ is the terminating character (sentinel character)

read_error_message db 'Error reading file', 0

times 1022-($-$$) db 0