#include <stddef.h>
#include "stdlib.h"
#include "peachos.h"

void* malloc(size_t size) {
    return peachos_malloc(size); // Should return 0 if no allocation is possible
}

void free(void* ptr) {
    peachos_free(ptr);
}

// Integer to ASCII (int to string)
char* itoa(int i) {
    static char text[12]; // static, so not on the stack
    int loc = 11; // Set to end of string (we operate this function from lower to higher digits)
    text[11] = 0; // Terminating character
    char neg = 1;

    if (i >= 0) {
        neg = 0;
        i = -i; // Why do we do this, then do ('0' - (i % 10)) below?
    }

    while(i)
    {
        text[--loc] = '0' - (i % 10); // Set character to its ASCII equivalent and shift location/index
        i /= 10; // Dividing by 10 essentially shifts the number rightwards
        // Keep doing this as long as i isn't yet 0
    }

    if (loc == 11)
        text[--loc] = '0'; // If loc is still 0 (i.e, supplied i was 0) then just set character to 0

    if (neg)
        text[--loc] = '-'; // Add '-' symbol if i is negative

    return &text[loc]; // Return address starting from loc (so we don't end up with unnecessary trailing characters)
}