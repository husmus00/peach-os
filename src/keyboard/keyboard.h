#if !defined(KEYBOARD_H)
#define KEYBOARD_H

struct process;

#define KEYBOARD_CAPSLOCK_OFF 0
#define KEYBOARD_CAPSLOCK_ON 1

typedef int KEYBOARD_CAPSLOCK_STATE;

// TODO: use flags instead of these constants for various keyboard states

// Represents a virtual keyboard
typedef int (*KEYBOARD_INIT_FUNCTION)();
struct keyboard {
    KEYBOARD_INIT_FUNCTION init;
    char name[32];
    KEYBOARD_CAPSLOCK_STATE capslock_state;
    struct keyboard* next;
};

int keyboard_insert(struct keyboard* keyboard);
void keyboard_init();
void keyboard_backspace(struct process* process);
void keyboard_set_capslock(struct keyboard* keyboard, KEYBOARD_CAPSLOCK_STATE state);
KEYBOARD_CAPSLOCK_STATE keyboard_get_capslock(struct keyboard* keyboard);
void keyboard_push(char c);
char keyboard_pop();

#endif // KEYBOARD_H
