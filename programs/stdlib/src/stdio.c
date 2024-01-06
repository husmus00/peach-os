#include "peachos.h"
#include "stdio.h"
#include "stdlib.h"
#include <stdarg.h>

int putchar(int c) {
    peachos_putchar((char)c);
    return 0;
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
            putchar(*p);
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
                putchar(*p);
                break;
        }
    }

    va_end(ap);

    return 0;
}

void println(const char* str) {
    print(str);
    putchar('\n');
}