/*
 * demo.c - Graphics demo implementation
 * version 0.0.6
 */

#include "demo.h"
#include "graphics.h"
#include "utils.h"

/*
 * Run rainbow circle demo
 * Displays a circle in the center that cycles through rainbow colors
 */
void demo_rainbow_circle(void) {
    int cx = GFX_WIDTH / 2;
    int cy = GFX_HEIGHT / 2;
    int radius = 150;
    int frame = 0;
    uint32_t color;
    
    print("Demo: Starting...\n");
    
    print("Demo: Initializing graphics...\n");
    graphics_init();
    print("Demo: Graphics initialized.\n");
    
    print("Demo: Framebuffer at: ");
    print_hex((unsigned int)gfx_get_framebuffer());
    print("\n");
    
    print("Demo: Clearing screen to blue...\n");
    gfx_clear(0x000000FF);  /* Blue */
    gfx_swap_buffers();
    print("Demo: Screen cleared.\n");
    
    wait(1000);
    
    print("Demo: Drawing red circle...\n");
    color = gfx_rgb(255, 0, 0);  /* Red */
    print("Demo: Color = ");
    print_hex(color);
    print("\n");
    
    gfx_draw_circle(cx, cy, radius, color);
    print("Demo: Circle drawn to buffer.\n");
    
    print("Demo: Swapping buffers...\n");
    gfx_swap_buffers();
    print("Demo: Buffers swapped.\n");
    
    print("Demo: Done. Looping...\n");
    
    while (1) {
        frame++;
        if (frame % 60 == 0) {
            print("Demo: Frame ");
            print_int(frame);
            print("\n");
        }
        wait(16);
    }
}
