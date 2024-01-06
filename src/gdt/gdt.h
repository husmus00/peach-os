#if !defined(GDT_H)
#define GDT_H

#include <stdint.h>

// This is the structure that actually resembles the kernel gdt,
// functions will convert between this and 'gdt_structured'
struct gdt {
    uint16_t segment;
    uint16_t base_first;
    uint8_t base;
    uint8_t access;
    uint8_t high_flags;
    uint8_t base_24_31_bits;
}__attribute__((packed));  

// This structure is easier to work with (abstraction)
struct gdt_structured {
    uint32_t base;
    uint32_t limit;
    uint8_t type;
};

// We call this function to get the proper 'gdt' struct, which we pass to gdt_load()
void gdt_structured_to_gdt(struct gdt* gdt, struct gdt_structured* structured_gdt, int total_entries);
void gdt_load(struct gdt* gdt, int size);

#endif // GDT_H
