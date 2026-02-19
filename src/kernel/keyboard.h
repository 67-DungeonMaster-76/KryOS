/*
 * keyboard.h - Keyboard driver header
 * version 0.0.1
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Special key codes */
#define KEY_NULL        0x00
#define KEY_ESCAPE      0x01
#define KEY_BACKSPACE   0x0E
#define KEY_TAB         0x0F
#define KEY_ENTER       0x1C
#define KEY_LEFT_CTRL   0x1D
#define KEY_LEFT_SHIFT  0x2A
#define KEY_RIGHT_SHIFT 0x36
#define KEY_LEFT_ALT    0x38
#define KEY_CAPS_LOCK   0x3A
#define KEY_F1          0x3B
#define KEY_F2          0x3C
#define KEY_F3          0x3D
#define KEY_F4          0x3E
#define KEY_F5          0x3F
#define KEY_F6          0x40
#define KEY_F7          0x41
#define KEY_F8          0x42
#define KEY_F9          0x43
#define KEY_F10         0x44
#define KEY_NUM_LOCK    0x45
#define KEY_SCROLL_LOCK 0x46
#define KEY_F11         0x57
#define KEY_F12         0x58

/* Key flags */
#define KEY_FLAG_RELEASED   0x80
#define KEY_FLAG_SHIFT      0x01
#define KEY_FLAG_CAPS       0x02
#define KEY_FLAG_CTRL       0x04
#define KEY_FLAG_ALT        0x08

/* Initialize keyboard driver */
void keyboard_init(void);

/* Keyboard interrupt handler - called from IRQ1 */
void keyboard_handler(void);

/* Get a character from keyboard (blocking) */
char keyboard_getchar(void);

/* Check if a key is available */
int keyboard_has_key(void);

/* Get current key state */
unsigned char keyboard_get_flags(void);

#endif /* KEYBOARD_H */
