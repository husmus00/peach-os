#include "peachos.h"
#include "stdio.h"
#include "string.h"
#include "../../../src/status.h"

// Attempt to getkey and block until key is pressed
int peachos_getkeyblock() {
    int val = 0;
    do {
        val = peachos_getkey();
    }
    while(val == 0);

    return val;
}

// Fill a buffer while user is typing until Enter is pressed
void peachos_terminal_readline(char* out, int max, bool output_while_typing) {
    int i = 0;

    // char* temp_string = "TEST/BLANK.ELF 0"; 
    // strncpy(out, temp_string, max);
    // return;

    // Loop until max is reached or break
    for (i = 0; i < max - 1; i++) {
        int key = peachos_getkeyblock();
        // Check if key is carriage return, we have read the line
        if (key == 13) {
            break;
        }
        // Check for backspace
        if (key == 0x08 && i >= 1) {
            out[i - 1] = 0x00;
            i -= 2; // Minus 2 because on loop i will be incremented again
            putchar(key);
            continue;
        }
        // Otherwise add character to buffer
        out[i] = key;

        // Output to the terminal
        if (output_while_typing) {
            putchar(key);
        }
    }

    out[i] = 0x00; // Add null terminator
    putchar('\n');
}

// Split a command into its arguments and return a linked list (command_argument struct)
struct command_argument* peachos_parse_command(const char* command, int max) {
    struct command_argument* root_command = 0;
    char scommand[1025]; // Max command size
    if (max >= (int)sizeof(scommand)) {
        // Invalid, supplied max size cannot be over the allowed threshold
        return 0;
    }

    // Copy entire command into local buffer (because we can't modify the supplied constant char pointer)
    strncpy(scommand, command, sizeof(scommand));
    char* token = strtok(scommand, " "); // Split the command by spaces
    if (!token) {
        goto out;
    }

    // Allocate space for the root command (program name)
    root_command = peachos_malloc(sizeof(struct command_argument));
    if (!root_command) {
        goto out;
    }

    // Copy value of first argument into the root command struct
    strncpy(root_command->argument, token, sizeof(root_command->argument));
    root_command->next = 0;

    // Keep track of current argument and index the next one
    struct command_argument* current = root_command;
    token = strtok(NULL, " "); // Keep parsing the previously supplied command string  

    // Keep indexing arguments until there are no more
    while(token != 0) {
        struct command_argument* new_command = peachos_malloc(sizeof(struct command_argument));
        if (!new_command) {
            break;
        }

        // Copy value of next argument into an 'argument' struct and set the 'next' field of the previous argument
        strncpy(new_command->argument, token, sizeof(new_command->argument));
        new_command->next = 0;
        current->next = new_command;
        current = new_command;
        token = strtok(NULL, " ");
    }

out:
    // Finally, return the linked list of arguments
    return root_command;
}

// Parse the given command and invoke the peachos_system syscall to run the supplied program by name
int peachos_system_run(const char* command) {
    char buf[1024]; // We're limiting the length of the command
    strncpy(buf, command, sizeof(buf));
    struct command_argument* arguments = peachos_parse_command(buf, sizeof(buf));
    if (!arguments) {
        return -EINVARG;
    }
    
    return peachos_system(arguments);
}