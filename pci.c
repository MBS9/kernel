#include "kernel.h"

extern int cursorX;
extern int cursorY;

uint8_t inb(uint16_t port)
{
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "d"(port));
  return ret;
}

void outb(uint16_t port, uint8_t data)
{
  asm volatile("outb %0, %1" : "=a"(data) : "d"(port));
}


uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;
 
    // Create configuration address as per Figure 1
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    // Write out the address
    outb(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t)((inb(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

uint16_t pciCheckDevice(uint8_t bus, uint8_t slot, uint8_t function) {
    uint16_t vendor, classID;
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */
    if ((vendor = pciConfigReadWord(bus, slot, function, 0)) != 0xFFFF) {
       classID = (pciConfigReadWord(bus, slot, function, 0xA) & ~0xFF00 ) >> 8;
       return classID;
    };
    return 0;
}


void checkAllBuses(void) {
    char temp[3];
    uint16_t bus, class;
    uint8_t device, function;

    for (bus = 0; bus < 256; bus++) {
        for (device = 0; device < 32; device++) {
            for (function = 0; function < 8; function++) {
                class = pciCheckDevice(bus, device, function);
                if (class == 0) {
                    continue;
                }
                itoa(class, &temp, 10);
                print(&temp, 2);
                while (get_input_keycode() != KEY_ENTER){
                    continue;
                }
                cursorY=0;
                cursorX=0;
                if (class == 0xD) {
                    print("Found network controller!", 25);
                }
            }
        }
    }
    print("Checked all busses", 18);
}