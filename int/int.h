#pragma once
#include <stdint.h>

#define GDT_OFFSET_KERNEL_CODE 5
#define IDT_MAX_DESCRIPTORS 256

void idt_init(void);

volatile struct idtEntry {
    uint16_t    isr_low;
	uint16_t    kernel_cs;
	uint8_t	    ist;
	uint8_t     attributes;
	uint16_t    isr_mid;
	uint32_t    isr_high;
	uint32_t    reserved;
};

volatile struct idtr {
	uint16_t	limit;
	uint64_t	base;
};