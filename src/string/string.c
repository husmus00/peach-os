#include "string.h"

char tolower(char c) {
    // Uppercase starts at decimal 65, lowercase at decimal 97
    int CASE_DIF = 32;

    if (c >= 65 && c <= 90) {
        c += CASE_DIF;
    }

    return c;
}

size_t strlen(const char* str) {
    size_t len = 0;

    while(str[len] != 0) {
        len++;
    }

    return len;
}

// Gets length of string (until NULL) up to a defined max length
size_t strnlen(const char* str, int max) {
    int i = 0;
    for (i = 0; i < max; i++) {
        if (str[i] == 0) {
            break;
        }
    }
    return i;
}

// Gets length of string (until NULL or 'terminator') up to a defined max length
int strnlen_terminator(const char* str, int max, char terminator) {
    int i = 0;
    while(max-- != 0) {
        if (*str++ == '\0' || *str++ == terminator) {
            i++;
            break;
        }
    }
    return i;
}

// Compare strings up to a defined max length
int strncmp(const char* str1, const char* str2, int max) {
    unsigned char u1, u2;

    while (max-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}

// Compares two case-insensitive
int strncmp_insensitive(const char* str1, const char* str2, int max) {
    unsigned char u1, u2;
    
    while(max-- != 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;

        if (u1 != u2 && tolower(u1) != tolower(u2)) {
            return u1 - u2;
        }

        if (u1 == '\0') {
            return 0;
        }
    }

    return 0;
}

char* strcpy(char* dest, const char* src) {
    char* res = dest;

    while(*src != 0) {
        *dest = *src;
        src += 1;
        dest += 1;
    }

    *dest = 0x00; // Append the null terminator to the new string

    return res;
}

char* strncpy(char* dest, const char* src, int length) {
    int i = 0;
    for (i = 0; i < length - 1; i++)
    {
        if (src[i] == 0x00)
            break;

        dest[i] = src[i];
    }
    dest[i] = 0x00; // Append the null terminator to the new string
    
    return dest;
}

bool is_digit(char c) {
    return (c >= 48 && c <= 57);
}

bool is_ascii_char(char c) {
    return (c >= 65 && c <= 122);
}

int to_numeric_digit(char c) {
    return c - 48;
}

int int_to_char(int i) {
    return i + 48;
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