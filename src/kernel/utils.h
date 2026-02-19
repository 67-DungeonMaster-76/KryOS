/*
 * utils.h - Utility functions header
 * version 0.0.1
 */

#ifndef UTILS_H
#define UTILS_H

/* Video driver constants */
#define VIDEO_MEMORY 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define WHITE_TXT 0x07    /* light gray on black text */

/* Color attributes */
#define COLOR_BLACK         0x00
#define COLOR_BLUE          0x01
#define COLOR_GREEN         0x02
#define COLOR_CYAN          0x03
#define COLOR_RED           0x04
#define COLOR_MAGENTA       0x05
#define COLOR_BROWN         0x06
#define COLOR_LIGHT_GRAY    0x07
#define COLOR_DARK_GRAY     0x08
#define COLOR_LIGHT_BLUE    0x09
#define COLOR_LIGHT_GREEN   0x0A
#define COLOR_LIGHT_CYAN    0x0B
#define COLOR_LIGHT_RED     0x0C
#define COLOR_LIGHT_MAGENTA 0x0D
#define COLOR_YELLOW        0x0E
#define COLOR_WHITE         0x0F

/* Video driver functions */
void vga_init(void);
void vga_clear_screen(void);
void vga_set_color(unsigned char fg, unsigned char bg);
void vga_set_cursor(int x, int y);
void vga_get_cursor(int *x, int *y);
void vga_scroll_up(void);

/* Print functions */
void print(const char *message);
void print_at(const char *message, int x, int y);
void print_char(char c);
void print_int(int num);
void print_hex(unsigned int num);

/* Utility functions */
void wait(unsigned int milliseconds);
void sleep(unsigned int seconds);

/* Port I/O functions */
static inline void outb(unsigned short port, unsigned char value) {
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif /* UTILS_H */
