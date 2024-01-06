#include "keyboard.h"
#include "status.h"
#include "kernel.h"
#include "task/process.h"
#include "task/task.h"
#include "classic.h"

static struct keyboard* keyboard_list_head = 0;
static struct keyboard* keyboard_list_last = 0;

// Will initialise the pre-compiled keyboard drivers
void keyboard_init() {
    keyboard_insert(classic_init());
}

int keyboard_insert(struct keyboard* keyboard) {
    int res = 0;

    // Check if supplied keyboard has an initialisation function
    if (keyboard->init == 0) {
        res = -EINVARG;
        goto out;
    }

    // Check if a previous keyboard exists (i.e., 'keyboard_list_last' is non-zero)
    if (keyboard_list_last) {
        keyboard_list_last->next = keyboard;
        keyboard_list_last = keyboard;
    }
    else {
        keyboard_list_head = keyboard;
        keyboard_list_last = keyboard;
    }

    res = keyboard->init();
out:
    return res;
}

// We use modulus to avoid any bound checks on tail
static int keyboard_get_tail_index(struct process* process) {
    return process->keyboard.tail % sizeof(process->keyboard.buffer);
    // TODO try putting PEACHOS_KEYBOARD_BUFFER_SIZE instead ^^^
}

void keyboard_backspace(struct process* process) {
    process->keyboard.tail -= 1;
    int real_index_tail = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index_tail] = 0x00; // Reset the backspaced character
} 

void keyboard_set_capslock(struct keyboard* keyboard, KEYBOARD_CAPSLOCK_STATE state) {
    keyboard->capslock_state = state;
}

KEYBOARD_CAPSLOCK_STATE keyboard_get_capslock(struct keyboard* keyboard) {
    return keyboard->capslock_state;
}

// Push a character to the current process' keyboard buffer. 
// In this case, the 'current process' is defined as the process we
// can 'see' (active), rather than the one currently running
void keyboard_push(char c) {
    struct process* process = process_current(); 
    if (!process) {
        return;
    }

    if (c == 0) {
        return;
    }

    int real_index_tail = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index_tail] = c;
    process->keyboard.tail++;
}

// Returns a character to the currently running task when it requests one from the process buffer
char keyboard_pop() {
    if (!task_current()) {
        return 0;
    }

    struct process* process = task_current()->process;
    int real_index_head = process->keyboard.head % sizeof(process->keyboard.buffer);
    char c = process->keyboard.buffer[real_index_head];
    if (c == 0x00) {
        // Nothing to pop
        return 0;
    }

    process->keyboard.buffer[real_index_head] = 0;
    process->keyboard.head++;

    return c;
}

