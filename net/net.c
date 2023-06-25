#include "net.h"
#include "e1000.h"
#include <pci.h>
#include <kernel.h>

uint32_t CSR_IO_BAR;
uint32_t* CSR_MEM_BAR;
int BAR_0;
struct ringElement* ring;

void writeOut(uint32_t reg, uint32_t data) {
    if (BAR_0) {
        *(CSR_MEM_BAR+reg)=data;
    } else {
        outportl(CSR_IO_BAR, reg);
        outportl(CSR_IO_BAR+0x04, data);
    }
}

uint32_t readIn(int32_t reg) {
    if (BAR_0) {
        return *(CSR_MEM_BAR+reg);
    }
    else {
        outportl(CSR_IO_BAR, reg);
        return inportl(CSR_IO_BAR+0x04);
    }
}

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func) {
    CSR_IO_BAR = (pciConfigReadRegister(bus, slot, func, PCI_BAR_1, PCI_SELECT_REGISTER) & ~1);
    CSR_MEM_BAR = (uint32_t *)(pciConfigReadRegister(bus, slot, func, PCI_BAR_0, PCI_SELECT_REGISTER) & ~3);
    BAR_0 = 1;
    if (CSR_MEM_BAR == 0) {
        BAR_0 = 0;
    }
    writeOut(E1000_TCTL, 0b0110000000000111111000011111010);
    ring 
        = calloc(RING_ELEMENT_NO,
        sizeof(struct ringElement));
    writeOut(E1000_TDBAH, (uint32_t)((uint64_t)ring>>32));
    writeOut(E1000_TDBAL, (uint32_t)((uint64_t)ring & 0xFFFFFFFF));
    writeOut(E1000_TDLEN, RING_ELEMENT_NO*sizeof(struct ringElement));
    writeOut(E1000_TDT, 0);
    writeOut(E1000_TCTL,  0b0110000000000111111000011111010);
    writeOut(E1000_TIPG,  0x0060200A);
    print("Started Contoller", 17);
}

void nicTransmit(void* data, size_t packetLen) {
    for (int i = 0; i<RING_ELEMENT_NO; i++) {
        ring[i].cmd=0b00011001;
        ring[i].addr=(uint64_t)data;
        ring[i].length = packetLen;
        ring[i].cso=0;
        ring[i].css=0;
    }
    writeOut(E1000_TDT, 1);
    while ((ring[0].status & 1)== 0) {
        continue;
    }
    print("Success!", 8);
}