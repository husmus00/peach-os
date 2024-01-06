#include <stdint.h>
#include <stddef.h>
#include "classic.h"
#include "keyboard.h"
#include "io/io.h"
#include "kernel.h"
#include "idt/idt.h"
#include "task/task.h"
#include "kernel.h"
#include "string/string.h"

#define CLASSIC_KEYBOARD_CAPSLOCK 15 // Actually tab

int classic_keyboard_init();

// The scan codes provided by the PS/2 keyboard is used as an index into this array
static uint8_t keyboard_scan_set_one[] = {
    0x00, 0x01B, '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    0x0D, 0x00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`',
    0x00, '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5', // Numpad scan codes
    '6', '+', '1', '2', '3', '0', '.'
};

// Represents a classic PS/2 keyboard
struct keyboard classic_keyboard = {
    .name = {"Classic PS2"},
    .init = classic_keyboard_init
};

void classic_keyboard_handle_interrupt();

// Initialise the keyboard
// Gets called from the virtual keyboard layer
int classic_keyboard_init() {
    idt_register_interrupt_callback(ISR_KEYBOARD_INTERRUPT, classic_keyboard_handle_interrupt);
    keyboard_set_capslock(&classic_keyboard, KEYBOARD_CAPSLOCK_OFF);
    outb(PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT);
    return 0;
}

uint8_t classic_keyboard_scancode_to_char(uint8_t scancode) {
    size_t size_of_keyboard_set_one = sizeof(keyboard_scan_set_one) / sizeof(uint8_t);
    if (scancode > size_of_keyboard_set_one) {
        return 0;
    }

    char c = keyboard_scan_set_one[scancode];

    // Check for case
    if (keyboard_get_capslock(&classic_keyboard) == KEYBOARD_CAPSLOCK_OFF) {
        if (is_ascii_char(c)) {
            c = tolower(c);
        }
    }

    return c;
}

// Called from the interrupt handler for the keyboard interrupt
void classic_keyboard_handle_interrupt() {
    kernel_page();
    uint8_t scancode = 0;
    scancode = insb(KEYBOARD_INPUT_PORT);
    insb(KEYBOARD_INPUT_PORT); // Ignores potential "rogue byte" that may be sent

    // Check if the key was released rather than pressed (do nothing)
    if (scancode & CLASSIC_KEYBOARD_KEY_RELEASED) {
        return;
    }

    // Check if the capslock was pressed and toggle the state
    if (scancode == CLASSIC_KEYBOARD_CAPSLOCK) {
        KEYBOARD_CAPSLOCK_STATE old_state = keyboard_get_capslock(&classic_keyboard);
        KEYBOARD_CAPSLOCK_STATE new_state = (old_state == KEYBOARD_CAPSLOCK_ON) ? KEYBOARD_CAPSLOCK_OFF : KEYBOARD_CAPSLOCK_ON;
        keyboard_set_capslock(&classic_keyboard, new_state);
        goto out;
    }

    // We only care if the key was pressed
    char c = (char)classic_keyboard_scancode_to_char(scancode);
    if (c != 0) {
        // Is a valid ASCII character
        keyboard_push(c); // Push to the keyboard buffer of the current process
        // print(itoa(scancode));
    }

out:
    task_page();
    
    // else {
    //     print_char(&c);
    // }
}

// Returns the instance of the classic keyboard
struct keyboard* classic_init() {
    return &classic_keyboard;
}