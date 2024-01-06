#ifndef KERNEL_H
#define KERNEL_H

#include "terminal/terminal.h"

void kernel_main();

void panic(const char* msg);
void kernel_page();
void kernel_registers(); // Assembly function in kernel.asm

#define ERROR(value) (void*)value
#define ERROR_I(value) (int)value
#define ISERR(value) ((int)value < 0)

#endif