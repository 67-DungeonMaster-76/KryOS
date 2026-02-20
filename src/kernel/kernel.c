/*
 * kernel.c - Main kernel entry point
 * version 0.0.10
 */

#include "utils.h"
#include "gdt.h"
#include "idt.h"
#include "drivers/input/keyboard.h"
#include "drivers/video/graphics.h"
#include "drivers/video/fb_console.h"
#include "cli.h"
#include "drivers/fs/ramdisk.h"
#include "drivers/fs/fat32.h"
#include "stdint.h"

/* Multiboot info structure */
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
} __attribute__((packed)) multiboot_info_t;

/* Global multiboot info pointer */
static multiboot_info_t *mb_info = (multiboot_info_t *)0;

/* Initialize FPU */
static void fpu_init(void) {
    __asm__ __volatile__(
        "fninit\n\t"           /* Initialize FPU */
        "mov %%cr0, %%eax\n\t"
        "and $0xFFFB, %%ax\n\t" /* Clear EM bit */
        "or $0x2, %%ax\n\t"     /* Set MP bit */
        "mov %%eax, %%cr0\n\t"
        : : : "eax"
    );
}

/* Initialize SSE */
static void sse_init(void) {
    __asm__ __volatile__(
        "mov %%cr0, %%eax\n\t"
        "and $0xFFFB, %%ax\n\t"  /* Clear EM bit (bit 2) */
        "or $0x2, %%ax\n\t"      /* Set MP bit (bit 1) */
        "mov %%eax, %%cr0\n\t"
        "mov %%cr4, %%eax\n\t"
        "or $0x600, %%ax\n\t"    /* Set OSFXSR (bit 9) and OSXMMEXCPT (bit 10) */
        "mov %%eax, %%cr4\n\t"
        : : : "eax"
    );
}

/* Get framebuffer info from multiboot */
uint32_t *gfx_get_framebuffer_from_multiboot(void);
int gfx_get_width_from_multiboot(void);
int gfx_get_height_from_multiboot(void);

uint32_t *gfx_get_framebuffer_from_multiboot(void) {
    if (mb_info && (mb_info->flags & (1 << 12))) {
        /* Framebuffer address is 64-bit, but on 32-bit system we use low 32 bits */
        uint64_t addr = mb_info->framebuffer_addr;
        return (uint32_t *)(uint32_t)addr;
    }
    /* Fallback for QEMU - typical LFB address */
    return (uint32_t *)0xE0000000;
}

int gfx_get_width_from_multiboot(void) {
    if (mb_info && (mb_info->flags & (1 << 12))) {
        return mb_info->framebuffer_width;
    }
    return 800;
}

int gfx_get_height_from_multiboot(void) {
    if (mb_info && (mb_info->flags & (1 << 12))) {
        return mb_info->framebuffer_height;
    }
    return 600;
}

/* Kernel entry point - called from boot.asm */
void k_main(uint32_t magic, uint32_t mbi) {
    /* Save multiboot info */
    mb_info = (multiboot_info_t *)mbi;
    
    /* Initialize SSE for faster graphics */
    sse_init();
    
    graphics_init();
    fb_console_init();
    

    fb_print("Initializing GDT... ");
    gdt_install();
    fb_print("Done!\n");
    
    /* Initialize IDT */
    fb_print("Initializing IDT... ");
    idt_install();
    fb_print("Done!\n");
    /* Initialize graphics first (needed for framebuffer console) */
   
    
    /* Initialize framebuffer console */
    
    /* Print welcome message */
    fb_print("Welcome to KryOS!\n");
    fb_print("=================\n\n");
    
    /* Print multiboot info */
    fb_print("Multiboot magic: ");
    fb_print_hex(magic);
    fb_putchar('\n');
    fb_print("Multiboot info at: ");
    fb_print_hex(mbi);
    fb_putchar('\n');
    
    if (mb_info && (mb_info->flags & (1 << 12))) {
        fb_print("Framebuffer found!\n");
        fb_print("  Address: ");
        fb_print_hex((uint32_t)(uint32_t)mb_info->framebuffer_addr);
        fb_putchar('\n');
        fb_print("  Size: ");
        fb_print_int(mb_info->framebuffer_width);
        fb_putchar('x');
        fb_print_int(mb_info->framebuffer_height);
        fb_putchar('x');
        fb_print_int(mb_info->framebuffer_bpp);
        fb_putchar('\n');
    } else {
        fb_print("No framebuffer info from GRUB.\n");
    }
    
    /* Initialize FPU */
    fb_print("Initializing FPU... ");
    fpu_init();
    fb_print("Done!\n");
    
    /* Initialize keyboard */
    fb_print("Initializing keyboard... ");
    keyboard_init();
    fb_print("Done!\n");
    
    /* Initialize RAM disk */
    fb_print("Initializing RAM disk... ");
    ramdisk_init();
    fb_print("Done!\n");
    
    /* Initialize FAT32 filesystem */
    fb_print("Initializing FAT32 filesystem... ");
    fat32_init();
    fb_print("Done!\n");
    
    /* Enable interrupts */
    fb_print("Enabling interrupts... ");
    __asm__ __volatile__("sti");
    fb_print("Done!\n\n");
    
    /* Start CLI */
    cli_init();
    cli_run();
}
