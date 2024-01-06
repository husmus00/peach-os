#if !defined(HEAP_H)
#define HEAP_H

#include "config.h" // The global config table
#include <stdint.h>
#include <stddef.h>


// First 4 bits of the HBT entry (taken status)
#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0x00

#define HEAP_BLOCK_HAS_NEXT 0b10000000
#define HEAP_BLOCK_IS_FIRST 0b01000000

// Represents a table entry (8 bits)
typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct heap_table {
    HEAP_BLOCK_TABLE_ENTRY* entries; // Beginning of the table
    size_t total;                    // Number of entries
};

struct heap {
    struct heap_table* table; // Pointer to the heap table
    void* saddr;              // Start address of the heap
};

int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table);
void* heap_malloc(struct heap* heap, size_t size);
void heap_free(struct heap* heap, void* ptr);

#endif // HEAP_H
