#include "elf.h"

// Returns virtual address to which the system first transfers control (cast)
void* elf_get_entry_ptr(struct elf_header* elf_header) {
    return (void*) elf_header->e_entry;
}

// Returns value of virtual address to which the system first transfers control (no cast)
uint32_t elf_get_entry(struct elf_header* elf_header) {
    return elf_header->e_entry;
}
