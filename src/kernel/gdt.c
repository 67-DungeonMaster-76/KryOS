/*
 * gdt.c - Global Descriptor Table implementation
 * version 0.0.1
 */

#include "gdt.h"

/* GDT with 3 entries + null entry */
struct gdt_entry gdt[4];
struct gdt_ptr gp;

/* External assembly function to load GDT */
extern void gdt_flush(void);

/*
 * Set a GDT gate
 */
void gdt_set_gate(int num, unsigned long base, unsigned long limit,
                  unsigned char access, unsigned char gran) {
    /* Setup the descriptor base address */
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    
    /* Setup the descriptor limits */
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    
    /* Finally, set up the granularity and access flags */
    gdt[num].access = access;
}

/*
 * Initialize GDT
 */
void gdt_install(void) {
    /* Setup the GDT pointer and limit */
    gp.limit = (sizeof(struct gdt_entry) * 4) - 1;
    gp.base = (unsigned int)&gdt;
    
    /* NULL descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);
    
    /* Code Segment: base=0, limit=4GB, access=0x9A (present, ring 0, code, exec, readable)
       granularity=0xCF (4KB blocks, 32-bit) */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    
    /* Data Segment: base=0, limit=4GB, access=0x92 (present, ring 0, data, writable)
       granularity=0xCF (4KB blocks, 32-bit) */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    
    /* User Mode Code Segment: base=0, limit=4GB, access=0xFA (present, ring 3, code, exec, readable)
       granularity=0xCF (4KB blocks, 32-bit) */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    
    /* Flush out the old GDT and install the new one */
    gdt_flush();
}
