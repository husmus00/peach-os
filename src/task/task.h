#if !defined(TASK_H)
#define TASK_H

#include "config.h"
#include "memory/paging/paging.h"

struct interrupt_frame;

// Enables us to save and restore registers upon switching tasks (including kernel interrupts)
struct registers {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
};

struct process;

// A task is like a thread and we can context switch between tasks
// Each task will be handled by a process (which may have multiple tasks)
struct task {
    struct paging_4gb_chunk* page_directory; // Each task has its own page directory
    struct registers registers; // Registers of the task, when the task is not running
    struct process* process; // The process that owns the task
    struct task* next; // The next task in the linked list
    struct task* prev; // The previous task in the linked list
};

struct task* task_new(struct process* process);
struct task* task_current();
struct task* task_get_next();
int task_free(struct task* task);

int task_switch(struct task* task);
int task_page();
int task_page_task(struct task* task);

void task_run_first_ever_task();

void task_return(struct registers* regs); // Drops us into user land, explanation in task.asm
void restore_general_purpose_registers(struct registers* regs);
void user_registers();

void task_current_save_state(struct interrupt_frame* frame);
int copy_string_from_task(struct task* task, void* virtual, void* physical, int max);
void* task_get_stack_item(struct task* task, int index);
void* task_virtual_address_to_physical(struct task* task, void* virtual_address);
void task_next();

#endif // TASK_H
