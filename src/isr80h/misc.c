#include <stddef.h>
#include "misc.h"
#include "idt/idt.h"
#include "kernel.h"
#include "task/task.h"
#include "memory/paging/paging.h"
#include "keyboard/keyboard.h"

// The return values of these functions are returned to user land

void* isr80h_command0_sum(struct interrupt_frame* frame) 
{
    // print("System call 0: sum");

    int v2 = (int)task_get_stack_item(task_current(), 1);
    int v1 = (int)task_get_stack_item(task_current(), 0);

    return (void*)(v1 + v2);
}

void* isr80h_command1_print(struct interrupt_frame* frame) {
    // print("System call 1: print");

    void* message_virt_addr = task_get_stack_item(task_current(), 0);

    char message_real_buffer[1024];

    copy_string_from_task(task_current(), message_virt_addr, message_real_buffer, sizeof(message_real_buffer));

    print(message_real_buffer);

    return 0;
}

void* isr80h_command2_putchar(struct interrupt_frame* frame) {
   char c = (char)(int) task_get_stack_item(task_current(), 0);

   terminal_writechar(c, TERMINAL_DEFAULT_COLOUR);
   return 0;
}

void* isr80h_command3_getkey(struct interrupt_frame* frame) {
    char c = keyboard_pop();
    return (void*)((int)c);
}
