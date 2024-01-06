#include <stdint.h>
#include <stdarg.h>
#include "terminal.h"
#include "string/string.h"

uint16_t* video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_col = 0;

void terminal_scroll_down() {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH - 2; x++) {
            video_mem[(y * VGA_WIDTH) + x] = video_mem[((y+1) * VGA_WIDTH) + x];
        }
    }

    // Clear last line
    for (int x = 0; x < VGA_WIDTH - 2; x++) {
        video_mem[((VGA_HEIGHT) * VGA_WIDTH) + x] = 0x00;
    }
}

char terminal_get_current_char() {
    return (char)video_mem[(terminal_row * VGA_WIDTH) + terminal_col];
}

// Colours:
// Grey = 7
uint16_t terminal_make_char(char c, char colour) {
    return (colour << 8) | c;
}

void terminal_putchar(int x, int y, char c, char colour) {
    video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(c, colour);
}

void terminal_backspace() {
    if (terminal_row == 0 && terminal_col == 0) {
        // Can't remove any more text
        return;
    }

    if (terminal_col == 1) {
        // Reached the beginning of a row
        terminal_col = VGA_WIDTH - 1;
        terminal_row -= 1;
    }

    terminal_col -= 1;
    terminal_writechar(' ', TERMINAL_DEFAULT_COLOUR);
    terminal_col -= 1;
}

void terminal_writechar(char c, char colour) {

    // Test if screen needs scrolling
    if (terminal_row > VGA_HEIGHT) {
        terminal_scroll_down();
        terminal_row--;
    }

    // Test for newline character (0x0D is the scancode)
    if (c == '\n' || c == 0x0D) {
        terminal_row += 1;
        terminal_col = TERMINAL_COL_DEFAULT;
        return;
    }

    // Test for backspace
    if (c == 0x08) {
        terminal_backspace();
        // while (terminal_get_current_char() == 0xB8000) {
        //     terminal_backspace();
        // }
        return;
    }

    // All other characters
    terminal_putchar(terminal_col, terminal_row, c, colour);

    if (terminal_col < VGA_WIDTH - 2) {
        terminal_col++;
    }
    else {
        terminal_col = TERMINAL_COL_DEFAULT;
        terminal_row++;
    }
}

void print_colour(const char* str, char colour) {
    size_t len = strlen(str);
    for (int i = 0; i < len; i++) {
        terminal_writechar(str[i], colour);
    }
}

void print(const char* str) {
    // print_colour(":: ", TERMINAL_DEFAULT_COLOUR); // TODO: distingush kernel from userland print
    print_colour(str, TERMINAL_DEFAULT_COLOUR);
}

void println(const char* str) {
    print(str);
    terminal_writechar('\n', TERMINAL_DEFAULT_COLOUR);
}

void print_char(int c) {
    terminal_writechar((char)c, TERMINAL_DEFAULT_COLOUR);
}

int printf(const char *fmt, ...) {
    va_list ap;
    const char *p;
    char* sval;
    int ival;

    va_start(ap, fmt);
    for (p = fmt; *p; p++) {
        // Keep prcoessing until we hit the null terminator
        if (*p != '%') {
            // If the character is not the formatting character '%' just print the normal character
            print_char(*p);
            continue;
        }

        switch(*++p) {
            case 'i': // %i = integer
                ival = va_arg(ap, int); // Read the next available argument as an integer
                print(itoa(ival)); // Print the int as a character
                break;

            case 's': // %s = string
                sval = va_arg(ap, char *); // Read the next available argument as a string
                print(sval);
                break;

            default:
                print_char(*p);
                break;
        }
    }

    va_end(ap);

    return 0;
}

void terminal_initialize() {
    video_mem = (uint16_t*)(0xB8000);
    terminal_row = TERMINAL_ROW_DEFAULT; // Not 0 to start with space above
    terminal_col = TERMINAL_COL_DEFAULT; // Not 0 to start with space to the left

    // Clear the terminal
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            // video_mem[(y * VGA_WIDTH) + x] = terminal_make_char(' ', 0);
            terminal_putchar(x, y, ' ', 0);
        }
    }

    println("Initialised terminal");
}