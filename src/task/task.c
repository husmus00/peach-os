#include "task.h"
#include "status.h"
#include "kernel.h"
#include "memory/heap/kernelheap.h"
#include "memory/memory.h"
#include "process.h"
#include "idt/idt.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "loader/formats/elfloader.h"

// Current task that is running
struct task* current_task = 0;

// Task linked list
struct task* task_tail = 0;
struct task* task_head = 0;

int task_init(struct task* task, struct process* process);

struct task* task_current() {
    return current_task;
}

struct task* task_get_next() {
    // Loop around if the current task is at the end of the list
    if (!current_task->next) {
        return task_head;
    }

    return current_task->next;
}

static void task_list_remove(struct task* task) {
    
    // TODO change the next task's 'prev' field?
    
    if (task->prev) {
        task->prev->next = task->next;
    }

    if (task == task_head) {
        task_head = task->next;
    }

    if (task == task_tail) {
        task_tail = task->prev;
    }

    if (task == current_task) {
        current_task = task_get_next();
    }
}

int task_free(struct task* task) {
    paging_free_4gb(task->page_directory);
    task_list_remove(task);

    // Finally free the task data
    kfree(task);
    return 0;
}

struct task* task_new(struct process* process) {
    int res = 0;
    struct task* task = kzalloc(sizeof(struct task));
    if (!task) {
        res = -ENOMEM;
        goto out;
    }

    // Set the default values for 'task'
    res = task_init(task, process);
    if (res != PEACHOS_ALL_OK) {
        goto out;
    }

    // Set this task to be the head if no other tasks exist
    if (task_head == 0) {
        task_head = task;
        task_tail = task;
        current_task = task;
        goto out;
    }

    // Set this task to be the new tail
    task_tail->next = task;
    task->prev = task_tail;
    task_tail = task;

out:
    if (ISERR(res)) {
        task_free(task);
        return ERROR(res);
    }

    return task;
}

void task_save_frame(struct task* task, struct interrupt_frame* frame) {
    task->registers.ip = frame->ip;
    task->registers.cs = frame->cs;
    task->registers.flags = frame->flags;
    task->registers.esp = frame->esp;
    task->registers.ss = frame->ss;
    task->registers.eax = frame->eax;
    task->registers.ebp = frame->ebp;
    task->registers.ebx = frame->ebx;
    task->registers.ecx = frame->ecx;
    task->registers.edi = frame->edi;
    task->registers.edx = frame->edx;
    task->registers.esi = frame->esi;
}

// We need to be in the kernel page before invoking this function
// Creates temporary space, moves the data from task to temp (in task page), then temp to physical (in kernal page)
int copy_string_from_task(struct task* task, void* virtual, void* physical, int max) {
    // We only want to modify a max of 1 page (easier to implement)
    if (max >= PAGING_PAGE_SIZE) {
        return -EINVARG;
    }

    int res = 0;

    // Will be shared by both physical (kernel) and virtual (task) memory
    // allowing them to share data/communicate
    char* tmp = kzalloc(max); 
    if (!tmp) {
        res = -ENOMEM;
        goto out;
    }

    // Get the task directory and store the old page (to restore later, it could contain information already)
    uint32_t* task_directory = task->page_directory->directory_entry;
    uint32_t old_entry = paging_get(task_directory, tmp);

    // Map the physical address of the tmp space to the same virtual address in the task page
    paging_map(task->page_directory, tmp, tmp, PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(task->page_directory);

    // Now move the data from the task into the tmp space
    strncpy(tmp, virtual, max);

    // We need to return to the kernel page
    kernel_page();

    // Restore the old page that was in the task
    res = paging_set(task_directory, tmp, old_entry);

    if (res < 0) {
        res = -EIO;
        goto out_free;
    }

    // Finally, move the data from the tmp space to the kernel's memory
    strncpy(physical, tmp, max);

out_free:
    kfree(tmp);
out:
    return res;
}

void task_current_save_state(struct interrupt_frame* frame) {
    if (!task_current()) {
        panic("No current task to save");
    }

    struct task* task = task_current();
    task_save_frame(task, frame);
}

// Takes us out of the kernel page into the task page
// Used after returning from an interrupt
int task_page() {
    user_registers();
    task_switch(current_task);
    return 0;
}

// Switch into the supplied task's page
int task_page_task(struct task* task) {
    user_registers();
    paging_switch(task->page_directory);
    return 0;
}

void task_next() {
    struct task* next_task = task_get_next();
    if (!next_task) {
        panic("task_next(): No more tasks");
    }

    task_switch(next_task);
    task_return(&next_task->registers);

    // We won't be returning from this function, execution flow will change
}

// Switch the current running task to the supplied task
// Also switch the page directory
int task_switch(struct task* task) {
    current_task = task;
    paging_switch(task->page_directory);
    return 0;
}

void task_run_first_ever_task() {
    if (!current_task) {
        panic("task_run_first_ever_task(): No current task exists");
    }
    
    task_switch(task_head);
    task_return(&task_head->registers);
}

// Initialising the given task
int task_init(struct task* task, struct process* process) {
    memset(task, 0, sizeof(struct task));
    // Map the entire 4GB address space to itself
    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->page_directory) {
        return -EIO;
    }

    // Assign appropriate entry point
    task->registers.ip = PEACHOS_PROGRAM_VIRTUAL_ADDRESS;
    if (process->filetype == PROCESS_FILETYPE_ELF) {
        task->registers.ip = elf_header(process->elf_file)->e_entry;
    }

    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;

    task->process = process;

    return 0;
}


// Allows us to 'peek' into and retrieve the task stack's contents
// Must be in the kernel page before invoking this function
void* task_get_stack_item(struct task* task, int index) {
    void* result = 0;

    uint32_t* sp_ptr = (uint32_t*) task->registers.esp; // Virtual address
    task_page_task(task); // Switch to the given task's page (not necessarily the current one)

    result = (void*) sp_ptr[index];

    kernel_page(); // Back to the kernel page

    return result;
}

// Translate the given task's (virtual) address into its physical address
void* task_virtual_address_to_physical(struct task* task, void* virtual_address) {
    return paging_get_physical_address(task->page_directory->directory_entry, virtual_address); 
}