/*
 * demo.c - Graphics demo implementation
 * version 0.0.9
 * Optimized animated pulsating circle with keyboard exit
 */

#include "demo.h"
#include "drivers/video/graphics.h"
#include "drivers/input/keyboard.h"
#include "drivers/video/fb_console.h"
#include "utils.h"

/*
 * Run rainbow circle demo
 * Displays a pulsating circle that changes size
 * Exits when any key is pressed
 * Optimized: only redraws the area that changed
 */
void demo_rainbow_circle(void) {
    int cx = 800 / 2;
    int cy = 600 / 2;
    int base_radius = 100;
    int max_radius = 200;
    int prev_radius = 0;
    int radius;
    int frame = 0;
    int hue = 0;
    uint32_t color;
    
    /* Clear screen to black once */
    gfx_clear(0x00000000);
    gfx_swap_buffers_full();
    
    /* Main animation loop */
    while (1) {
        /* Check for key press to exit using keyboard buffer */
        if (keyboard_has_key()) {
            /* Consume the key */
            keyboard_getchar();
            /* Clear screen before returning to CLI */
            gfx_clear(0x00000000);
            gfx_swap_buffers_full();
            return;
        }
        
        /* Calculate pulsating radius */
        radius = base_radius + (frame % 100);
        if ((frame / 100) % 2 == 1) {
            radius = base_radius + 100 - (frame % 100);
        }
        
        /* Only redraw if radius changed significantly */
        if (radius != prev_radius) {
            /* Cycle hue for rainbow effect */
            hue = (hue + 2) % 360;
            color = gfx_hsv(hue, 255, 255);
            
            /* Clear only the area that was drawn (dirty region) */
            /* Draw black circle over previous position to erase */
            if (prev_radius > 0) {
                gfx_draw_circle(cx, cy, prev_radius + 2, 0x00000000);
            }
            
            /* Draw the new circle */
            gfx_draw_circle(cx, cy, radius, color);
            
            /* Swap only the dirty region */
            gfx_swap_buffers();
            
            prev_radius = radius;
        }
        
        /* Small delay */
        wait(16);
        
        frame++;
    }
}
