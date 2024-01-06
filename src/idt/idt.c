#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "kernel.h"
#include "io/io.h"
#include "task/task.h"
#include "task/process.h"
#include "string/string.h"
#include "status.h"

struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS]; // List of interrupt descriptors (The IDT table)
struct idtr_desc idtr_descriptor;                          // IDTR is the IDT register

// External array of interrupt pointers defined in idt.asm
extern void* interrupt_pointer_table[PEACHOS_TOTAL_INTERRUPTS];

static INTERRUPT_CALLBACK_FUNCTION interrupt_callbacks[PEACHOS_TOTAL_INTERRUPTS];
static ISR80H_COMMAND isr80h_commands[PEACHOS_MAX_ISR80H_COMMANDS];

extern void idt_load(struct idtr_desc *ptr);
extern void no_interrupt(); // The assembly routine to call for unimplemented interrupts
extern void isr80h_wrapper();

// The assembly routines to call on interrupts
extern void int0Dh(); // General protection fault
extern void int20h(); // Timer
extern void int21h(); // Keyboard


void idt_zero()
{
    panic("Divide by zero error");
}

void int0Dh_handler() {
    panic("General protection fault");
}

void int20h_handler()
{
    print("Timer triggered!");
}

void int21h_handler()
{
    print("Keyboard pressed!");
    outb(0x20, 0x20);
}

void no_interrupt_handler()
{
    // DO NOT CALL DIRECTLY FROM C
    outb(0x20, 0x20); // Tell the PIC we're done handling the interrupt
}

// Handles all the (macro-auto-generated) interrupts (Not the 0x80 system calls)
// Arguments are pulled from the stack after this function is called from assembly
void interrupt_handler(int interrupt, struct interrupt_frame* frame) {
    
    // print("\nCalled interrupt handler:");
    // print_char((const char*)int_to_char(interrupt));
    // print("\n");

    // We need to go to kernel land to handle the interrupt
    kernel_page();
    if (interrupt_callbacks[interrupt] != 0) {
        task_current_save_state(frame);
        interrupt_callbacks[interrupt](frame);
    }

    task_page();      // Return to user land
    outb(0x20, 0x20); // Send the acknowledgement to the interrupt handler (PIC)
}

// For assembly functions (with wrappers perhaps)
void idt_set(int interrupt_no, void *address)
{
    // Get a pointer to a struct of type idt_desc from the idt array
    struct idt_desc *desc = &idt_descriptors[interrupt_no];

    desc->offset_1 = (uint32_t)address & 0x0000FFFF;
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    desc->type_attr = 0xEE; // 0b11101110
    desc->offset_2 = (uint32_t)address >> 16;
}

void idt_handle_exception() {
    println("\nException reached");
    process_terminate(task_current()->process);
    task_next();
}

void idt_clock() {
    outb(0x20, 0x20); // Send the acknowledgement to the interrupt handler (PIC)
    // Switch to the next task
    task_next();
    // We never reach this point
}

void idt_init()
{
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    // Set all the interrupts
    // Not doing this causes the system to keep resetting (why?)
    for (int i = 0; i < PEACHOS_TOTAL_INTERRUPTS; i++)
    {
        idt_set(i, interrupt_pointer_table[i]);
    }

    // Set all the exception interrupts
    for (int i = 0; i < 0x20; i++) {
        // Interrupts 0x00 - 0x19 are exceptions
        idt_register_interrupt_callback(i, idt_handle_exception);
    }

    // System calls
    idt_set(0x80, isr80h_wrapper); // Any time a user program calls 0x80 (kernel interrupt) isr80h_wrapper will be called

    idt_register_interrupt_callback(0, idt_zero);
    // idt_set(0x0D, int0Dh);
    // idt_set(0x21, int21h);
    idt_register_interrupt_callback(0x20, idt_clock);

    // Load the interrupt descriptor table
    idt_load(&idtr_descriptor);

    println("Initialised IDT");
}

// For C functions
int idt_register_interrupt_callback(int interrupt_no, INTERRUPT_CALLBACK_FUNCTION callback_function) {
    
    if (interrupt_no < 0 || interrupt_no >= PEACHOS_TOTAL_INTERRUPTS) {
        return -EINVARG;
    }

    interrupt_callbacks[interrupt_no] = callback_function;
    return 0;
}

void isr80h_register_command(int command_id, ISR80H_COMMAND command_func)
{
    if (command_id < 0 || command_id >= PEACHOS_MAX_ISR80H_COMMANDS)
    {
        // Invalid command
        panic("isr80h_register_command(): Command is out of bounds");
    }

    if (isr80h_commands[command_id])
    {
        panic("isr80h_register_command(): Attempting to overwrite existing command");
    }

    isr80h_commands[command_id] = command_func;
}

void *isr80h_handle_command(int command, struct interrupt_frame *frame)
{
    void *result = 0;

    if (command < 0 || command >= PEACHOS_MAX_ISR80H_COMMANDS)
    {
        // Invalid command
        return 0;
    }

    ISR80H_COMMAND command_func = isr80h_commands[command];

    if (!command_func)
    {
        return 0;
    }

    result = command_func(frame);
    // print("Called command:");
    // const char command_char = int_to_char(command);
    // print_char(&command_char);

    return result;
}

void *isr80h_handler(int command, struct interrupt_frame *frame)
{
    void *res = 0;
    kernel_page(); // Switch to the kernel page

    // Save the registers into the task object, helps mainly with task switching.
    // Must be called from kernel land since the task objects exist in the kernel page
    task_current_save_state(frame);

    res = isr80h_handle_command(command, frame); // Execute the correct command function
    task_page();                                 // Switch back to the task page
    // The assembly function 'isr80h_wrapper', which called this function, restores the user general purpose registers
    return res;
}