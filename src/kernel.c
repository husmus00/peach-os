#include <stdint.h>
#include <stddef.h>
#include "kernel.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kernelheap.h"
#include "memory/paging/paging.h"
#include "memory/memory.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "string/string.h"
#include "fs/pparser.h"
#include "fs/file.h"
#include "gdt/gdt.h"
#include "config.h"
#include "task/tss.h"
#include "task/task.h"
#include "task/process.h"
#include "status.h"
#include "isr80h/isr80h.h"
#include "keyboard/keyboard.h"
#include "terminal/terminal.h"

void panic(const char* msg) {
    println("\nKERNEL PANIC:");
    println(msg);

    // TODO: implement a dumping/diagnostics mechanism
    while(1) {};
}

struct tss tss;
struct gdt gdt_real[PEACHOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[PEACHOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},        // NULL segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x9A},  // Kernel code segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0x92},  // Kernel data segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF8},  // User code segment
    {.base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF2},  // User data segment
    {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9}, // TSS segment
};

// The kernel page is linear (all virtual addresses point to the same physical address)
static struct paging_4gb_chunk* kernel_chunk = 0;

// Switch the page directory to the kernel page and restore kernel registers
void kernel_page() {
    kernel_registers(); // Set the segment registers to point to the kernel data segment
    paging_switch(kernel_chunk);
}

void kernel_init() {
    terminal_initialize();

    // Load the GDT
    memset(gdt_real, 0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, PEACHOS_TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real, sizeof(gdt_real));

    // Initialise the kernel heap
    kheap_init();

    // Initialise filesystems
    fs_init();

    // Search and initialise the disks
    disk_search_and_init();

    // Initialise the interrupt descriptor table
    idt_init();

    // Set up the TSS
    memset(&tss, 0x00, sizeof(tss));
    tss.esp0 = 0x600000; // Kernel stack location
    tss.ss0 = KERNEL_DATA_SELECTOR;
    // Load the TSS
    tss_load(0x28); // Location in the GDT

    // Set up paging, switch to kernel paging chunk, and enable paging.
    // Paging is initially set up as linear, and all virtual addresses will point to
    // the same exact physical address until those page entries are modified.
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    paging_switch(kernel_chunk);

    enable_paging();

    // Register all the kernel interrupts
    isr80h_register_all_commands();

    // Initialise all system keyboards
    keyboard_init();

    // char* ptr1 = kzalloc(4096); // Real address
    // paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t) ptr1 | PAGING_ACCESS_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);
    // char* ptr2 = (char*) 0x1000; // Now this virtual address points to ptr1
    // // ptr1 still points to itself after enable_paging because of initial linear setup (ptr1's now virtual address still points to the exact physical address)
    // if (ptr2) {
    //     // To disable unused var error
    // }

    // enable_interrupts(); // Interrupts will be enabled when the first program is loaded

    print("\n");
}

void kernel_main() {
    kernel_init();

    // panic("The system cannot continue");

    char* str = "Booting PeachOS....\n";
    println(str);
    // video_mem[0] = terminal_make_char('A', 7);
    // terminal_putchar(0, 0, 'A', 7);

    // struct path_root* root = pathparser_parse("0:/bin/shell.exe", NULL);

    // if (root) {
    //     print("Path parsing worked!");
    // }

    // struct disk_stream* stream = disk_streamer_new(0);
    // diskstreamer_seek(stream, 0x201);
    // unsigned char c = 0;
    // diskstreamer_read(stream, &c, 1);

    // char buf[20];
    // strcpy(buf, "hello");
    // print("TEST");

    // int fd = fopen("0:/hello.txt", "r");
    // char buf[14];
    // if (fd) {
    //     print("We opened hello.txt!");
    //     fseek(fd, 2, SEEK_SET);
    //     fread(buf, 11, 1, fd);
    //     print(buf);

    //     struct file_stat s;
    //     fstat(fd, &s);
    //     while(1) {
        
    //     }
    // }
    // fclose(fd);

    // terminal_putchar(0, 20, '0', 7);
    // while(1) {
        
    // }

    struct process* process = 0;
    int res = process_load_switch("0:/blank.elf", &process);
    if (res != PEACHOS_ALL_OK) {
        panic("Failed to load blank");
    }
    struct command_argument root_argument;
    strcpy(root_argument.argument, "TWO");
    // struct command_argument second_argument;
    // strcpy(second_argument.argument, "Second!");
    // root_argument.next = &second_argument;
    process_inject_arguments(process, &root_argument);

    //
    //

    struct process* process_2 = 0;
    res = process_load_switch("0:/blank.elf", &process_2);
    if (res != PEACHOS_ALL_OK) {
        panic("Failed to load blank");
    }
    struct command_argument root_argument_2;
    strcpy(root_argument_2.argument, "ONE");
    // struct command_argument second_argument;
    // strcpy(second_argument.argument, "Second!");
    // root_argument.next = &second_argument;
    process_inject_arguments(process_2, &root_argument_2);

    task_run_first_ever_task();

    println("TEST"); // This should not run

    while(1) {
        
    }
}