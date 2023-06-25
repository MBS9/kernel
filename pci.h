#pragma once

uint32_t pciConfigReadRegister(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset, uint32_t words);
void pciConfigSetRegister(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset, uint32_t data);

uint8_t inb(uint16_t portid);
uint16_t inportw(uint16_t portid);
uint32_t inportl(uint16_t portid);
void outb(uint16_t portid, uint8_t value);
void outportw(uint16_t portid, uint16_t value);
void outportl(uint16_t portid, uint32_t value);

#define PCI_SELECT_UPPER_BYTE ~0x00FF
#define PCI_SELECT_LOWER_BYTE 0x00FF
#define PCI_OFFSET_CLASS 0xA
#define PCI_SELECT_ONE_WORD 0xFFFF
#define PCI_SELECT_REGISTER 0xFFFFFFFF

#define PCI_BAR_0 0x10
#define PCI_BAR_1 0x14
