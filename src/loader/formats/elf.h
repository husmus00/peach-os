#if !defined(ELF_H)
#define ELF_H

#include <stdint.h>

// Segment permissions, defines permissions for loadable segments
// (See following section with segment types -> PT_LOAD)
#define PF_X 0x01 // Execute
#define PF_W 0x02 // Write
#define PF_R 0x04 // Read

// Segment types, defined in the program header
#define PT_NULL 0    // Unused/No type
#define PT_LOAD 1    // Loadable
#define PT_DYNAMIC 2 // Dynamic linking
#define PT_INTERP 3  // For calling an interpreter (what is this?)
#define PT_NOTE 4    // Defines auxillary information
#define PT_SHLIB 5   // Reserved segment
#define PT_PHDR 6    // Specifies the location and size of the program header table itself

// Section types (sh_type in section header)
#define SHT_NULL 0     // unused/No type
#define SHT_PROGBITS 1 // Holds information defined by the program (e.g., text and data)
#define SHT_SYMTAB 2   // Symbol table
#define SHT_STRTAB 3   // String table
#define SHT_RELA 4     // Relocatable
#define SHT_HASH 5     // Hash table
#define SHT_DYNAMIC 6  // Dynamic linking
#define SHT_NOTE 7     // Information that marks the file
#define SHT_NOBITS 8   // Occupies no space
#define SHT_REL 9      // Relocatable
#define SHT_SHLIB 10   // Reserved
#define SHT_DYNSYM 11  // Symbol table
#define SHT_LOPROC 12  // This through HIPROC,
#define SHT_HIPROC 13  // ^ These are reserved for processor-specific semantics
#define SHT_LOUSER 14  // Lower bound of indexes reserved for application programs
#define SHT_HIUSER 15  // ^ Upper bound

// ELF file types (e_type entry in ELF header)
#define ET_NONE 0 // No file type
#define ET_REL  1 // Relocatable file
#define ET_EXEC 2 // Executable file
#define ET_DYN  3 // Shared object file
#define ET_CORE 4 // Core dump file

// Size in bytes of the EI_NIDENT (ELF identity) field (e_ident[EI_NIDENT])
// This is an array with each byte representing some machine-independent information
#define EI_NIDENT 16

// Bytes (indexes) of e_ident (ELF identity), each one will have a specific value
#define EI_CLASS 4 // 4th byte, capacity of file (32 or 64 bits), see following section
#define EI_DATA  5 // 5th byte, encoding (or endianess) of file, see 2 sections forward

// EI_CLASS types
#define ELFCLASSNONE 0 // Invalid class
#define ELFCLASS32   1 // 32-bit program
#define ELF_CLASS64  2 // 64-bit program

// EI_DATA types
#define ELFDATANONE 0 // Invalid data encoding
#define ELFDATA2LSB 1 // Least significant bit (stored in reverse)
#define ELFDATA2MSB 2 // Most significant bit 

// Reserved section header indexes, these indexes have special meaning (predefined types of section headers)
#define SHN_UNDEF 0          // Undefined section reference
// #define SHN_LORESERVE 0xff00 // Lower bound of reserved indexes
// #define SHN_LOPROC    0xff00 // This range through HIPROC
// #define SHN_HIPROC    0xff1f // ^ Reserved for processor-specific semantics
// #define SHN_ABS       0xfff1 // Related to absolute references (related to relocation)
// #define SHN_COMMON    0xfff2 // Symbols in this section are common symbols (e.g., unallocated C external variables)
// #define SHN_HIRESERVE 0xffff // Upper bound of reserved indexes

typedef uint16_t elf32_half; // Unsigned medium integer
typedef uint32_t elf32_word;  // Unsigned large integer
typedef int32_t elf32_sword;  // Signed large integer
typedef uint32_t elf32_addr;  // Unsigned program address
typedef uint32_t elf32_off;   // Unsigned file offset

// ELF program header (Each segment is described by one of these)
struct elf32_phdr {
    elf32_word p_type;   // Type of segment
    elf32_off p_offset;  // Offset from start of file to first byte of the segment itself
    elf32_addr p_vaddr;  // Virtual address of first segment byte
    elf32_addr p_paddr;  // Physical address of first byte of segment
    elf32_word p_filesz; // Bytes of "file image of the segment"
    elf32_word p_memsz;  // Bytes of "memory image of the segment"
    elf32_word p_flags;  // Segment flags
    elf32_word p_align;  // TODO: Value to which the segments are aligned in memory (4096 in our case), p_addr = p_offset % p_align
}__attribute__((packed));

// ELF section header (each section is described by one of these)
struct elf32_shdr {
    elf32_word sh_name;      // Name of section (index into string table)
    elf32_word sh_type;      // Type of section (types defined near top of file)
    elf32_word sh_flags;     // 1-bit flags describing miscellaneous attributes
    elf32_addr sh_addr;      // The address at which the first byte of this section will appear
    elf32_off sh_offset;     // Offset of first byte of this section from the beginning of the file
    elf32_word sh_size;      // Section's size in bytes
    elf32_word sh_link;      // Section header table index link (depending on type of section)
    elf32_word sh_info;      // Holds info depending on section type (page 1-14 figure 1-12 in reference)
    elf32_word sh_addralign; // TODO: Value to which the section is aligned to (4096 in our case)
    elf32_word sh_entsize;   // Size in bytes for each entry of section table (for sections that hold a fixed-sized table, such as a symbol table). 0 if no table
    // Number of entries in the section (number of entries sh_offset is pointing to) (if it is a table) is sh_size / sh_entsize
}__attribute__((packed));

// ELF header (the main one at the top of the file)
struct elf_header {
    unsigned char e_ident[EI_NIDENT];
    elf32_half e_type;      // Object file type, types defined above
    elf32_half e_machine;   // Specifies required architecture (e.g., SPARC)
    elf32_word e_version;   // Object file version (ELF version) currently 1
    elf32_addr e_entry;     // Virtual address to which the system first transfers control
    elf32_off e_phoff;      // Offset of program header table in bytes (0 if no program header)
    elf32_off e_shoff;      // Offset of section header table in bytes (0 if no section header)
    elf32_word e_flags;     // Processor-specific flags
    elf32_half e_ehsize;    // Size of ELF header in bytes
    elf32_half e_phentsize; // Size in bytes of each entry in program header table
    elf32_half e_phnum;     // Number of entries in the program header (e_phentsize * e_phnum = size of program header table)
    elf32_half e_shentsize; // Size in bytes of section header (each entry in section header table)
    elf32_half e_shnum;     // Number of entries in the section header (e_shentsize * e_shnum = size of section header table)
    elf32_half e_shstrndx;  // Index of section header associated with the string table
}__attribute__((packed));

// An array of the following struct exists if the object file participates in dynamic linking,
// and the program header table with have an element of type PT_DYNAMIC.
// (The .dynamic section is the array of these structs).
// See page 2-8
struct elf32_dyn
{
    elf32_sword d_tag; // Controls the interpretation of d_un
    union {
        elf32_word d_val;
        elf32_addr d_ptr; // Represents program virtual addresses
    } d_un;
}__attribute__((packed));

// Represents a symbol (function, global variable, etc.)
// The .symtab section will contain an array (table) of these structs
// TODO: (sh_entsize in the section header for .symtab will equal the size of this struct)
struct elf32_sym {
    elf32_word st_name;     // Index into dynamic string table, name of the symbol
    elf32_addr st_value;    // Value of associated symbol (could be absolute value or address to value)
    elf32_word st_size;     // Size of symbol in bytes (but depends on context)
    unsigned char st_info;  // Type and binding attributes of symbol
    unsigned char st_other; // No defined meaning, holds 0
    elf32_half st_shndx;    // Index into section header table, defines the section to which this symbol is related/defined
}__attribute__((packed));

// Function headers
void* elf_get_entry_ptr(struct elf_header* elf_header);
uint32_t elf_get_entry(struct elf_header* elf_header);

#endif // ELF_H
