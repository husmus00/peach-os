#if !defined(PEACHOS_H)
#define PEACHOS_H

#include <stddef.h>
#include <stdbool.h>

#include "../../../src/config.h"

// A linked list of command arguments for the shell
struct command_argument {
    char argument[512];
    struct command_argument* next;
};

// There's a system argument that supplies these to a process.
// The start.asm routine can call it and pass the arguments to the main() function 
struct process_arguments {
    int argc;
    char** argv;
};

// System calls 0x80
// TODO: syscall 0 should be program exit
void  peachos_exit(int exit_code);
void  print(const char* message);
void  peachos_putchar(char c);
int   peachos_getkey();
void* peachos_malloc(size_t size);
void  peachos_free(void* ptr);
void* peachos_process_load_start(const char* filename);
int   peachos_system(struct command_argument* arguments);
void  peachos_process_get_arguments(struct process_arguments* arguments);

int peachos_getkeyblock();
void peachos_terminal_readline(char* out, int max, bool output_while_typing);
struct command_argument* peachos_parse_command(const char* command, int max);
int peachos_system_run(const char* command);

#endif // PEACHOS_H
