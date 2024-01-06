#include "process.h"
#include "config.h"
#include "memory/memory.h"
#include "status.h"
#include "task/task.h"
#include "memory/heap/kernelheap.h"
#include "fs/file.h"
#include "string/string.h"
#include "kernel.h"
#include "memory/paging/paging.h"
#include "loader/formats/elfloader.h"
#include "string/string.h"

// The current running (active) process (which we're 'looking' at)
// This is in contrast to the current_task which is what is currently executing (perhaps in the background)
struct process* current_process = 0;

// List of active processes (index is equivalent to ID)
static struct process* processes[PEACHOS_MAX_PROCESSES] = {};

static void process_init(struct process* process) {
    memset(process, 0, sizeof(struct process));
}

struct process* process_current() {
    return current_process;
}

struct process* process_get(int process_id) {
    if (process_id < 0 || process_id >= PEACHOS_MAX_PROCESSES) {
        return NULL;
    }

    return processes[process_id];
}

// Switches the current process
int process_switch(struct process* process) {
    current_process = process;
    return 0;
}


// Loops through a process' allocations structs and checks for an empty allocation slot
static int process_find_free_allocation_index(struct process* process) {
    int res = -ENOMEM;

    for (int i = 0; i < PEACHOS_MAX_PROGRAM_ALLOCATIONS; i++) {
        if (process->allocations[i].ptr == 0) {
            res = i;
            break;
        }
    }

    if (res == -ENOMEM) {
        println("Error");
    }

    return res;
}


// Allocate memory for a process
void* process_malloc(struct process* process, size_t size) {
    void* ptr = kzalloc(size);
    if (!ptr) {
        // Failed to allocate memory
        goto out_error;
    }

    int index = process_find_free_allocation_index(process);
    if (ISERR(index)) {
        panic("process_malloc: Allocation not successful");
        goto out_error;
    }

    // If no error occured: 
    // 1. Map the pointer with the correct flags
    int res = paging_map_to(process->task->page_directory, ptr, ptr, paging_align_address(ptr + size), PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (res < 0)
        goto out_error;
    // 2. Add allocation to the process struct's 'process_allocation' struct
    process->allocations[index].ptr = ptr;
    process->allocations[index].size = size;
    // 3. Track active allocations
    process->active_allocations++;

    return ptr;

out_error:
    if (ptr)
        kfree(ptr);
    return 0;
}

bool process_ptr_belongs_to_process(struct process* process, void* ptr) {
    for (int i = 0; i < PEACHOS_MAX_PROGRAM_ALLOCATIONS; i++) {
        if (process->allocations[i].ptr == ptr)
            return true;
    }

    return false;
}

static void process_allocation_unjoin(struct process* process, void* ptr) {
    for (int i = 0; i < PEACHOS_MAX_PROGRAM_ALLOCATIONS; i++) {
        
        if (process->allocations[i].ptr == ptr) 
        {
            process->allocations[i].ptr = 0x00;
            process->allocations[i].size = 0x00;
        }
    }
}

static struct process_allocation* process_get_allocation_by_addr(struct process* process, void* addr) {
    for (int i = 0; i < PEACHOS_MAX_PROGRAM_ALLOCATIONS; i++) {
        if (process->allocations[i].ptr == addr) {
            return &process->allocations[i];
        }
    }

    return 0;
}

// Frees all of a process' allocations
int process_terminate_allocations(struct process* process) {
    for (int i = 0; i < PEACHOS_MAX_PROGRAM_ALLOCATIONS; i++) {
        // TODO: use process_free() if we need to do anything special upon freeing the allocations
        kfree(process->allocations[i].ptr);
    }

    return 0;
}

int process_terminate_binary_data(struct process* process) {
    kfree(process->ptr);
    return 0;
}

int process_terminate_elf_data(struct process* process) {
    elf_close(process->elf_file);
    return 0;
}

int process_terminate_program_data(struct process* process) {
    int res = 0;

    switch(process->filetype) {
        case PROCESS_FILETYPE_BINARY:
            res = process_terminate_binary_data(process);
            break;
        case PROCESS_FILETYPE_ELF:
            res = process_terminate_elf_data(process);
            break;
        default:
            res = -EINVARG;
    }

    return res;
}

void process_switch_to_any() {
    for (int i = 0; i < PEACHOS_MAX_PROCESSES; i++) {
        if (processes[i]) {
            process_switch(processes[i]);
            return;
        }
    }

    panic("process_switch_to_any(): No processes to switch to");
}

static void process_unlink(struct process* process) {
    processes[process->id] = 0x00;
    
    if (current_process == process) {
        process_switch_to_any();
        // TODO: better mechanism for choosing which process to switch into
    }
}

// Frees the task, stack, process allocations, and process object, and terminates the process
int process_terminate(struct process* process) {
    int res = 0;

    // 1. Free the allocations
    res = process_terminate_allocations(process);
    if (res < 0) {
        goto out;
    }

    // Free the program data
    res = process_terminate_program_data(process);
    if (res < 0) {
        goto out;
    }

    // 3. Free the process stack memory
    kfree(process->stack);
    // 4. Free the task
    task_free(process->task);
    // 5. Remove the process from the processes array
    process_unlink(process);
    // 6. Free the process object
    printf("Terminating process: %s ...\n", process->filename);
    kfree(process);

out:
    return res;
}

void process_get_arguments(struct process* process, int* argc, char*** argv) {
    *argc = process->arguments.argc;
    *argv = process->arguments.argv;
}

int process_count_commands(struct command_argument* root_argument) {
    struct command_argument* current = root_argument;
    int count = 0;
    while(current) {
        count++;
        current = current->next;
    }
    return count;
}

// Takes a root 'command_argument' struct and inserts all the values into the process' struct
int process_inject_arguments(struct process* process, struct command_argument* root_argument) {
    int res = 0;
    struct command_argument* current = root_argument;

    int argc = process_count_commands(root_argument);
    if (argc == 0) {
        res = -EIO;
        goto out;
    }

    // Allocate some space for the (pointers to) arguments accessible from within the process space
    // (arg count * size of an arg pointer) because argv is an array of pointers
    char **argv = process_malloc(process, argc * sizeof(const char*));
    if (!argv) {
        res = -ENOMEM;
        goto out;
    }

    int i = 0;
    // Copy each argument into the process space by assigning it memory, and add its pointer to argv**
    while(current) {
        // Allocate space for the local current argument
        char* argument_string = process_malloc(process, sizeof(current->argument));
        if (!argument_string) {
            res = -ENOMEM;
            goto out;
        }
        // Copy the current argument into the local buffer
        strncpy(argument_string, current->argument, sizeof(current->argument));
        argv[i] = argument_string;

        // Increment for loop
        current = current->next;
        i++;
    }

    // Finally, add the newly generated argc and argv to the process struct
    process->arguments.argc = argc;
    process->arguments.argv = argv;

    // Now argc and argv will be available to the process via a system command
    
out:
    return res;
}

// Frees the memory allocated for this process at the given address
void process_free(struct process* process, void* ptr) {

    // This check is now redundant since 'process_get_allocation_by_addr' checks anyways
    //    
    // // Check if the process doesn't own the pointer it's sending
    // if (!process_ptr_belongs_to_process(process, ptr)) {
    //     // If so, do nothing
    //     return;
    // }

    // Unlink the pages from the process for the given address by removing the flags
    struct process_allocation* allocation = process_get_allocation_by_addr(process, ptr);
    if (!allocation) {
        // Oops, it's not our pointer L:(
        return;
    }
    int res = paging_map_to(process->task->page_directory, allocation->ptr, allocation->ptr, paging_align_address(allocation->ptr + allocation->size), 0x00);
    if (res < 0) {
        return;
    }

    // Clear the entry and break from the loop
    process_allocation_unjoin(process, ptr);

    // Decrement active allocations
    process->active_allocations--;

    // We can now free the memory
    kfree(ptr);
}

// Loads a binary file as a process
static int process_load_binary(const char* filename, struct process* process) {
    int res = 0;

    void* program_data_ptr = 0;

    int fd = fopen(filename, "r");
    if (!fd) {
        res = -EIO;
        goto out;
    }

    // We need to know how large the file is to know how much memory to allocate
    struct file_stat stat;
    res = fstat(fd, &stat);
    if (res != PEACHOS_ALL_OK) {
        goto out;
    }

    // Declare the memory for the program data we're loading
    program_data_ptr = kzalloc(stat.file_size);
    if (!program_data_ptr) {
        res = -ENOMEM;
        goto out;
    }

    if (fread(program_data_ptr, stat.file_size, 1, fd) != 1) {
        // fread should return 1, the number of memory blocks we specified
        // If this block is true, we failed to read the memory block specified
        res = -EIO;
        goto out;
    }

    process->filetype = PROCESS_FILETYPE_BINARY;
    process->ptr = program_data_ptr;
    process->size = stat.file_size;

out:
    // Check for an error
    if (res < 0) {
        if (program_data_ptr) {
            kfree(program_data_ptr);
        }
    }
    
    fclose(fd);
    return res;
}

// Loads an elf file as a process
static int process_load_elf(const char* filename, struct process* process) {
    int res = 0;

    struct elf_file* elf_file = 0;
    res = elf_load(filename, &elf_file);
    if(ISERR(res)) {
        goto out;
    }
    process->filetype = PROCESS_FILETYPE_ELF;
    process->elf_file = elf_file;
out:
    return res;
}

// Responsible for loading the data from the provided file after checking what
// type it is (e.g., is it a raw binary or elf file?)
static int process_load_data(const char* filename, struct process* process) {
    int res = 0;
    res = process_load_elf(filename, process);
    if (res == -EINFORMAT) {
        // If file isn't elf, load as binary
        res = process_load_binary(filename, process);
    }
    return res;
}

int process_map_binary(struct process* process) {
    int res = 0;

    paging_map_to(process->task->page_directory, (void*)PEACHOS_PROGRAM_VIRTUAL_ADDRESS, process->ptr, paging_align_address(process->ptr + process->size), PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);

    return res;
}

static int process_map_elf(struct process* process) {
    int res = 0;

    struct elf_file* elf_file = process->elf_file;
    struct elf_header* header = elf_header(elf_file);

    // Get the program headers and loop through them
    struct elf32_phdr* phdrs = elf_pheader(header);
    for (int i = 0; i < header->e_phnum; i++) {
        struct elf32_phdr* phdr = &phdrs[i];
        void* phdr_phys_address = elf_phdr_phys_address(elf_file, phdr); // Physical address of segment (of this header) (which is currently in memory)
        // Now append correct flags
        int flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;
        if (phdr->p_flags & PF_W) {
            flags |= PAGING_IS_WRITEABLE;
        }

        // Map and ensure addresses are aligned
        res = paging_map_to(process->task->page_directory, paging_align_to_lower_page((void*)phdr->p_vaddr), paging_align_to_lower_page(phdr_phys_address), paging_align_address(phdr_phys_address + phdr->p_memsz), flags);
        if (ISERR(res)) {
            break;
        }
    }

    return res;
}

// Map the process' memory to the virtual paging system
// Expects the process to already have loaded memory
// Is generic and maps based on type of file (e.g., elf or raw binary)
int process_map_memory(struct process* process) {
    int res = 0;

    // Map the program memory
    switch(process->filetype) {
        case PROCESS_FILETYPE_ELF:
            res = process_map_elf(process);
            break;
        case PROCESS_FILETYPE_BINARY:
            res = process_map_binary(process);
            break;
        default:
            panic("process_map_memory(): Invalid file type");
    }

    if (res < 0) {
        goto out;
    }

    // Map the stack memory
    res = paging_map_to(process->task->page_directory, (void*)PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END, process->stack, paging_align_address(process->stack + PEACHOS_USER_PROGRAM_STACK_SIZE), PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
    if (res < 0) {
        goto out;
    }
    
out:
    return res;
}

int process_get_free_slot() {
    for (int i = 0; i < PEACHOS_MAX_PROCESSES; i++) {
        if (processes[i] == 0)
            return i;
    }

    return -EISTKN;
}

// Load and assign the stack memory, filename, id for the process
// The rest is handled by other functions (called from this function)
int process_load_for_slot(const char* filename, struct process** process, int process_slot) {
    int res = 0;
    struct task* task = 0;
    struct process* _process;
    void* program_stack_ptr = 0;

    if (process_get(process_slot) != 0) {
        // If result isn't 0, then the slot is occupied with a process address
        res = -EISTKN;
        goto out;
    }

    // Assign memory for the process structure itself (not the process memory when loading the file)
    _process = kzalloc(sizeof(struct process));
    if (!_process) {
        res = -ENOMEM;
        goto out;
    }

    process_init(_process);
    res = process_load_data(filename, _process); // Loads the data based on what type of file is provided (populates process->ptr, ->size)
    if (res < 0) {
        goto out;
    }

    // Allocate memory for the process stack
    program_stack_ptr = kzalloc(PEACHOS_USER_PROGRAM_STACK_SIZE);
    if (!program_stack_ptr) {
        res = -ENOMEM;
        goto out;
    }

    // Set the process filename
    strncpy(_process->filename, filename, sizeof(_process->filename));
    _process->stack = program_stack_ptr;
    _process->id = process_slot;

    // Create a task
    task = task_new(_process);
    if (ERROR_I(task) == 0) {
        res = ERROR_I(task);
        goto out;
    }
    _process->task = task;

    // Provide paging-mapped memory for the process
    res = process_map_memory(_process);

    *process = _process;

    // Add the process to the array
    processes[process_slot] = _process;

out:
    // Clean up memory if there was an error
    if (ISERR(res)) {
        if (_process && _process->task) {
            task_free(_process->task);
        }

        // kfree(_process);
        // TODO free all the process data
    }

    return res;
}

// Loads a process and sets it as the current process
int process_load_switch(const char* filename, struct process** process) {
    int res = process_load(filename, process);

    if (res != PEACHOS_ALL_OK) {
        print("Could not find program: ");
        println(filename);
        return res;
    }

    process_switch(*process);
    return res;
}

// Finds an empty slot in the process array and starts the initialisation of the process
int process_load(const char* filename, struct process** process) {
    int res = 0;

    int process_slot = process_get_free_slot();
    if (process_slot < 0) {
        res = -EISTKN;
        goto out;
    }

    res = process_load_for_slot(filename, process, process_slot);

out:
    return res;
}