/*
 * graphics.c - Graphics driver implementation
 * version 0.0.5
 */

#include "graphics.h"
#include "utils.h"
#include "string.h"

/* Framebuffer info */
static uint32_t *framebuffer = (uint32_t *)0;
static int fb_width = 800;
static int fb_height = 600;
static int fb_pitch = 800 * 4;

/* Double buffer */
static uint32_t double_buffer[800 * 600];

/* External functions from kernel.c */
extern uint32_t *gfx_get_framebuffer_from_multiboot(void);
extern int gfx_get_width_from_multiboot(void);
extern int gfx_get_height_from_multiboot(void);

/*
 * Initialize graphics mode
 */
int graphics_init(void) {
    /* Get framebuffer info from multiboot */
    framebuffer = gfx_get_framebuffer_from_multiboot();
    fb_width = gfx_get_width_from_multiboot();
    fb_height = gfx_get_height_from_multiboot();
    fb_pitch = fb_width * 4;
    
    print("GFX: Framebuffer at ");
    print_hex((unsigned int)framebuffer);
    print("\n");
    print("GFX: Size ");
    print_int(fb_width);
    print("x");
    print_int(fb_height);
    print("\n");
    
    /* Clear the double buffer */
    memset(double_buffer, 0, sizeof(double_buffer));
    
    return 0;
}

/*
 * Set a pixel color
 */
void gfx_set_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < fb_width && y >= 0 && y < fb_height) {
        double_buffer[y * fb_width + x] = color;
    }
}

/*
 * Get a pixel color
 */
uint32_t gfx_get_pixel(int x, int y) {
    if (x >= 0 && x < fb_width && y >= 0 && y < fb_height) {
        return double_buffer[y * fb_width + x];
    }
    return 0;
}

/*
 * Clear screen with color
 */
void gfx_clear(uint32_t color) {
    int i;
    int size = fb_width * fb_height;
    for (i = 0; i < size; i++) {
        double_buffer[i] = color;
    }
}

/*
 * Draw a filled circle
 */
void gfx_draw_circle(int cx, int cy, int radius, uint32_t color) {
    int x, y;
    int r2 = radius * radius;
    
    for (y = -radius; y <= radius; y++) {
        for (x = -radius; x <= radius; x++) {
            if (x * x + y * y <= r2) {
                gfx_set_pixel(cx + x, cy + y, color);
            }
        }
    }
}

/*
 * Create RGB color
 */
uint32_t gfx_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

/*
 * Create HSV color and convert to RGB using integer math
 */
uint32_t gfx_hsv(int h, int s, int v) {
    int region, remainder, p, q, t;
    int r, g, b;
    
    if (s == 0) {
        return gfx_rgb(v, v, v);
    }
    
    while (h < 0) h += 360;
    while (h >= 360) h -= 360;
    
    region = h / 60;
    remainder = ((h % 60) * 255) / 60;
    
    p = (v * (255 - s)) / 255;
    q = (v * (255 - ((s * remainder) / 255))) / 255;
    t = (v * (255 - ((s * (255 - remainder)) / 255))) / 255;
    
    switch (region) {
        case 0:  r = v; g = t; b = p; break;
        case 1:  r = q; g = v; b = p; break;
        case 2:  r = p; g = v; b = t; break;
        case 3:  r = p; g = q; b = v; break;
        case 4:  r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    
    return gfx_rgb(r, g, b);
}

/*
 * Get framebuffer address
 */
uint32_t *gfx_get_framebuffer(void) {
    return framebuffer;
}

/*
 * Swap buffers - copy double buffer to screen
 */
void gfx_swap_buffers(void) {
    if (framebuffer) {
        memcpy(framebuffer, double_buffer, fb_width * fb_height * 4);
    }
}
