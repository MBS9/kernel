#include "net.h"
#include <pci.h>
#include <kernel.h>

uint32_t CSR_IO_MAPPED_BASE_ADDR;

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func) {
    CSR_IO_MAPPED_BASE_ADDR = pciConfigReadWord(bus, slot, func, INTEL_ETHER_CSR_IO_BASE_REG, PCI_SELECT_REGISTER);
    outportl(CSR_IO_MAPPED_BASE_ADDR + 0x4, INTEL_ETHER_PORT_RESET);
    sleep(0x3FFFFF);
}