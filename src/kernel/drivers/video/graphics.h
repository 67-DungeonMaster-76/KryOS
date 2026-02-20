/*
 * graphics.h - Graphics driver header
 * version 0.0.2
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "../../stdint.h"

/* Screen dimensions */
#define GFX_WIDTH  800
#define GFX_HEIGHT 600
#define GFX_BPP    32   /* Bits per pixel */

/* VBE mode number for 800x600x32 */
#define VBE_MODE_800x600x32 0x115

/* Color structure */
typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
} color_t;

/* Initialize graphics mode */
int graphics_init(void);

/* Set a pixel color */
void gfx_set_pixel(int x, int y, uint32_t color);

/* Get a pixel color from double buffer */
uint32_t gfx_get_pixel(int x, int y);

/* Get pixel from actual framebuffer */
uint32_t gfx_get_screen_pixel(int x, int y);

/* Clear screen with color */
void gfx_clear(uint32_t color);

/* Clear only dirty region */
void gfx_clear_dirty(uint32_t color);

/* Mark entire screen as dirty */
void gfx_mark_all_dirty(void);

/* Draw a filled circle */
void gfx_draw_circle(int cx, int cy, int radius, uint32_t color);

/* Create RGB color */
uint32_t gfx_rgb(uint8_t r, uint8_t g, uint8_t b);

/* Create HSV color and convert to RGB (integer version)
 * h: 0-359 (hue), s: 0-255 (saturation), v: 0-255 (value) */
uint32_t gfx_hsv(int h, int s, int v);

/* Get framebuffer address */
uint32_t *gfx_get_framebuffer(void);

/* Get screen dimensions */
int gfx_get_width(void);
int gfx_get_height(void);

/* Swap buffers - copy only dirty region */
void gfx_swap_buffers(void);

/* Force full screen swap */
void gfx_swap_buffers_full(void);

/* Fill a rectangle with a color (optimized batch operation) */
void gfx_fill_rect(int x, int y, int width, int height, uint32_t color);

/* Copy a rectangular region (for scrolling) */
void gfx_copy_rect(int src_x, int src_y, int dst_x, int dst_y, int width, int height);

/* Draw a horizontal line (optimized) */
void gfx_draw_hline(int x, int y, int length, uint32_t color);

/* Get direct access to double buffer (for fast character rendering) */
uint32_t *gfx_get_double_buffer(void);

#endif /* GRAPHICS_H */
