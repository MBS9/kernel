#include "kernel.h"
#include "pci.h"
#include "net/net.h"

extern int cursorX;
extern int cursorY;

// Stolen from: https://github.com/levex/osdev/blob/master/kernel/hal.c#L40
uint8_t inb(uint16_t portid)
{
	uint8_t ret;
	asm volatile("inb %%dx, %%al":"=a"(ret):"d"(portid));
	return ret;
}
uint16_t inportw(uint16_t portid)
{
	uint16_t ret;
	asm volatile("inw %%dx, %%ax":"=a"(ret):"d"(portid));
	return ret;
}
uint32_t inportl(uint16_t portid)
{
	uint32_t ret;
	asm volatile("inl %%dx, %%eax":"=a"(ret):"d"(portid));
	return ret;
}
void outb(uint16_t portid, uint8_t value)
{
	asm volatile("outb %%al, %%dx": :"d" (portid), "a" (value));
}
void outportw(uint16_t portid, uint16_t value)
{
	asm volatile("outw %%ax, %%dx": :"d" (portid), "a" (value));
}
void outportl(uint16_t portid, uint32_t value)
{
	asm volatile("outl %%eax, %%dx": :"d" (portid), "a" (value));
}

uint32_t pciConfigReadRegister(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset, uint32_t words) {
    // offset is in bytes
    // https://wiki.osdev.org/PCI
    uint64_t address;
    uint64_t lbus  = (uint64_t)bus;
    uint64_t lslot = (uint64_t)slot;
    uint64_t lfunc = (uint64_t)func;
    uint32_t tmp = 0;
 
    address = (uint64_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    // Write out the address
    outportl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    uint32_t reg = inportl(0xCFC);
    tmp = (uint32_t)((reg >> ((offset & 2) * 8)) & words);
    return tmp;
}

void pciConfigSetRegister(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset, uint32_t data) {
    // offset is in bytes
    // https://wiki.osdev.org/PCI
    uint64_t address;
    uint64_t lbus  = (uint64_t)bus;
    uint64_t lslot = (uint64_t)slot;
    uint64_t lfunc = (uint64_t)func;
 
    address = (uint64_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    outportl(0xCF8, address);
    outportl(0xCFC, data);
}

uint16_t pciCheckDevice(uint16_t bus, uint16_t slot, uint16_t function) {
    uint32_t vendor, classID;
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */
    if ((vendor = pciConfigReadRegister(bus, slot, function, 0, PCI_SELECT_ONE_WORD)) != 0xFFFF) {
        uint16_t temp = pciConfigReadRegister(bus, slot, function, PCI_OFFSET_CLASS, PCI_SELECT_ONE_WORD);
        classID = (temp & PCI_SELECT_UPPER_BYTE ) >> 8;
        return classID;
    };
    return 0;
}


void checkAllBuses(void) {
    uint32_t bus, device, function, class;

    for (bus = 0; bus < 256; bus++) {
        for (device = 0; device < 32; device++) {
            for (function = 0; function < 8; function++) {
                class = pciCheckDevice(bus, device, function);
                uint32_t subClass = pciConfigReadRegister(bus, device, function, PCI_OFFSET_CLASS, PCI_SELECT_ONE_WORD);
                subClass = subClass & PCI_SELECT_LOWER_BYTE;
                if (class == 0x2 && subClass == 0x0) {
                    print("Found ethernet controller", 25);
                    nicAttach(bus, device, function);
                }
            }
        }
    }
}