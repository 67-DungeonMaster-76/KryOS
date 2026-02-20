/*
 * idt.h - Interrupt Descriptor Table header
 * version 0.0.2
 */

#ifndef IDT_H
#define IDT_H

#include "stdint.h"

/* IDT entry structure */
struct idt_entry {
    unsigned short base_low;
    unsigned short sel;         /* Kernel segment selector */
    unsigned char always0;      /* Always 0 */
    unsigned char flags;        /* Flags */
    unsigned short base_high;
} __attribute__((packed));

/* IDT pointer structure */
struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

/* Number of IDT entries */
#define IDT_ENTRIES 256

/* Initialize IDT */
void idt_install(void);

/* Set an IDT gate */
void idt_set_gate(unsigned char num, unsigned long base,
                  unsigned short sel, unsigned char flags);

/* Interrupt handlers */
void isr_handler(int int_num, uint32_t eip, uint32_t eax, uint32_t ebx, uint32_t esp);
void irq_handler(int int_num);

#endif /* IDT_H */
