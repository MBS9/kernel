#include "int.h"
#include <kernel.h>

static struct idtEntry idt[256];
static struct idtr idtr;

void exception_handler() {
    print("IDT Run", 7);
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    struct idtEntry* descriptor = &idt[vector];
 
    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs      = GDT_OFFSET_KERNEL_CODE;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}

extern void* isr_stub_table[];
 
void idt_init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(struct idtEntry) * IDT_MAX_DESCRIPTORS - 1;
 
    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
    }
 
    asm volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    asm volatile ("sti"); // set the interrupt flag
}