/*
 * utils.c - Utility functions implementation
 * version 0.0.3
 */

#include "utils.h"
#include "string.h"

/* Current cursor position */
static int cursor_x = 0;
static int cursor_y = 0;

/* Current color attribute */
static unsigned char current_color = WHITE_TXT;

/* VGA hardware ports */
#define VGA_CTRL_REGISTER 0x3D4
#define VGA_DATA_REGISTER 0x3D5
#define VGA_CURSOR_HIGH   0x0E
#define VGA_CURSOR_LOW    0x0F

/*
 * Update hardware cursor position
 */
static void update_hardware_cursor(void) {
    unsigned short position = cursor_y * SCREEN_WIDTH + cursor_x;
    
    outb(VGA_CTRL_REGISTER, VGA_CURSOR_HIGH);
    outb(VGA_DATA_REGISTER, (unsigned char)(position >> 8));
    outb(VGA_CTRL_REGISTER, VGA_CURSOR_LOW);
    outb(VGA_DATA_REGISTER, (unsigned char)(position & 0xFF));
}

/*
 * Initialize VGA driver
 */
void vga_init(void) {
    cursor_x = 0;
    cursor_y = 0;
    current_color = WHITE_TXT;
    vga_clear_screen();
}

/*
 * Clear the screen
 */
void vga_clear_screen(void) {
    char *vidmem = (char *)VIDEO_MEMORY;
    unsigned int i = 0;
    
    while (i < (SCREEN_WIDTH * SCREEN_HEIGHT * 2)) {
        vidmem[i] = ' ';
        i++;
        vidmem[i] = current_color;
        i++;
    }
    
    cursor_x = 0;
    cursor_y = 0;
    update_hardware_cursor();
}

/*
 * Set text color (foreground and background)
 */
void vga_set_color(unsigned char fg, unsigned char bg) {
    current_color = (bg << 4) | (fg & 0x0F);
}

/*
 * Set cursor position
 */
void vga_set_cursor(int x, int y) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        cursor_x = x;
        cursor_y = y;
        update_hardware_cursor();
    }
}

/*
 * Get current cursor position
 */
void vga_get_cursor(int *x, int *y) {
    *x = cursor_x;
    *y = cursor_y;
}

/*
 * Scroll screen up by one line
 */
void vga_scroll_up(void) {
    char *vidmem = (char *)VIDEO_MEMORY;
    unsigned int i;
    
    /* Copy all lines up by one */
    for (i = 0; i < (SCREEN_WIDTH * (SCREEN_HEIGHT - 1) * 2); i++) {
        vidmem[i] = vidmem[i + SCREEN_WIDTH * 2];
    }
    
    /* Clear the last line */
    for (i = (SCREEN_WIDTH * (SCREEN_HEIGHT - 1) * 2); 
         i < (SCREEN_WIDTH * SCREEN_HEIGHT * 2); i += 2) {
        vidmem[i] = ' ';
        vidmem[i + 1] = current_color;
    }
    
    cursor_y = SCREEN_HEIGHT - 1;
}

/*
 * Print a single character
 */
void print_char(char c) {
    char *vidmem = (char *)VIDEO_MEMORY;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~(4 - 1);
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            unsigned int pos = (cursor_y * SCREEN_WIDTH + cursor_x) * 2;
            vidmem[pos] = ' ';
            vidmem[pos + 1] = current_color;
        }
    } else {
        unsigned int pos = (cursor_y * SCREEN_WIDTH + cursor_x) * 2;
        vidmem[pos] = c;
        vidmem[pos + 1] = current_color;
        cursor_x++;
    }
    
    /* Handle line wrap */
    if (cursor_x >= SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    /* Handle scroll */
    if (cursor_y >= SCREEN_HEIGHT) {
        vga_scroll_up();
    }
    
    update_hardware_cursor();
}

/*
 * Print a string
 */
void print(const char *message) {
    while (*message != 0) {
        print_char(*message);
        message++;
    }
}

/*
 * Print a string at specific position
 */
void print_at(const char *message, int x, int y) {
    vga_set_cursor(x, y);
    print(message);
}

/*
 * Convert nibble to hex character (inline to avoid array)
 */
static char nibble_to_hex(unsigned char nibble) {
    nibble &= 0x0F;
    if (nibble < 10) {
        return '0' + nibble;
    } else {
        return 'A' + (nibble - 10);
    }
}

/*
 * Print an integer (without using local arrays)
 */
void print_int(int num) {
    unsigned int unum;
    unsigned int divisor;
    int started = 0;
    
    if (num < 0) {
        print_char('-');
        unum = (unsigned int)(-num);
    } else {
        unum = (unsigned int)num;
    }
    
    /* Print digits from highest to lowest */
    for (divisor = 1000000000; divisor > 0; divisor /= 10) {
        unsigned char digit = (unsigned char)(unum / divisor);
        unum %= divisor;
        
        if (digit != 0 || started || divisor == 1) {
            print_char('0' + digit);
            started = 1;
        }
    }
}

/*
 * Print a hexadecimal number (without using local arrays)
 */
void print_hex(unsigned int num) {
    int i;
    
    print_char('0');
    print_char('x');
    
    /* Print each nibble from high to low */
    for (i = 28; i >= 0; i -= 4) {
        print_char(nibble_to_hex((num >> i) & 0xF));
    }
}

/*
 * Busy-wait delay in milliseconds
 * Note: This is approximate and depends on CPU speed
 */
void wait(unsigned int milliseconds) {
    /* Roughly calibrated delay loop for ~1ms per iteration */
    /* Adjust the inner loop count based on your CPU speed */
    volatile unsigned int i, j;
    
    for (i = 0; i < milliseconds; i++) {
        for (j = 0; j < 10000; j++) {
            /* Busy wait - volatile prevents optimization */
            __asm__ __volatile__("nop");
        }
    }
}

/*
 * Sleep in seconds
 */
void sleep(unsigned int seconds) {
    wait(seconds * 1000);
}

/*
 * Check if running in QEMU
 * QEMU has specific signatures in BIOS area and CPUID
 */
int is_qemu(void) {
    /* Check for QEMU signature in BIOS ROM area */
    /* QEMU stores "QEMU" at physical address 0xFFFF5 */
    char *bios_sig = (char *)0xFFFF5;
    
    /* Check for "QEMU" signature */
    if (bios_sig[0] == 'Q' && bios_sig[1] == 'E' && 
        bios_sig[2] == 'M' && bios_sig[3] == 'U') {
        return 1;
    }
    
    /* Alternative: Check CPUID for QEMU */
    /* QEMU returns "QEMUQEMUQEMUQEMU" in CPUID leaf 0x40000000 */
    unsigned int eax, ebx, ecx, edx;
    __asm__ __volatile__(
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0x40000000)
    );
    
    /* QEMU signature: ebx="QEMU", ecx="QEMU", edx="QEMU" */
    if (ebx == 0x554D4551 || ecx == 0x554D4551 || edx == 0x554D4551) {
        return 1;
    }
    
    return 0;
}

/*
 * Shutdown the system
 * Uses different methods for QEMU vs real hardware
 */
void shutdown(void) {
    /* Check if QEMU */
    if (is_qemu()) {
        /* QEMU shutdown via ISA debug exit device */
        /* This causes QEMU to exit immediately */
        outw(0x604, 0x2000);  /* QEMU ACPI shutdown */
        outw(0xB004, 0x2000);  /* Alternative QEMU/Bochs shutdown */
        outb(0xF4, 0x00);  /* QEMU debug exit */
    }
    
    /* Real hardware: Try ACPI shutdown */
    /* ACPI shutdown: write 0x2000 to port 0x604 or 0x0 to port 0xB004 */
    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);
    
    /* If ACPI fails, try APM */
    outb(0x64, 0xFE);  /* Fast APM shutdown (reset) */
    
    /* If all else fails, halt */
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
