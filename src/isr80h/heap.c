#include <stddef.h>
#include "heap.h"
#include "idt/idt.h"
#include "task/task.h"
#include "task/process.h"
#include "kernel.h"

void* isr80h_command4_malloc(struct interrupt_frame* frame) {
    // First, get the size pushed to the stack in peachos.asm
    size_t size = (int)task_get_stack_item(task_current(), 0);

    // Return the address returned from process_malloc
    // (which will allocate for the current task's process and record it in the struct)
    return process_malloc(task_current()->process, size);
}

void* isr80h_command5_free(struct interrupt_frame* frame) {
    void* ptr_to_free = task_get_stack_item(task_current(), 0);
    process_free(task_current()->process, ptr_to_free);

    return 0;
}