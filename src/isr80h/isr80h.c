#include "isr80h.h"
#include "idt/idt.h"
#include "misc.h"
#include "heap.h"
#include "process.h"

void isr80h_register_all_commands() {
    // isr80h_register_command(SYSTEM_COMMAND_0_SUM, isr80h_command0_sum);
    isr80h_register_command(SYSTEM_COMMAND_0_EXIT, isr80h_command0_process_exit);
    isr80h_register_command(SYSTEM_COMMAND_1_PRINT, isr80h_command1_print);
    isr80h_register_command(SYSTEM_COMMAND_2_PUTCHAR, isr80h_command2_putchar);
    isr80h_register_command(SYSTEM_COMMAND_3_GETKEY, isr80h_command3_getkey);
    isr80h_register_command(SYSTEM_COMMAND_4_MALLOC, isr80h_command4_malloc);
    isr80h_register_command(SYSTEM_COMMAND_5_FREE, isr80h_command5_free);
    isr80h_register_command(SYSTEM_COMMAND_6_PROCESS_LOAD_START, isr80h_command6_process_load_start);
    isr80h_register_command(SYSTEM_COMMAND_7_INVOKE_SYSTEM_COMMAND, isr80h_command7_invoke_system_command);
    isr80h_register_command(SYSTEM_COMMAND_8_GET_PROGRAM_ARGUMENTS, isr80h_command8_get_program_arguments);
}