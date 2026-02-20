/*
 * fb_console.h - Framebuffer console header
 * version 0.0.2
 * Text console for VBE graphics mode
 */

#ifndef FB_CONSOLE_H
#define FB_CONSOLE_H

#include "../../stdint.h"

/* Initialize framebuffer console */
void fb_console_init(void);

/* Print a character to framebuffer console */
void fb_putchar(char c);

/* Print a string to framebuffer console */
void fb_print(const char *str);

/* Print a hex number */
void fb_print_hex(uint32_t value);

/* Print a decimal number */
void fb_print_int(int value);

/* Clear the console */
void fb_console_clear(void);

/* Reset cursor to top-left */
void fb_console_reset_cursor(void);

/* Set text color */
void fb_set_text_color(uint32_t fg, uint32_t bg);

/* Flush buffer to screen */
void fb_flush(void);

#endif /* FB_CONSOLE_H */
