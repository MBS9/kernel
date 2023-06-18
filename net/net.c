#include "net.h"
#include <pci.h>
#include <kernel.h>

uint32_t CSR_IO_MAPPED_BASE_ADDR;

void writeOut(uint32_t reg, uint32_t data) {
    outportl(CSR_IO_MAPPED_BASE_ADDR, reg);
    outportl(CSR_IO_MAPPED_BASE_ADDR+0x04, data);
}

uint32_t readIn(int32_t reg) {
    outportl(CSR_IO_MAPPED_BASE_ADDR, reg);
    return inportl(CSR_IO_MAPPED_BASE_ADDR+0x04);
}

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func) {
    CSR_IO_MAPPED_BASE_ADDR = pciConfigReadRegister(bus, slot, func, INTEL_ETHER_CSR_IO_BASE_REG, PCI_SELECT_REGISTER);
    writeOut(INTEL_ETHER_CSR_PORT, INTEL_ETHER_PORT_RESET);
    sleep(0xFF);
}

void nicTransmit(void* data[], size_t packetLen) {
    struct ringElement* ring 
        = calloc(RING_ELEMENT_NO,
        sizeof(struct ringElement)+packetLen);
    for (int i = 0; i<RING_ELEMENT_NO; i++) {
        ring[i].cmd=INTEL_ETHER_RING_TRANSMIT;
        ring[i].arrayAdrr=0xFFFFFFFF;
        ring[i].threshold=0xE0;
        ring[i].tbdNumber=0;
        ring[i].size = packetLen;
        ring[i].link = &(ring[i+1]);
        memcpy(&ring[i].content, (void*)data[i], packetLen);
    }
    ring[7].link = &ring[0];
    ring[7].cmd = INTEL_ETHER_RING_TRANSMIT | INTEL_ETHER_RING_TRANSMIT_FINAL;
    writeOut(INTEL_ETHER_CSR_GENERAL_POINTER , (uint32_t)&ring[0]);
    writeOut(0, INTEL_ETHER_CSR_TRANSMIT);
    while ((ring[0].status & (1<<13))== 0) {
        continue;
    }
    print("Success!", 8);
}