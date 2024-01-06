#if !defined(CLASSIC_KEYBOARD_H)
#define CLASSIC_KEYBOARD_H

#define KEYBOARD_INPUT_PORT 0x60 // Data port for PS/2
#define PS2_PORT 0x64            // Command register for PS/2
#define PS2_COMMAND_ENABLE_FIRST_PORT 0xAE

#define ISR_KEYBOARD_INTERRUPT 0x21

// Bitmasks for the scancodes
#define CLASSIC_KEYBOARD_KEY_RELEASED 0x80

struct keyboard* classic_init();

#endif // CLASSIC_KEYBOARD_H
