#include <stdbool.h>
#include "elfloader.h"
#include "fs/file.h"
#include "status.h"
#include "memory/memory.h"
#include "memory/heap/kernelheap.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "kernel.h"
#include "config.h"

// Valid signature the loader expects
const char elf_signature[] = {0x7f, 'E', 'L', 'F'};

// Compare the supplied signature to the valid one
static bool elf_valid_signature(void* buffer) {
    return memcmp(buffer, (void*)elf_signature, sizeof(elf_signature)) == 0;
}

// Check if supplied ELF file is 32 bit or NONE (64 bit is not supported)
static bool elf_valid_class(struct elf_header* header) {
    return header->e_ident[EI_CLASS] == ELFCLASSNONE || header->e_ident[EI_CLASS] == ELFCLASS32;
}

// Check if supplied ELF file has a little endian format (default on INTEL machines) or NONE
static bool elf_valid_encoding(struct elf_header* header) {
    return header->e_ident[EI_DATA] == ELFDATANONE || header->e_ident[EI_DATA] == ELFDATA2LSB;
}

// Check if supplied ELF file is executable (we don't currently support shared libraries)
// Also check that entry point is appropriate (at least 0x400000)
static bool elf_is_executable(struct elf_header* header) {
    return header->e_type == ET_EXEC && header->e_entry >= PEACHOS_PROGRAM_VIRTUAL_ADDRESS;
}

// Check if supplied ELF file has a program header
static bool elf_has_program_header(struct elf_header* header) {
    return header->e_phoff != 0;
}

// Returns the physical address that this elf file is loaded at
void* elf_memory(struct elf_file* file) {
    return file->elf_memory;
}

// Returns the ELF file's header (which is at the start of the ELF file)
struct elf_header* elf_header(struct elf_file* file) {
    return file->elf_memory;
}

// Return the ELF file's section header (location of ELF header itself + section header offset) (start of the sections)
struct elf32_shdr* elf_sheader(struct elf_header* header) {
    return (struct elf32_shdr*)((int)header + header->e_shoff);
}

// Return the ELF file's program header (location of ELF header itself + program header offset) (start of the program headers)
struct elf32_phdr* elf_pheader(struct elf_header* header) {
    // First check that a program header exists
    if (header->e_phoff == 0) {
        return 0;
    }
    return (struct elf32_phdr*)((int)header + header->e_phoff);
}

// Return individual program header based on index
struct elf32_phdr* elf_program_header(struct elf_header* header, int index) {
    return &elf_pheader(header)[index];
}

// Return individual section header based on index
struct elf32_shdr* elf_section_header(struct elf_header* header, int index) {
    return &elf_sheader(header)[index];
}

// Returns address of string table itself (not the address of its section header)
// ELF header (start of file) + section_headers[string_table_header_index]->offset
char* elf_str_table(struct elf_header* header) {
    return (char*) header + elf_section_header(header, header->e_shstrndx)->sh_offset;
}

// Returns virtual base (lowest) address of this file
void* elf_virtual_base(struct elf_file* file) {
    return file->virtual_base_address;
}

// Returns virtual ending (highest) address of the file
void* elf_virtual_end(struct elf_file* file) {
    return file->virtual_end_address;
}

// Returns physical base (lowest) address
void* elf_physical_base(struct elf_file* file) {
    return file->physical_base_address;
}

// Returns physical end address
void* elf_physical_end(struct elf_file* file) {
    return file->physical_end_address;
}

// Returns the physical (in-memory) address of the supplied header's segment
void* elf_phdr_phys_address(struct elf_file* elf_file, struct elf32_phdr* phdr) {
    return elf_memory(elf_file) + phdr->p_offset;
}

// Performs various checks to ensure that supplied ELF file is valid and can be loaded
int elf_validate_loaded(struct elf_header* header) {
    return (elf_valid_signature(header) && elf_valid_class(header) && elf_valid_encoding(header) && elf_has_program_header(header)) ? PEACHOS_ALL_OK : -EINFORMAT;
}

// Process a loadable elf segment. Calculate virtual and physical base addresses from header info
int elf_process_pheader_pt_load(struct elf_file* elf_file, struct elf32_phdr* phdr) {
    // Remember that elf_file is our internal representation for ease of use/abstraction

    // This function is called for each program header
    // So, this check ensures we've set the (virtual and physical) addresses to the lowest address among all the program headers (the base address)
    if (elf_file->virtual_base_address >= (void*)phdr->p_vaddr || elf_file->virtual_base_address == 0x00) {
        // Set the elf_file (internal representation)'s virtual and physical addresses from the actual elf file header
        elf_file->virtual_base_address = (void*)phdr->p_vaddr; 
        elf_file->physical_base_address = elf_memory(elf_file)+phdr->p_offset; // phdr->p_offset contains the offset from the beginning of the elf file to the segment content
    }

    // Perform a similar check for the end (physical and virtual) addresses
    unsigned int end_virtual_address = phdr->p_vaddr + phdr->p_filesz; // End virtual address is (virtual start + size)
    if (elf_file->virtual_end_address <= (void*)(end_virtual_address) || elf_file->virtual_end_address == 0x00) {
        elf_file->virtual_end_address = (void*)end_virtual_address;
        elf_file->physical_end_address = elf_memory(elf_file) + phdr->p_offset + phdr->p_filesz; // Physical end of segment at (elf start + offset to segment + size of segment)
    }

    return 0;
}

// Process an individual program header
// Currently, we only supported loadable (PT_LOAD) segments
int elf_process_pheader(struct elf_file* elf_file, struct elf32_phdr* phdr) {
    int res = 0;
    switch(phdr->p_type) {
        case PT_LOAD:
            res = elf_process_pheader_pt_load(elf_file, phdr);
            break;
        default:
            break; // Do nothing
    }

    return res;
}

// Validate and process the elf file's program headers
int elf_process_pheaders(struct elf_file* elf_file) {
    int res = 0;

    // Loop through all program headers and process each
    struct elf_header* header = elf_header(elf_file); // Address of the start of the elf file in memory
    for (int i = 0; i < header->e_phnum; i++) {
        struct elf32_phdr* phdr = elf_program_header(header, i);
        res = elf_process_pheader(elf_file, phdr);
        if (res < 0) {
            break;
        }
    }

    return res;
}

// Process the loaded elf file, allowing it to run and execute 
int elf_process_loaded(struct elf_file* elf_file) {
    int res = 0;

    // Validate elf file is loaded
    struct elf_header* header = elf_header(elf_file); // Address of the start of the elf file in memory
    res = elf_validate_loaded(header);
    if (res < 0) {
        goto out;
    }

    // Process all program headers
    res = elf_process_pheaders(elf_file);
    if (res < 0) {
        goto out;
    }

out:
    return 0;
}

// Loads an elf file supplied by file name
// Sets the supplied file_out double pointer to the resulting elf file structure's pointer
// This function is called by a process trying to load an elf file
int elf_load(const char* filename, struct elf_file** file_out) {
    
    struct elf_file* elf_file = kzalloc(sizeof(struct elf_file));
    
    // First, successfully open the desired file
    int fd = 0; // File descriptor
    int res = fopen(filename, "r"); // We only need read permission
    if (res <= 0) {
        res = -EIO; // IO Error, fopen failed
        goto out;
    }
    fd = res;

    // Get file information
    struct file_stat stat;
    res = fstat(fd, &stat);
    if (ISERR(res)) {
        goto out;
    }

    // Assign enough space in memory and load file into it
    elf_file->elf_memory = kzalloc(stat.file_size);
    res = fread(elf_file->elf_memory, stat.file_size, 1, fd);
    if (res < 0) {
        goto out;
    }

    // Now
    res = elf_process_loaded(elf_file);
    if (res < 0) {
        goto out;
    }

    // Result out
    *file_out = elf_file;

out:
    fclose(fd);
    return res;
}

// Close elf file by freeing up memory
void elf_close(struct elf_file* elf_file) {
    if (!elf_file)
        return;

    // Free up memory used by elf program and elf_file struct
    kfree(elf_file->elf_memory);
    kfree(elf_file);
}