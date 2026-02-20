/*
 * idt.c - Interrupt Descriptor Table implementation
 * version 0.0.3
 * Updated to use framebuffer console for error messages
 */

#include "idt.h"
#include "drivers/video/fb_console.h"
#include "drivers/video/graphics.h"
#include "drivers/input/keyboard.h"
#include "string.h"
#include "utils.h"

/* IDT with 256 entries */
struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

/* External assembly function to load IDT */
extern void idt_load(void);

/* External assembly ISR stubs */
extern void isr0(void);   /* Divide by zero */
extern void isr1(void);   /* Debug */
extern void isr2(void);   /* NMI */
extern void isr3(void);   /* Breakpoint */
extern void isr4(void);   /* Overflow */
extern void isr5(void);   /* Bounds */
extern void isr6(void);   /* Invalid Opcode */
extern void isr7(void);   /* Coprocessor not available */
extern void isr8(void);   /* Double fault */
extern void isr9(void);   /* Coprocessor Segment Overrun */
extern void isr10(void);  /* Invalid TSS */
extern void isr11(void);  /* Segment not present */
extern void isr12(void);  /* Stack exception */
extern void isr13(void);  /* General Protection fault */
extern void isr14(void);  /* Page fault */
extern void isr15(void);  /* Reserved */
extern void isr16(void);  /* Coprocessor error */
extern void isr17(void);  /* Alignment check */
extern void isr18(void);  /* Machine check */
extern void isr19(void);  /* Reserved */
extern void isr20(void);  /* Reserved */
extern void isr21(void);  /* Reserved */
extern void isr22(void);  /* Reserved */
extern void isr23(void);  /* Reserved */
extern void isr24(void);  /* Reserved */
extern void isr25(void);  /* Reserved */
extern void isr26(void);  /* Reserved */
extern void isr27(void);  /* Reserved */
extern void isr28(void);  /* Reserved */
extern void isr29(void);  /* Reserved */
extern void isr30(void);  /* Reserved */
extern void isr31(void);  /* Reserved */

/* IRQ handlers */
extern void irq0(void);   /* System timer */
extern void irq1(void);   /* Keyboard */
extern void irq2(void);   /* Cascade */
extern void irq3(void);   /* COM2 */
extern void irq4(void);   /* COM1 */
extern void irq5(void);   /* LPT2 */
extern void irq6(void);   /* Floppy */
extern void irq7(void);   /* LPT1 */
extern void irq8(void);   /* CMOS RTC */
extern void irq9(void);   /* Free for peripherals */
extern void irq10(void);  /* Free for peripherals */
extern void irq11(void);  /* Free for peripherals */
extern void irq12(void);  /* PS2 Mouse */
extern void irq13(void);  /* FPU / Coprocessor */
extern void irq14(void);  /* Primary ATA Hard Disk */
extern void irq15(void);  /* Secondary ATA Hard Disk */

/*
 * Set an IDT gate
 */
void idt_set_gate(unsigned char num, unsigned long base,
                  unsigned short sel, unsigned char flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

/*
 * Remap the PIC
 * IRQs 0-7 map to IDT entries 32-39
 * IRQs 8-15 map to IDT entries 40-47
 */
static void pic_remap(void) {
    /* Save masks */
    unsigned char mask1 = inb(0x21);
    unsigned char mask2 = inb(0xA1);
    
    /* Start initialization sequence */
    outb(0x20, 0x11);  /* ICW1: Initialize master PIC */
    outb(0xA0, 0x11);  /* ICW1: Initialize slave PIC */
    
    /* ICW2: Set vector offsets */
    outb(0x21, 0x20);  /* Master PIC vector offset: 32 */
    outb(0xA1, 0x28);  /* Slave PIC vector offset: 40 */
    
    /* ICW3: Tell Master PIC there's a slave PIC at IRQ2 */
    outb(0x21, 0x04);
    /* ICW3: Tell Slave PIC its cascade identity */
    outb(0xA1, 0x02);
    
    /* ICW4: 8086 mode */
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    /* Restore masks - disable all IRQs except cascade */
    outb(0x21, 0xFF);  /* Mask all IRQs on master */
    outb(0xA1, 0xFF);  /* Mask all IRQs on slave */
}

/*
 * Initialize IDT
 */
void idt_install(void) {
    /* Setup the IDT pointer */
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (unsigned int)&idt;
    
    /* Clear IDT */
    memset(&idt, 0, sizeof(struct idt_entry) * IDT_ENTRIES);
    
    /* Remap the PIC */
    pic_remap();
    
    /* Install ISRs (0-31) */
    idt_set_gate(0, (unsigned)isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned)isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned)isr2, 0x08, 0x8E);
    idt_set_gate(3, (unsigned)isr3, 0x08, 0x8E);
    idt_set_gate(4, (unsigned)isr4, 0x08, 0x8E);
    idt_set_gate(5, (unsigned)isr5, 0x08, 0x8E);
    idt_set_gate(6, (unsigned)isr6, 0x08, 0x8E);
    idt_set_gate(7, (unsigned)isr7, 0x08, 0x8E);
    idt_set_gate(8, (unsigned)isr8, 0x08, 0x8E);
    idt_set_gate(9, (unsigned)isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned)isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned)isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned)isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned)isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned)isr15, 0x08, 0x8E);
    idt_set_gate(16, (unsigned)isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned)isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned)isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned)isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned)isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned)isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned)isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned)isr23, 0x08, 0x8E);
    idt_set_gate(24, (unsigned)isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned)isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned)isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned)isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned)isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned)isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned)isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned)isr31, 0x08, 0x8E);
    
    /* Install IRQs (32-47) */
    idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);
    idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);
    
    /* Load the IDT */
    idt_load();
}

/*
 * ISR handler - called from assembly
 * This handles CPU exceptions with BSOD-style display
 * Parameters: int_num, eip, eax, ebx, esp
 */
void isr_handler(int int_num, uint32_t eip, uint32_t eax, uint32_t ebx, uint32_t esp) {
    /* Exception names */
    static const char *exception_names[] = {
        "Division By Zero",
        "Debug",
        "Non Maskable Interrupt",
        "Breakpoint",
        "Into Detected Overflow",
        "Out of Bounds",
        "Invalid Opcode",
        "No Coprocessor",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Bad TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection Fault",
        "Page Fault",
        "Unknown Interrupt",
        "Coprocessor Fault",
        "Alignment Check",
        "Machine Check",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved"
    };
    
    /* Clear screen to blue */
    gfx_clear(0x00FF0000);  /* Blue background (RGB: 0,0,255 -> 0x00FF0000 in XRGB) */
    
    /* Set white text on blue background */
    fb_set_text_color(0x00FFFFFF, 0x00FF0000);
    
    /* Reset cursor to top-left */
    fb_console_reset_cursor();
    
    /* BSOD Header */
    fb_print("eh oh.\n\n");
    fb_print("System encountered an exception which can't be resolved.\n\n");
    fb_print("More info: ");
    if (int_num >= 0 && int_num < 32) {
        fb_print(exception_names[int_num]);
    } else {
        fb_print("Unknown Exception");
    }
    fb_print("\n\n");
    
    /* Register dump */
    fb_print("Register dump:\n");
    fb_print("  EAX: ");
    fb_print_hex(eax);
    fb_print("\n  EBX: ");
    fb_print_hex(ebx);
    fb_print("\n  ESP: ");
    fb_print_hex(esp);
    fb_print("\n  EIP: ");
    fb_print_hex(eip);
    fb_print("\n  INT: ");
    fb_print_int(int_num);
    fb_print("\n\n");
    
    fb_print("System halted. Please restart your computer.\n");
    
    fb_flush();
    
    /* Halt the system on exception */
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}

/*
 * IRQ handler - called from assembly
 * This handles hardware interrupts
 * Parameters: int_num - the interrupt number (32-47)
 */
void irq_handler(int int_num) {
    /* Call keyboard handler for IRQ1 (interrupt 33) */
    if (int_num == 33) {
        keyboard_handler();
    }
    
    /* Send EOI to PIC */
    outb(0x20, 0x20);
    
    /* If IRQ came from slave PIC (IRQ 8-15), send EOI to slave too */
    if (int_num >= 40) {
        outb(0xA0, 0x20);
    }
}
