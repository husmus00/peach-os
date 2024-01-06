#include "memory.h"

void* memset(void* ptr, int c, size_t size) {
    char* c_ptr = (char*) ptr;

    for (int i = 0; i < size; i++) {
        c_ptr[i] = (char) c;
    }

    return ptr;
}

// Compare strings s1 and s2 and return 0 if equal
int memcmp(void* s1, void* s2, int len) {
    char* c1 = (char*)s1;
    char* c2 = (char*)s2;

    // for (int i = 0; i < len; i++) {
    //     if (c1[i] != c2[i]) {
    //         return c1[i] < c2[i] ? -1: 1;
    //     }
    // }

    while(len-- > 0) {
        if (*c1++ != *c2++) {
            return c1[-1] < c2[-1] ? -1 : 1;
        }
    }

    return 0;
}

void* memcpy(void* dest, void* src, int len) {
    char *d = dest;
    char *s = src;
    while(len--) {
        *d++ = *s++;
    }
    return dest;
}