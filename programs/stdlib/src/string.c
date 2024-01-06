#include "string.h"

// char tolower(char c) {
//     // Uppercase starts at decimal 65, lowercase at decimal 97
//     int CASE_DIF = 32;

//     if (c >= 65 && c <= 90) {
//         c += CASE_DIF;
//     }

//     return c;
// }

// size_t strlen(const char* str) {
//     size_t len = 0;

//     while(str[len] != 0) {
//         len++;
//     }

//     return len;
// }

// // Gets length of string (until NULL) up to a defined max length
// size_t strnlen(const char* str, int max) {
//     int i = 0;
//     for (i = 0; i < max; i++) {
//         if (str[i] == 0) {
//             break;
//         }
//     }
//     return i;
// }

// // Gets length of string (until NULL or 'terminator') up to a defined max length
// int strnlen_terminator(const char* str, int max, char terminator) {
//     int i = 0;
//     while(max-- != 0) {
//         if (*str++ == '\0' || *str++ == terminator) {
//             i++;
//             break;
//         }
//     }
//     return i;
// }

// // Compare strings up to a defined max length
// int strncmp(const char* str1, const char* str2, int max) {
//     unsigned char u1, u2;

//     while (max-- > 0) {
//         u1 = (unsigned char)*str1++;
//         u2 = (unsigned char)*str2++;
//         if (u1 != u2)
//             return u1 - u2;
//         if (u1 == '\0')
//             return 0;
//     }

//     return 0;
// }

// // Compares two case-insensitive
// int strncmp_insensitive(const char* str1, const char* str2, int max) {
//     unsigned char u1, u2;
    
//     while(max-- != 0) {
//         u1 = (unsigned char)*str1++;
//         u2 = (unsigned char)*str2++;

//         if (u1 != u2 && tolower(u1) != tolower(u2)) {
//             return u1 - u2;
//         }

//         if (u1 == '\0') {
//             return 0;
//         }
//     }

//     return 0;
// }

// char* strcpy(char* dest, const char* src) {
//     char* res = dest;

//     while(*src != 0) {
//         *dest = *src;
//         src += 1;
//         dest += 1;
//     }

//     *dest = 0x00; // Append the null terminator to the new string

//     return res;
// }

// char* strncpy(char* dest, const char* src, int length) {
//     int i = 0;
//     for (i = 0; i < length - 1; i++)
//     {
//         if (src[i] == 0x00)
//             break;

//         dest[i] = src[i];
//     }
//     dest[i] = 0x00; // Append the null terminator to the new string
    
//     return dest;
// }

// bool is_digit(char c) {
//     return c >= 48 && c <= 57;
// }

// int to_numeric_digit(char c) {
//     return c - 48;
// }

// int int_to_char(int i) {
//     return i + 48;
// }

// // global variable
// char* sp = 0;
// // Seperates a string based on the supplied delimiters
// char* strtok(char* str, const char* delimiters) {
//     int i = 0;
//     int len = strlen(delimiters);

//     if (!str && !sp)
//         return 0;

//     if (str && !sp) {
//         sp = str;
//     }

//     char* p_start = sp;
//     // Check if the character we're currently on is equal to any of the delimiters
//     while(1) {
//         for (i = 0; i < len; i++) {
//             if (*p_start == delimiters[i]) {
//                 p_start++;
//                 break;
//             }
//         }

//         if (i == len) {
//             sp = p_start;
//             break;
//         }
//     }

//     if (*sp == '\0') {
//         sp = 0;
//         return sp;
//     } 

//     // Find end of substring
//     while(*sp != '\0') {
//         for (i = 0; i < len; i++) {
//             if (*sp == delimiters[i]) {
//                 *sp = '\0';
//                 break;
//             }
//         }

//         sp++;
//         if (i < len)
//             break;
//     }

//     return p_start;
// }
#include "string.h"

char tolower(char s1)
{
    if (s1 >= 65 && s1 <= 90)
    {
        s1 += 32;
    }

    return s1;
}

int strlen(const char* ptr)
{
    int i = 0;
    while(*ptr != 0)
    {
        i++;
        ptr += 1;
    }

    return i;
}

int strnlen(const char* ptr, int max)
{
    int i = 0;
    for (i = 0; i < max; i++)
    {
        if (ptr[i] == 0)
            break;
    }

    return i;
}

int strnlen_terminator(const char* str, int max, char terminator)
{
    int i = 0;
    for(i = 0; i < max; i++)
    {
        if (str[i] == '\0' || str[i] == terminator)
            break;
    }

    return i;
}

int istrncmp(const char* s1, const char* s2, int n)
{
    unsigned char u1, u2;
    while(n-- > 0)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2 && tolower(u1) != tolower(u2))
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}
int strncmp(const char* str1, const char* str2, int n)
{
    unsigned char u1, u2;

    while(n-- > 0)
    {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }

    return 0;
}

char* strcpy(char* dest, const char* src)
{
    char* res = dest;
    while(*src != 0)
    {
        *dest = *src;
        src += 1;
        dest += 1;
    }

    *dest = 0x00;

    return res;
}

char* strncpy(char* dest, const char* src, int count)
{
    int i = 0;
    for (i = 0; i < count-1; i++)
    {
        if (src[i] == 0x00)
            break;

        dest[i] = src[i];
    }

    dest[i] = 0x00;
    return dest;
}

bool isdigit(char c)
{
    return c >= 48 && c <= 57;
}
int tonumericdigit(char c)
{
    return c - 48;
}

char* sp = 0;
char* strtok(char* str, const char* delimiters)
{
    int i = 0;
    int len = strlen(delimiters);
    if (!str && !sp)
        return 0;
    
    if (str && !sp)
    {
        sp = str;
    }

    char* p_start = sp;
    while(1)
    {
        for (i = 0; i < len; i++)
        {
            if(*p_start == delimiters[i])
            {
                p_start++;
                break;
            }
        }

        if (i == len)
        {
            sp = p_start;
            break;
        }
    }

    if (*sp == '\0')
    {
        sp = 0;
        return sp;
    }

    // Find end of substring
    while(*sp != '\0')
    {
        for (i = 0; i < len; i++)
        {
            if (*sp == delimiters[i])
            {
                *sp = '\0';
                break;
            }
        }

        sp++;
        if (i < len)
            break;
    }

    return p_start;
}