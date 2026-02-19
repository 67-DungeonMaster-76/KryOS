/*
 * keyboard.c - Keyboard driver implementation
 * version 0.0.1
 */

#include "keyboard.h"
#include "utils.h"

/* Keyboard buffer */
#define KB_BUFFER_SIZE 256
static char kb_buffer[KB_BUFFER_SIZE];
static int kb_buffer_head = 0;
static int kb_buffer_tail = 0;

/* Keyboard state flags */
static unsigned char kb_flags = 0;

/* US QWERTY scancode to ASCII (normal, no shift) */
static const char scancode_normal[128] = {
    0,    0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,   'a', 's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b',  'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2',  '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

/* US QWERTY scancode to ASCII (shift pressed) */
static const char scancode_shift[128] = {
    0,    0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,   'A', 'S',
    'D',  'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|', 'Z', 'X', 'C', 'V',
    'B',  'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2',  '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

/* Check if character is a letter */
static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/* Convert to uppercase */
static char to_upper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 32;
    }
    return c;
}

/* Convert to lowercase */
static char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c;
}

/*
 * Keyboard interrupt handler
 * Called from IRQ1 (interrupt 33)
 */
void keyboard_handler(void) {
    unsigned char scancode;
    unsigned char released = 0;
    char ascii = 0;
    
    /* Read scancode from keyboard */
    scancode = inb(0x60);
    
    /* Check if key was released */
    if (scancode & KEY_FLAG_RELEASED) {
        released = 1;
        scancode &= ~KEY_FLAG_RELEASED;
    }
    
    /* Handle modifier keys */
    switch (scancode) {
        case KEY_LEFT_SHIFT:
        case KEY_RIGHT_SHIFT:
            if (released) {
                kb_flags &= ~KEY_FLAG_SHIFT;
            } else {
                kb_flags |= KEY_FLAG_SHIFT;
            }
            return;
            
        case KEY_CAPS_LOCK:
            if (!released) {
                kb_flags ^= KEY_FLAG_CAPS;
            }
            return;
            
        case KEY_LEFT_CTRL:
            if (released) {
                kb_flags &= ~KEY_FLAG_CTRL;
            } else {
                kb_flags |= KEY_FLAG_CTRL;
            }
            return;
            
        case KEY_LEFT_ALT:
            if (released) {
                kb_flags &= ~KEY_FLAG_ALT;
            } else {
                kb_flags |= KEY_FLAG_ALT;
            }
            return;
    }
    
    /* Only process key press (not release) */
    if (released) {
        return;
    }
    
    /* Get ASCII character based on shift state */
    if (kb_flags & KEY_FLAG_SHIFT) {
        ascii = scancode_shift[scancode];
    } else {
        ascii = scancode_normal[scancode];
    }
    
    /* Apply caps lock to letters */
    if ((kb_flags & KEY_FLAG_CAPS) && is_alpha(ascii)) {
        if (kb_flags & KEY_FLAG_SHIFT) {
            ascii = to_lower(ascii);
        } else {
            ascii = to_upper(ascii);
        }
    }
    
    /* Add to buffer if valid character */
    if (ascii != 0) {
        int next_head = (kb_buffer_head + 1) % KB_BUFFER_SIZE;
        if (next_head != kb_buffer_tail) {
            kb_buffer[kb_buffer_head] = ascii;
            kb_buffer_head = next_head;
        }
    }
}

/*
 * Initialize keyboard driver
 */
void keyboard_init(void) {
    kb_buffer_head = 0;
    kb_buffer_tail = 0;
    kb_flags = 0;
    
    /* Enable IRQ1 (keyboard) */
    unsigned char mask = inb(0x21);
    mask &= ~0x02;  /* Clear bit 1 to enable IRQ1 */
    outb(0x21, mask);
}

/*
 * Check if a key is available
 */
int keyboard_has_key(void) {
    return kb_buffer_head != kb_buffer_tail;
}

/*
 * Get a character from keyboard (blocking)
 */
char keyboard_getchar(void) {
    while (!keyboard_has_key()) {
        __asm__ __volatile__("hlt");
    }
    
    char c = kb_buffer[kb_buffer_tail];
    kb_buffer_tail = (kb_buffer_tail + 1) % KB_BUFFER_SIZE;
    return c;
}

/*
 * Get current key state flags
 */
unsigned char keyboard_get_flags(void) {
    return kb_flags;
}
