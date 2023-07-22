#include "net.h"
#include "e1000.h"
#include <pci.h>
#include <kernel.h>

volatile uint32_t CSR_IO_BAR;
volatile uint32_t CSR_MEM_BAR;
volatile int BAR_0;
volatile struct ringElement tx_ring[RING_ELEMENT_NO];
uint64_t ringPhysicalAdrr;

extern uint64_t kernelBaseVMem;
extern uint64_t kernelBasePMem;
extern uint64_t hhdm_offset;

void writeOut(uint32_t reg, uint32_t data)
{
    if (BAR_0)
    {
        *((volatile uint32_t *)(CSR_MEM_BAR + reg)) = data;
    }
    else
    {
        outportl(CSR_IO_BAR, reg);
        outportl(CSR_IO_BAR + 0x04, data);
    }
}

uint32_t readIn(int32_t reg)
{
    if (BAR_0)
    {
        return *((volatile uint32_t *)(CSR_MEM_BAR + reg));
    }
    else
    {
        outportl(CSR_IO_BAR, reg);
        return inportl(CSR_IO_BAR + 0x04);
    }
}

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func)
{
    CSR_IO_BAR = (pciConfigReadRegister(bus, slot, func, PCI_BAR_1, PCI_SELECT_REGISTER) & ~1);
    CSR_MEM_BAR = (pciConfigReadRegister(bus, slot, func, PCI_BAR_0, PCI_SELECT_REGISTER) & ~3);
    BAR_0 = 1;
    if (CSR_MEM_BAR == 0)
    {
        BAR_0 = 0;
    }
    //tx_ring = calloc(RING_ELEMENT_NO, sizeof(struct ringElement));
    pciConfigSetRegister(bus, slot, func, 0x4, PCI_CMD_IO | PCI_CMD_MEM | PCI_CMD_FBBE | PCI_CMD_BM | PCI_CMD_SC | PCI_CMD_MWIE);
    ringPhysicalAdrr = getPhysicalMemKernel((void*)&tx_ring);
    writeOut(E1000_CTRL, 1 << 26);
    sleep(0xFFFF);
    while ((readIn(E1000_CTRL) & (1<<26)) != 0) sleep(0xFFFF);
    print("Reset success!", 14);
    writeOut(E1000_CTRL, INTEL_ETHER_CTRL_ASDE | INTEL_ETHER_CTRL_FD | INTEL_ETHER_CTRL_SLU | INTEL_ETHER_CTRL_LRST);
    writeOut(E1000_TDBAH, (uint32_t)(ringPhysicalAdrr >> 32));
    writeOut(E1000_TDBAL, (uint32_t)(ringPhysicalAdrr & 0xFFFFFFFF));
    writeOut(E1000_TDLEN, RING_ELEMENT_NO * sizeof(struct ringElement));
    writeOut(E1000_TDT, 0);
    writeOut(E1000_TDH, 0);
    writeOut(E1000_TCTL, INTEL_ETHER_TCTL_EN | INTEL_ETHER_TCTL_PSP | 0x0F << INTEL_ETHER_TCTL_CT_OFF | 0x40 << INTEL_ETHER_TCTL_COLD_OFF);
    writeOut(E1000_TIPG, 0x0060200A);
    print("Started Contoller", 17);
}

void nicTransmit(void *data, size_t packetLen)
{
    for (int i = 0; i < RING_ELEMENT_NO; i++)
    {
        tx_ring[i].lower.flags.cmd = TX_CTRL_EOP | TX_CTRL_RS | TX_CTRL_RPS;
        tx_ring[i].buffer_addr = getPhysicalMemHeap(data);
        tx_ring[i].lower.flags.length = packetLen;
        tx_ring[i].lower.flags.cso = 0;
        tx_ring[i].upper.fields.css = 0;
    }
    writeOut(E1000_TDT, 1);
    while ((tx_ring[0].upper.fields.status & 1) == 0)
        continue;
    print("Success!", 8);
}