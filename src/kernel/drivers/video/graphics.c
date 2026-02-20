/*
 * graphics.c - Graphics driver implementation
 * version 0.0.7
 * Optimized with SSE for faster memory operations
 */

#include "graphics.h"
#include "../../utils.h"
#include "../../string.h"

/* Framebuffer info */
static uint32_t *framebuffer = (uint32_t *)0;
static int fb_width = 800;
static int fb_height = 600;
static int fb_pitch = 800 * 4;

/* Double buffer - aligned for SSE */
static uint32_t double_buffer[800 * 600] __attribute__((aligned(16)));

/* Dirty rectangle tracking */
static int dirty_x1 = 0, dirty_y1 = 0;
static int dirty_x2 = 799, dirty_y2 = 599;
static int dirty_enabled = 1;  /* Start with dirty enabled */

/* External functions from kernel.c */
extern uint32_t *gfx_get_framebuffer_from_multiboot(void);
extern int gfx_get_width_from_multiboot(void);
extern int gfx_get_height_from_multiboot(void);

/*
 * SSE-optimized memory copy (16 bytes at a time)
 * Uses movups for unaligned access
 */
static void sse_memcpy(void *dst, const void *src, int size) {
    int i;
    uint8_t *d = (uint8_t *)dst;
    uint8_t *s = (uint8_t *)src;
    
    /* Copy 16 bytes at a time using SSE */
    int chunks = size / 16;
    for (i = 0; i < chunks; i++) {
        __asm__ __volatile__(
            "movups (%0), %%xmm0\n\t"
            "movups %%xmm0, (%1)"
            :
            : "r"(s + i*16), "r"(d + i*16)
            : "xmm0", "memory"
        );
    }
    
    /* Copy remaining bytes */
    for (i = chunks * 16; i < size; i++) {
        d[i] = s[i];
    }
}

/*
 * SSE-optimized memory set (fill with 32-bit value)
 * Uses movups for unaligned access
 */
static void sse_memset32(void *dst, uint32_t value, int count) {
    int i;
    uint32_t *d = (uint32_t *)dst;
    
    /* Fill 4 pixels (16 bytes) at a time using SSE */
    int chunks = count / 4;
    for (i = 0; i < chunks; i++) {
        __asm__ __volatile__(
            "movd %0, %%xmm0\n\t"
            "shufps $0, %%xmm0, %%xmm0\n\t"
            "movups %%xmm0, (%1)"
            :
            : "r"(value), "r"(d + i*4)
            : "xmm0", "memory"
        );
    }
    
    /* Fill remaining pixels */
    for (i = chunks * 4; i < count; i++) {
        d[i] = value;
    }
}

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
    
    /* Clear the double buffer using SSE */
    sse_memset32(double_buffer, 0, fb_width * fb_height);
    
    /* Mark entire screen as dirty initially - force full redraw */
    gfx_mark_all_dirty();
    
    return 0;
}

/*
 * Mark a region as dirty (needs redraw)
 */
static void mark_dirty(int x, int y) {
    if (!dirty_enabled) return;
    
    if (x < dirty_x1) dirty_x1 = x;
    if (y < dirty_y1) dirty_y1 = y;
    if (x > dirty_x2) dirty_x2 = x;
    if (y > dirty_y2) dirty_y2 = y;
}

/*
 * Mark entire screen as dirty
 */
void gfx_mark_all_dirty(void) {
    dirty_x1 = 0;
    dirty_y1 = 0;
    dirty_x2 = fb_width - 1;
    dirty_y2 = fb_height - 1;
    dirty_enabled = 1;
}

/*
 * Set a pixel color
 */
void gfx_set_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < fb_width && y >= 0 && y < fb_height) {
        double_buffer[y * fb_width + x] = color;
        mark_dirty(x, y);
    }
}

/*
 * Get a pixel color from double buffer
 */
uint32_t gfx_get_pixel(int x, int y) {
    if (x >= 0 && x < fb_width && y >= 0 && y < fb_height) {
        return double_buffer[y * fb_width + x];
    }
    return 0;
}

/*
 * Get pixel from actual framebuffer (for reading current screen state)
 */
uint32_t gfx_get_screen_pixel(int x, int y) {
    if (x >= 0 && x < fb_width && y >= 0 && y < fb_height && framebuffer) {
        return framebuffer[y * fb_width + x];
    }
    return 0;
}

/*
 * Clear screen with color (marks entire screen dirty)
 * Uses SSE for faster clearing
 */
void gfx_clear(uint32_t color) {
    sse_memset32(double_buffer, color, fb_width * fb_height);
    gfx_mark_all_dirty();
}

/*
 * Clear only dirty region (dirty triangle effect)
 * Fills only the area that has been modified
 */
void gfx_clear_dirty(uint32_t color) {
    int x, y;
    for (y = dirty_y1; y <= dirty_y2; y++) {
        for (x = dirty_x1; x <= dirty_x2; x++) {
            double_buffer[y * fb_width + x] = color;
        }
    }
    /* Reset dirty region */
    dirty_x1 = fb_width;
    dirty_y1 = fb_height;
    dirty_x2 = 0;
    dirty_y2 = 0;
    dirty_enabled = 0;
}

/*
 * Draw a filled circle (optimized with horizontal lines)
 */
void gfx_draw_circle(int cx, int cy, int radius, uint32_t color) {
    int y;
    int r2 = radius * radius;
    
    for (y = -radius; y <= radius; y++) {
        /* Calculate width of this scanline using circle equation */
        int y2 = y * y;
        int max_x = 0;
        int x;
        
        /* Find the rightmost x for this y */
        for (x = 0; x <= radius; x++) {
            if (x * x + y2 <= r2) {
                max_x = x;
            }
        }
        
        /* Draw horizontal line from -max_x to +max_x */
        if (max_x > 0) {
            gfx_draw_hline(cx - max_x, cy + y, max_x * 2 + 1, color);
        } else if (max_x == 0) {
            gfx_set_pixel(cx, cy + y, color);
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
 * Get screen dimensions
 */
int gfx_get_width(void) {
    return fb_width;
}

int gfx_get_height(void) {
    return fb_height;
}

/*
 * Swap buffers - copy only dirty region to screen
 * Uses SSE for faster copying
 */
void gfx_swap_buffers(void) {
    if (!framebuffer) return;
    
    /* If no dirty region or invalid, do full swap */
    if (!dirty_enabled || dirty_x1 > dirty_x2 || dirty_y1 > dirty_y2) {
        gfx_swap_buffers_full();
        return;
    }
    
    /* Copy only the dirty rectangle using SSE */
    int y;
    int row_bytes = (dirty_x2 - dirty_x1 + 1) * 4;
    
    for (y = dirty_y1; y <= dirty_y2; y++) {
        uint32_t *src = &double_buffer[y * fb_width + dirty_x1];
        uint32_t *dst = &framebuffer[y * fb_width + dirty_x1];
        sse_memcpy(dst, src, row_bytes);
    }
    
    /* Reset dirty region after swap */
    dirty_x1 = fb_width;
    dirty_y1 = fb_height;
    dirty_x2 = 0;
    dirty_y2 = 0;
    dirty_enabled = 0;
}

/*
 * Force full screen swap (copy entire buffer)
 * Uses SSE for faster copying
 */
void gfx_swap_buffers_full(void) {
    if (framebuffer) {
        sse_memcpy(framebuffer, double_buffer, fb_width * fb_height * 4);
    }
    /* Reset dirty region */
    dirty_x1 = fb_width;
    dirty_y1 = fb_height;
    dirty_x2 = 0;
    dirty_y2 = 0;
    dirty_enabled = 0;
}

/*
 * Fill a rectangle with a color (optimized batch operation)
 * Uses SSE for faster filling
 */
void gfx_fill_rect(int x, int y, int width, int height, uint32_t color) {
    int row;
    
    /* Clip to screen bounds */
    if (x < 0) { width += x; x = 0; }
    if (y < 0) { height += y; y = 0; }
    if (x + width > fb_width) width = fb_width - x;
    if (y + height > fb_height) height = fb_height - y;
    
    if (width <= 0 || height <= 0) return;
    
    /* Fill each row using SSE */
    for (row = 0; row < height; row++) {
        uint32_t *row_ptr = &double_buffer[(y + row) * fb_width + x];
        sse_memset32(row_ptr, color, width);
    }
    
    /* Mark region as dirty */
    if (dirty_enabled) {
        if (x < dirty_x1) dirty_x1 = x;
        if (y < dirty_y1) dirty_y1 = y;
        if (x + width - 1 > dirty_x2) dirty_x2 = x + width - 1;
        if (y + height - 1 > dirty_y2) dirty_y2 = y + height - 1;
    }
}

/*
 * Copy a rectangular region (for scrolling)
 * Uses SSE for faster copying
 */
void gfx_copy_rect(int src_x, int src_y, int dst_x, int dst_y, int width, int height) {
    int row;
    
    /* Clip to screen bounds */
    if (src_x < 0 || src_y < 0 || dst_x < 0 || dst_y < 0) return;
    if (src_x + width > fb_width || dst_x + width > fb_width) return;
    if (src_y + height > fb_height || dst_y + height > fb_height) return;
    
    /* Handle overlapping regions (scrolling up vs down) */
    if (src_y < dst_y) {
        /* Copy from bottom to top to avoid overwriting source */
        for (row = height - 1; row >= 0; row--) {
            uint32_t *src = &double_buffer[(src_y + row) * fb_width + src_x];
            uint32_t *dst = &double_buffer[(dst_y + row) * fb_width + dst_x];
            sse_memcpy(dst, src, width * 4);
        }
    } else {
        /* Copy from top to bottom */
        for (row = 0; row < height; row++) {
            uint32_t *src = &double_buffer[(src_y + row) * fb_width + src_x];
            uint32_t *dst = &double_buffer[(dst_y + row) * fb_width + dst_x];
            sse_memcpy(dst, src, width * 4);
        }
    }
    
    /* Mark destination region as dirty */
    if (dirty_enabled) {
        if (dst_x < dirty_x1) dirty_x1 = dst_x;
        if (dst_y < dirty_y1) dirty_y1 = dst_y;
        if (dst_x + width - 1 > dirty_x2) dirty_x2 = dst_x + width - 1;
        if (dst_y + height - 1 > dirty_y2) dirty_y2 = dst_y + height - 1;
    }
}

/*
 * Draw a horizontal line (optimized with SSE)
 */
void gfx_draw_hline(int x, int y, int length, uint32_t color) {
    /* Clip to screen bounds */
    if (y < 0 || y >= fb_height) return;
    if (x < 0) { length += x; x = 0; }
    if (x + length > fb_width) length = fb_width - x;
    
    if (length <= 0) return;
    
    /* Fill the line using SSE */
    uint32_t *line_ptr = &double_buffer[y * fb_width + x];
    sse_memset32(line_ptr, color, length);
    
    /* Mark as dirty */
    if (dirty_enabled) {
        if (x < dirty_x1) dirty_x1 = x;
        if (y < dirty_y1) dirty_y1 = y;
        if (x + length - 1 > dirty_x2) dirty_x2 = x + length - 1;
        if (y > dirty_y2) dirty_y2 = y;
    }
}

/*
 * Get direct access to double buffer (for fast character rendering)
 */
uint32_t *gfx_get_double_buffer(void) {
    return double_buffer;
}
