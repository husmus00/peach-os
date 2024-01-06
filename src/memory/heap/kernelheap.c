#include "kernelheap.h"
#include "heap.h"
#include "config.h"
#include "kernel.h"
#include "status.h"
#include "memory/memory.h"

// The heap and heap_table structs are defined in heap.h/c
struct heap kernel_heap;
struct heap_table kernel_heap_table;

void kheap_init() {
    // 104857800 bytes or 100MB of heap space will be allocated
    // 104857800 / 4096 = 25600 total entries
    // Above numbers defined in config.h

    int total_table_entries = PEACHOS_HEAP_SIZE_BYTES / PEACHOS_HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)(PEACHOS_HEAP_TABLE_ADDRESS);
    kernel_heap_table.total = total_table_entries;

    void* end = (void*)(PEACHOS_HEAP_ADDRESS + PEACHOS_HEAP_SIZE_BYTES);

    int response = heap_create(&kernel_heap, (void*)(PEACHOS_HEAP_ADDRESS), end, &kernel_heap_table);
    if (response < 0) {
        panic("Failed to create heap");
    }

    println("Initialised heap");
}

void* kmalloc(size_t size) {
    void* ptr = heap_malloc(&kernel_heap, size);
    if (ptr == 0) {
        panic("kmalloc: Ran out of memory");
    }
    return ptr;
}

// malloc and set all to zero
void* kzalloc(size_t size) {
    void* ptr = kmalloc(size);
    if (!ptr) {
        return 0;
    }
    memset(ptr, 0x00, size);
    return ptr;
}

void kfree(void* ptr) {
    heap_free(&kernel_heap, ptr);
}