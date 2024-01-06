#if !defined(ELFLOADER_H)
#define ELFLOADER_H

#include <stdint.h>
#include <stddef.h>

#include "elf.h"
#include "config.h"

// Internal representation for ease of use/abstraction
struct elf_file {
    char filename[PEACHOS_MAX_PATH];

    int in_memory_size; // Size of elf file when loaded in memory

    void* elf_memory; // The physical address that this elf file is loaded at

    void* virtual_base_address; // Virtual base (lowest) address of this binary

    void* virtual_end_address; // Virtual ending (highest) address of the binary

    void* physical_base_address; // Physical base (lowest) address

    void* physical_end_address; // Physical ending (highest) address of the binary
};

int elf_load(const char* filename, struct elf_file** file_out);
void elf_close(struct elf_file* elf_file);

void* elf_virtual_base(struct elf_file* file);
void* elf_virtual_end(struct elf_file* file);
void* elf_physical_base(struct elf_file* file);
void* elf_physical_end(struct elf_file* file);
void* elf_phdr_phys_address(struct elf_file* elf_file, struct elf32_phdr* phdr);

void* elf_memory(struct elf_file* file);
struct elf_header* elf_header(struct elf_file* file);
struct elf32_shdr* elf_sheader(struct elf_header* header);
struct elf32_phdr* elf_pheader(struct elf_header* header);
struct elf32_phdr* elf_program_header(struct elf_header* header, int index);
struct elf32_shdr* elf_section_header(struct elf_header* header, int index);

#endif // ELFLOADER_H
