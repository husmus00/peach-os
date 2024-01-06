#include "heap.h"
#include "kernel.h"
#include "memory/memory.h"
#include "status.h"
#include <stdbool.h>

// Ensure that the ptr and end addresses are valid for the table provided
// (Ensure that the heap calculations via the provided values were valid)
// (e.g., ensure that there are indeed 100MB of entries for a 100MB heap)
static bool heap_validate_table(void* ptr, void* end, struct heap_table* table) {
    int res = 0;

    size_t table_size = (size_t)(end - ptr);
    size_t total_blocks = table_size / PEACHOS_HEAP_BLOCK_SIZE;

    if (table->total != total_blocks) {
        res = -EINVARG;
        goto out;
    }

out:
    return res;
}

// Ensure that the beginning and end pointers of the heap are aligned
static bool heap_validate_alignment(void* ptr) {
    return ((unsigned int) ptr % PEACHOS_HEAP_BLOCK_SIZE) == 0;
}

int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table) {
    int res = 0; // No error

    if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end))  {
        res = -EINVARG;
        goto out;
    }

    memset(heap, 0, sizeof(struct heap));
    heap->saddr = ptr;
    heap->table = table;

    res = heap_validate_table(ptr, end, table);
    if (res < 0) {
        goto out;
    }

    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

out:
    return res; 
}

// Return the size to allocate rounded up to the nearest valid block size
static uint32_t heap_align_value_to_upper(uint32_t val) {
    if (val % PEACHOS_HEAP_BLOCK_SIZE == 0) {
        return val;
    }

    val = (val - (val % PEACHOS_HEAP_BLOCK_SIZE));
    val += PEACHOS_HEAP_BLOCK_SIZE;
    return val;
}

int heap_get_start_block(struct heap* heap, uint32_t total_blocks) {
    struct heap_table* heap_table = heap->table;
    
    int start_block = -1;
    int free_blocks = 0;

    for (int i = 0; i < heap_table->total; i++) {
        char entry_status = heap_table->entries[i] & 0x0F;
        if (entry_status != HEAP_BLOCK_TABLE_ENTRY_FREE) {
            start_block = -1;
            continue;
        }

        if (start_block == -1) {
            // This is the first block
            start_block = i;
        }

        free_blocks++;
        if (free_blocks == total_blocks) {
            break;
        }
    }

    if (start_block == -1) {
        return -ENOMEM;
    }

    return start_block;
}

// Converts a heap block index into a real address
void* heap_block_to_address(struct heap* heap, int block) {
    void* block_addr = heap->saddr + (block * PEACHOS_HEAP_BLOCK_SIZE);
    return block_addr;
}

int heap_address_to_block(struct heap* heap, void* address) {
    int block = ((int) (address - heap->saddr)) / PEACHOS_HEAP_BLOCK_SIZE;
    return block;
}

void heap_mark_blocks_free(struct heap* heap, int starting_block) {
    for (int i = starting_block; i < (int)heap->table->total; i++)
    {
        HEAP_BLOCK_TABLE_ENTRY entry = heap->table->entries[i];
       heap->table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;

       if (!(entry & HEAP_BLOCK_HAS_NEXT)) {
            break;
       }
    }
}

void heap_mark_blocks_taken(struct heap* heap, uint32_t start_block, uint32_t total_blocks) {
    int end_block = (start_block + total_blocks) - 1;

    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
    if (total_blocks > 1) {
        entry = entry | HEAP_BLOCK_HAS_NEXT;
    }
    for (int i = start_block; i <= end_block; i++)
    {
        heap->table->entries[i] = entry;
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        if (i != end_block - 1) {
            // For the next iteration
            entry = entry | HEAP_BLOCK_HAS_NEXT;
        }
    }
    
}

void* heap_malloc_blocks(struct heap* heap, uint32_t total_blocks) {
    void* allocated_address = 0;

    // The entry number of first of the available blocks
    int start_block = heap_get_start_block(heap, total_blocks);
    if (start_block < 0) {
        // start_block == -ENOMEM
        goto out; // Function will return allocated_address 0
    }

    // Convert the entry number into a real address
    allocated_address = (void*) heap_block_to_address(heap, start_block);

    // Mark the blocks as taken
    heap_mark_blocks_taken(heap, start_block, total_blocks);
    
out:
    return allocated_address;
}

void* heap_malloc(struct heap* heap, size_t size) {
    size_t aligned_size = heap_align_value_to_upper(size);
    uint32_t total_blocks = aligned_size / PEACHOS_HEAP_BLOCK_SIZE;
    return heap_malloc_blocks(heap, total_blocks);
}

void heap_free(struct heap* heap, void* ptr) {
    heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));
}