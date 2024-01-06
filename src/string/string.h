#if !defined(STRING_H)
#define STRING_H

#include <stddef.h>
#include <stdbool.h>
char tolower(char c);
size_t strlen(const char* str);
size_t strnlen(const char* str, int max);
int strnlen_terminator(const char* str, int max, char terminator);
int strncmp(const char* str1, const char* str2, int max);
int strncmp_insensitive(const char* str1, const char* str2, int max);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, int length);
bool is_digit(char c);
bool is_ascii_char(char c);
int to_numeric_digit(char c);
int int_to_char(int i);
char* itoa(int i);

#endif // STRING_H
