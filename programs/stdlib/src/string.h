// #if !defined(PEACHOS_STRING_H)
// #define PEACHOS_STRING_H

// #include <stddef.h>
// #include <stdbool.h>

// char tolower(char c);
// size_t strlen(const char* str);
// size_t strnlen(const char* str, int max);
// int strnlen_terminator(const char* str, int max, char terminator);
// int strncmp(const char* str1, const char* str2, int max);
// int strncmp_insensitive(const char* str1, const char* str2, int max);
// char* strcpy(char* dest, const char* src);
// char* strncpy(char* dest, const char* src, int length);
// bool is_digit(char c);
// int to_numeric_digit(char c);
// int int_to_char(int i);
// char* strtok(char* str, const char* delimiters);

// #endif // PEACHOS_STRING_H

#ifndef PEACHOS_STRING_H
#define PEACHOS_STRING_H

#include <stdbool.h>

char tolower(char s1);
int strlen(const char* ptr);
int strnlen(const char* ptr, int max);
int strnlen_terminator(const char* str, int max, char terminator);
int istrncmp(const char* s1, const char* s2, int n);
int strncmp(const char* str1, const char* str2, int n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, int count);
bool isdigit(char c);
int tonumericdigit(char c);
char* strtok(char* str, const char* delimiters);

#endif
