#if !defined(PROCESS_H)
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "task.h"

#define PROCESS_FILETYPE_ELF 0
#define PROCESS_FILETYPE_BINARY 1

typedef unsigned char PROCESS_FILETYPE;

// Used to keep track of an allocation in the process (the address/pointer and size)
struct process_allocation {
    size_t size;
    void* ptr;
};

// A linked list of command arguments (the strings themselves) for the shell
struct command_argument {
    char argument[512];
    struct command_argument* next;
};

// Holds the argc (argument count) and argv (list of *pointers* to the argument strings)
// There's a system argument that supplies these to a process.
// The start.asm routine can call it and pass the arguments to the main() function 
struct process_arguments {
    int argc;
    char** argv;
};

// Theoretically, a process may have multiple tasks
// For this course, we have one task per process
struct process {
    // The process id
    uint16_t id;
    // 
    char filename[PEACHOS_MAX_PATH];
    // The main process task
    struct task* task;
    // Allows us to keep track of all the allocations (malloc) the program performed,
    // so we can free any unfreed memory upon process termination (or crash, etc.)
    int active_allocations;
    struct process_allocation allocations[PEACHOS_MAX_PROGRAM_ALLOCATIONS];
    
    // Type of file loaded by the process (e.g., elf, binary)
    PROCESS_FILETYPE filetype;

    // The physical pointer to the process memory (code and data) 
    union
    {
        // binary format (no sections e.g., header)
        void* ptr;
        // ELF format
        struct elf_file* elf_file;
    };
    
    // The physical pointer to the stack memory
    void* stack;
    // The size of the data pointed to by 'ptr'
    uint32_t size;
    
    struct keyboard_buffer {
        char buffer[PEACHOS_KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    } keyboard;

    struct process_arguments arguments;
};

struct process* process_current();
struct process* process_get(int process_id);
int process_switch(struct process* process);
int process_load_switch(const char* filename, struct process** process);
int process_load_for_slot(const char* filename, struct process** process, int process_slot);
int process_load(const char* filename, struct process** process);

void* process_malloc(struct process* process, size_t size);
void process_free(struct process* process, void* ptr);

void process_get_arguments(struct process* process, int* argc, char*** argv);
int process_inject_arguments(struct process* process, struct command_argument* root_argument);

int process_terminate(struct process* process);

#endif // PROCESS_H
