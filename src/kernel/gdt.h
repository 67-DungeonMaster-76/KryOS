/*
 * gdt.h - Global Descriptor Table header
 * version 0.0.1
 */

#ifndef GDT_H
#define GDT_H

/* GDT entry structure */
struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

/* GDT pointer structure */
struct gdt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

/* Initialize GDT */
void gdt_install(void);

/* Set a GDT gate */
void gdt_set_gate(int num, unsigned long base, unsigned long limit,
                  unsigned char access, unsigned char gran);

#endif /* GDT_H */
