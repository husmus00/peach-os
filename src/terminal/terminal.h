#if !defined(TERMINAL_H)
#define TERMINAL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 20
#define TERMINAL_DEFAULT_COLOUR 7

#define TERMINAL_ROW_DEFAULT 0
#define TERMINAL_COL_DEFAULT 0

void print(const char* str);
void println(const char* str);
void terminal_putchar(int x, int y, char c, char colour);
void terminal_writechar(char c, char colour);
int printf(const char *fmt, ...);
void terminal_backspace();
void terminal_initialize();

#endif // TERMINAL_H
