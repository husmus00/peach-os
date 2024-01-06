ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; _start:
jmp short start
nop

; times 33 db 0 ; This was in place of the FAT headers

; FAT16 HEADER
OEMIdentifier       db 'PEACHOS ' ; 8 bytes
BytesPerSector      dw 0x200      ; 512 bytes per sector
SectorsPerCluster   db 0x80       ; 128
ReservedSectors     dw 200        ; 100KB for the kernel
FATCopies           db 0x02
RootDirEntries      dw 0x40       ; 64
NumSectors          dw 0x00
MediaType           db 0xF8
SectorsPerFat       dw 0x100      ; 256 sectors for each table
SectorsPerTrack     dw 0x20
NumberOfHeads       dw 0x40
HiddenSectors       dd 0x00
SectorsBig          dd 0x773594

; Extended BPB (Dos 4.0)

DriveNumber         db 0x80
WinNTBit            db 0x00
Signature           db 0x29
VolumeID            dd 0xD105
VolumeIDString      db 'PEACHOS BOO' ; 11 bytes
SystemIDString      db 'FAT16   '    ; 8 bytes


start:
    jmp 0:init ; Set code segment
init:
    ; Initialisation
    cli ; Disable Interrupts 
    ; Set Data and Extra segments
    mov ax, 0
    mov ds, ax
    mov es, ax
    ; Set stack pointers
    mov ss, ax
    mov sp, 0
    sti ; Enable interrupts

; Load into protected mode
.load_protected:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32

;; GDT (GLOBAL DESCRIPTOR TABLE) DATA
gdt_start:
gdt_null: ; Null segment 64 bits
    dd 0x0
    dd 0x0

; offset 0x8
gdt_code:        ; CS SHOULD POINT TO THIS
    dw 0xffff    ; Segment limit first 0-15 bits
    dw 0         ; Base first 0-15 bits
    db 0         ; Base 16-23 bits
    db 0x9a      ; Access byte
    db 11001111b ; High 4 bit flags and low 4 bit flags
    db 0         ; Base 24-31 bits

; offset 0x10
gdt_data:        ; DS, SS, ES, FS, GS
    dw 0xffff    ; Segment limit first 0-15 bits
    dw 0         ; Base first 0-15 bits
    db 0         ; Base 16-23 bits
    db 0x92      ; Access byte
    db 11001111b ; High 4 bit flags and low 4 bit flags
    db 0         ; Base 24-31 bits

gdt_end:

gdt_descriptor: 
    dw gdt_end - gdt_start - 1 ; Size of the descriptor
    dd gdt_start


[BITS 32]
load32:
    mov eax, 1
    mov ecx, 100
    mov edi, 0x0100000
    call ata_lba_read
    jmp CODE_SEG:0x0100000

ata_lba_read:
    mov ebx, eax ; Backup the LBA
    ; Send the highest 8 bits of the lba to hard disk controller
    shr eax, 24 ; Only the highest 8 bits will remain in eax
    or eax, 0xE0 ; Select the master drive
    mov dx, 0x1F6
    out dx, al ; al is the first 8 bits of eax
    ; Finished sending the highest 8 bits of the lba
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; Finished sending the total sectors to read

    ; Send more bits of the LBA
    mov eax, ebx ; Restore the backup LBA
    mov dx, 0x1F3
    out dx, al
    ; Finished sending more bits of the LBA

    ; Send more bits of the LBA
    mov dx, 0x1F4
    mov eax, ebx ; Restore the backup lBA
    shr eax, 8
    out dx, al
    ; Finished sending more bits of the LBA

    ; Send UPPER 16 bits of the LBA
    mov dx, 0x1F5
    mov eax, ebx ; Restore the backup LBA
    shr eax, 16
    out dx, al
    ; Finished sending UPPER 16 bits of the LBA

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

    ; Read all sectors into memory
.next_sector:
    push ecx

; Checking if we need to read:
.try_again:
    mov dx, 0x1F7
    in al, dx
    test al, 8
    jz .try_again

; We need to read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw

    ; Restore the number of sectors, then loop (decrement ecx) until 0
    pop ecx
    loop .next_sector
    ; End of reading sectors into memory
    ret


times 510-($-$$) db 0

dw 0xAA55