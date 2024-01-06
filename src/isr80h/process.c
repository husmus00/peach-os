#include "isr80h.h"
#include "process.h"
#include "task/task.h"
#include "task/process.h"
#include "status.h"
#include "config.h"
#include "string/string.h"
#include "kernel.h"

void* isr80h_command0_process_exit(struct interrupt_frame* frame) {
    struct process* process = task_current()->process;
    int exit_code = (int)task_get_stack_item(task_current(), 0);
    printf("\nProcess \"%s\" ended with exit code %i\n", process->filename, exit_code);
    process_terminate(process);
    task_next();
    return 0;
}

void* isr80h_command6_process_load_start(struct interrupt_frame* frame) {
    void* path_user_ptr = task_get_stack_item(task_current(), 0);
    char path[PEACHOS_MAX_PATH];
    int res = copy_string_from_task(task_current(), path_user_ptr, path, sizeof(path));
    if (res < 0) {
        goto out;
    }

    // TODO: Implement proper environment variables
    // char path[PEACHOS_MAX_PATH];
    // strcpy(path, "0:/");
    // strcpy(path+3, filename);

    struct process* process = 0;
    res = process_load_switch(path, &process);
    if (res < 0) {
        goto out;
    }

    // If we reach this part the process was loaded successfully, and we won't be returning from this function

    task_switch(process->task); // Because process_load_switch() only switches the current process, not task
    task_return(&process->task->registers); // Drop into lower privilege

    // Cannot actually move past this point since the execution flow will change

out:
    return 0;
}

// Runs a program via the supplied command arguments (command was already parsed in the stdlib)
// and injects the arguments into the process
void* isr80h_command7_invoke_system_command(struct interrupt_frame* frame) {
    int res = 0;
    void* arguments_virt_addr = task_get_stack_item(task_current(), 0);
    struct command_argument* arguments = task_virtual_address_to_physical(task_current(), arguments_virt_addr);
    // Ensure a command with a non-zero-length program name was passed
    if (!arguments || strlen(arguments[0].argument) == 0) {
        return ERROR(-EINVARG);
    }

    // TODO: check if root_command and arguments are the same
    struct command_argument* root_command_argument = &arguments[0]; // Should contain the name of the program to run
    const char* path = root_command_argument->argument;
    if (strlen(path) > PEACHOS_MAX_PATH - 4) {
        panic("isr80h_command7_invoke_system_command: program name is too long");
    }

    // Get the program's path
    // TODO: Implement proper environment variables
    // char path[PEACHOS_MAX_PATH];
    // strcpy(path, "0:/");
    // strncpy(path+3, program_name, sizeof(path));

    // Now load the process
    struct process* process = 0;
    res = process_load_switch(path, &process);
    if (res < 0) {
        return 0; // ERROR(res);
    }

    // If we reach this part the process was loaded successfully, and we won't be returning from this function

    // We now inject all the arguments into the process
    res = process_inject_arguments(process, root_command_argument);
    if (res < 0 ) {
        return 0; // ERROR(res);
    }

    task_switch(process->task); // Because process_load_switch() only switches the current process, not task
    task_return(&process->task->registers); // Drop into lower privilege

out:
    return 0;
}

// Returns the calling process' arguments to it via the address it provided
void* isr80h_command8_get_program_arguments(struct interrupt_frame* frame) {
    struct process* process = task_current()->process;
    // A virtual address will be passed to us from the process that invoked the system command, into which we'll supply the arguments
    void* virt_addr = task_get_stack_item(task_current(), 0);
    struct process_arguments* arguments = task_virtual_address_to_physical(task_current(), virt_addr);

    process_get_arguments(process, &arguments->argc, &arguments->argv);

    return  0;
}