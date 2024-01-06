#include "paging.h"
#include "kernel.h"
#include "status.h"
#include "memory/heap/kernelheap.h"

// For loading the directory via assembly (Called from paging_switch function)
void paging_load_directory(uint32_t* directory);
void enable_paging_asm();

static uint32_t* current_directory = 0;

struct paging_4gb_chunk* paging_new_4gb(uint8_t flags) {
    // Array of 1024 page *directory* entries
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++) {
        // Each of these 1024 page table entries contains 1024 real addresses 
        uint32_t* entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        for (int b = 0; b < PAGING_TOTAL_ENTRIES_PER_TABLE; b++) {
            entry[b] = (offset + (b * PAGING_PAGE_SIZE)) | flags;
        }
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
        directory[i] = (uint32_t) entry | flags | PAGING_IS_WRITEABLE;
        // We want the actual directory entries to be writeable by default,
        // but not the individual page entries. Those will be set individually
    }
    struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
}

void paging_free_4gb(struct paging_4gb_chunk* chunk) {
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++) {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t* table = (uint32_t*)(entry & 0xFFFFF000); // To remove the flags on the lower 12 bits
        kfree(table);
    }
    kfree(chunk->directory_entry);
    kfree(chunk);
}

// Switch into the supplied directory('s pointer)
void paging_switch(struct paging_4gb_chunk* directory) {
    paging_load_directory(directory->directory_entry);
    current_directory = directory->directory_entry;
}

uint32_t* paging_4gb_chunk_get_directory(struct paging_4gb_chunk* chunk) {
    return chunk->directory_entry;
}

void enable_paging() {
    enable_paging_asm();
    println("Enabled paging");
}

bool paging_is_aligned(void* addr) {
    return ((uint32_t)addr % PAGING_PAGE_SIZE) == 0;
}

// Returns an aligned address (aligned above the supplied address to the next page)
void* paging_align_address(void* ptr) {
    // Check if already aligned
    if((uint32_t)ptr % PAGING_PAGE_SIZE) {
        // True if not aligned
        return (void*)((uint32_t)ptr + PAGING_PAGE_SIZE - ((uint32_t)ptr % PAGING_PAGE_SIZE));
    }
    // Otherwise, if already aligned just return ptr
    return ptr;
}

// Return the address aligned down to a lower aligned address;
// E.g., 4098 will get aligned down to 4096
void* paging_align_to_lower_page(void* ptr) {
    uint32_t _addr = (uint32_t) ptr;
    _addr -= (_addr % PAGING_PAGE_SIZE);
    return (void*) _addr;   
}

// Gets the page directory/table indexes for the supplied virtual address
// These indexes are supplied to the caller via the index_out arguments
int paging_get_indexes(void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out) {
    int res = 0;
    if (!paging_is_aligned(virtual_address)) {
        res = -EINVARG;
        goto out;
    }

    // Gets the directory index by dividing by the 4MB range each directory entry covers (corresponding to an entire page table)
    *directory_index_out = ((uint32_t)virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    // Gets the page table index by further dividing the 4MB page table range into 4096 byte entries
    *table_index_out = (uint32_t)virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE;
out:
    return res;
}

int paging_map(struct paging_4gb_chunk* directory, void* virt, void* phys, int flags) {
    // Check that both physical and virtual address are aligned
    if (((unsigned int)virt % PAGING_PAGE_SIZE) || (unsigned int)phys % PAGING_PAGE_SIZE) {
        return -EINVARG;
    }

    return paging_set(directory->directory_entry, virt, (uint32_t)phys | flags);
}

// Map 'total_pages' number of pages from 'phys' to 'virt'
int paging_map_range(struct paging_4gb_chunk* directory, void* virt, void* phys, int page_count, int flags) {
    int res = 0;

    for (int i = 0; i < page_count; i++) {
        // Maps an individual page
        res = paging_map(directory, virt, phys, flags);
        if (res < 0)
            break;
        virt += PAGING_PAGE_SIZE;
        phys += PAGING_PAGE_SIZE;
    }

    return res;
}

// Map the provided real memory to the provided virtual memory using the provided directory
// Checks parameters for errors then calls paging_map_range() to actually map the memory
int paging_map_to(struct paging_4gb_chunk* directory, void* virt, void* phys, void* phys_end, int flags) {
    int res = 0;

    // Check virtual address is aligned
    if ((uint32_t)virt % PAGING_PAGE_SIZE) {
        res = -EINVARG;
        goto out;
    }
    // Check physical address is aligned
    if ((uint32_t)phys % PAGING_PAGE_SIZE) {
        res = -EINVARG;
        goto out;
    }
    // Check physical end address is aligned
    if ((uint32_t)phys_end % PAGING_PAGE_SIZE) {
        res = -EINVARG;
        goto out;
    }

    // Check that phys_end is after phys
    if ((uint32_t)phys_end < (uint32_t)phys) {
        res = -EINVARG;
        goto out;
    }

    uint32_t total_bytes = phys_end - phys;
    int total_pages = total_bytes / PAGING_PAGE_SIZE;
    res = paging_map_range(directory, virt, phys, total_pages, flags);

out:
    return res;
}

// Set the value of the designated page (at the virtual address) to val (val contains real address + flags)
int paging_set(uint32_t* directory, void* virtual_address, uint32_t val) {
    if (!paging_is_aligned(virtual_address)) {
        return -EINVARG;
    }

    uint32_t page_directory_index = 0;
    uint32_t page_table_index = 0;
    int res = paging_get_indexes(virtual_address, &page_directory_index, &page_table_index);

    if (res < 0) {
        return res;
    }

    // Get the directory entry
    uint32_t entry = directory[page_directory_index];
    // Get the real address of the desired page table (array) for direct access
    // Each table is 4096 bytes apart from one another
    uint32_t* table = (uint32_t*)(entry & 0xfffff000); // Last 3 bits are just the flags
    // Set the page table entry value
    table[page_table_index] = val; 
    
    return res;
}

// Return the directory entry (physical address with flags removed) for the given virtual address
uint32_t paging_get(uint32_t* directory, void* virt) {
    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    paging_get_indexes(virt, &directory_index, &table_index);
    uint32_t entry = directory[directory_index];
    uint32_t* table = (uint32_t*)(entry & 0xfffff000);
    return table[table_index];
}

// Get the physical address for the given virtual address within the given page directory
void* paging_get_physical_address(uint32_t* directory, void* virt) {
    // First, align the supplied virtual address (because paging_get() will only work with an aligned address)
    void* aligned_virt_addr = (void*) paging_align_to_lower_page(virt);
    // Then calculate the difference in the addresses which will be re-added after getting the physical address
    void* difference = (void*)((uint32_t) virt - (uint32_t) aligned_virt_addr);
    // Now get the aligned physical address (ignoring the flags), add the difference, and return
    return (void*)((paging_get(directory, aligned_virt_addr) & 0xfffff000) + difference);
}


