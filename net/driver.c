#include "net.h"
#include "e1000.h"
#include <pci.h>
#include <kernel.h>

volatile uint32_t CSR_IO_BAR;
volatile uint32_t CSR_MEM_BAR;
volatile int BAR_0;
int tx_offset;
static int rx_head;
static int rx_cur;
volatile struct tx_desc tx_ring[RING_ELEMENT_NO];
volatile struct rx_desc rx_ring[RING_ELEMENT_NO];
uint8_t mac[6];

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

uint32_t eepromRead(uint32_t reg)
{
    writeOut(E1000_EERD, 1 | (reg << EEPROM_ADRR_SHIFT));
    uint32_t temp;
    while (!((temp = readIn(E1000_EERD)) & EEPROM_DONE));
    uint16_t data = (temp >> 16) & 0xFFFF;
    return data;
}

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func)
{
    uint64_t ringPhysicalAdrr;
    CSR_IO_BAR = (pciConfigReadRegister(bus, slot, func, PCI_BAR_1, PCI_SELECT_REGISTER) & ~1);
    CSR_MEM_BAR = (pciConfigReadRegister(bus, slot, func, PCI_BAR_0, PCI_SELECT_REGISTER) & ~3);
    BAR_0 = 1;
    if (CSR_MEM_BAR == 0)
    {
        BAR_0 = 0;
    }
    // tx_ring = calloc(RING_ELEMENT_NO, sizeof(struct tx_desc));
    pciConfigSetRegister(bus, slot, func, 0x4, PCI_CMD_IO | PCI_CMD_MEM | PCI_CMD_FBBE | PCI_CMD_BM | PCI_CMD_SC | PCI_CMD_MWIE);
    if (!(readIn(E1000_EECD) & EEPROM_EXIST))
        return; // EEPROM does not exist...
    uint32_t temp;
    temp = eepromRead(0);
    mac[0] = temp & 0xff;
    mac[1] = temp >> 8;
    temp = eepromRead(1);
    mac[2] = temp & 0xff;
    mac[3] = temp >> 8;
    temp = eepromRead(2);
    mac[4] = temp & 0xff;
    mac[5] = temp >> 8;
    ringPhysicalAdrr = getPhysicalMemKernel((void *)&tx_ring);
    writeOut(E1000_CTRL, INTEL_ETHER_CTRL_RESET);
    sleep(0xFFFF);
    while ((readIn(E1000_CTRL) & INTEL_ETHER_CTRL_RESET) != 0)
        sleep(0xFFFF);
    print("Reset success!", 14);
    // Transmit  Setup
    writeOut(E1000_CTRL, INTEL_ETHER_CTRL_ASDE | INTEL_ETHER_CTRL_FD | INTEL_ETHER_CTRL_SLU | INTEL_ETHER_CTRL_LRST);
    writeOut(E1000_TDBAH, (uint32_t)(ringPhysicalAdrr >> 32));
    writeOut(E1000_TDBAL, (uint32_t)(ringPhysicalAdrr & 0xFFFFFFFF));
    writeOut(E1000_TDLEN, RING_ELEMENT_NO * sizeof(struct tx_desc));
    writeOut(E1000_TDT, 0);
    writeOut(E1000_TDH, 0);
    writeOut(E1000_TCTL, INTEL_ETHER_TCTL_EN | INTEL_ETHER_TCTL_PSP | INTEL_ETHER_TCTL_RTLC | 0x0F << INTEL_ETHER_TCTL_CT_OFF | 0x40 << INTEL_ETHER_TCTL_COLD_OFF);
    writeOut(E1000_TIPG, 0x0060200A);
    tx_offset = 0;

    // Recieve Setup
    for (size_t i = 0; i < RING_ELEMENT_NO; i++)
        rx_ring[i].buffer_addr = (uint64_t)calloc(1, RECIEVE_BUFFER_SIZE);
    writeOut(E1000_RA, (uint32_t)((uint64_t)mac & 0xFFFFFFFF));
    writeOut(E1000_RA+0x4, (uint32_t)((uint64_t)mac >> 32) & 0xFFFF);
    writeOut(E1000_MTA, 0);
    writeOut(E1000_IMS, 0);
    ringPhysicalAdrr = getPhysicalMemKernel((void *)&rx_ring);
    writeOut(E1000_RDBAL, (uint32_t)(ringPhysicalAdrr & 0xFFFFFFFF));
    writeOut(E1000_RDBAH, (uint32_t)(ringPhysicalAdrr >> 32));
    writeOut(E1000_RDLEN, RECIEVE_BUFFER_SIZE);
    writeOut(E1000_RDH, 0);
    rx_head = 0;
    writeOut(E1000_RDT, RING_ELEMENT_NO-1);
    rx_cur = 0;
    writeOut(E1000_RCTL, RCTL_EN | RCTL_LPE | RCTL_BAM | RCTL_SECRC | RCTL_BSIZE_1048);
    print("Started Contoller", 17);
}

void nicTransmit(void* data, size_t packetLen)
{
    tx_ring[tx_offset].lower.flags.cmd = TX_CTRL_EOP | TX_CTRL_RS | TX_CTRL_RPS | TX_CTRL_IFCS;
    tx_ring[tx_offset].buffer_addr = getPhysicalMemHeap(data);
    tx_ring[tx_offset].lower.flags.length = packetLen;
    tx_ring[tx_offset].lower.flags.cso = 0;
    tx_ring[tx_offset].upper.fields.css = 0;
    register int tx_offset_number = tx_offset + 1;
    writeOut(E1000_TDT, tx_offset_number);
    while ((tx_ring[tx_offset].upper.fields.status & 1) == 0)
        continue;
    tx_offset = tx_offset_number % RING_ELEMENT_NO;
    print("Success!", 8);
}

void* nicReadFrame()
{
    /*
    Returns null pointer if no new packet has arrived.
    */
    uint8_t old_cur;
    void* buffer = 0x00;
    if ((rx_ring[rx_cur].status & 0x1))
    {
        uint8_t *buf = (uint8_t *)getVirtualMemHeap(rx_ring[rx_cur].buffer_addr);
        uint16_t len = rx_ring[rx_cur].length;
        buffer = calloc(1, len);
        memcpy(buffer, buf, len);
        rx_ring[rx_cur].status = 0;
        old_cur = rx_cur;
        rx_cur = (rx_cur + 1) % RING_ELEMENT_NO;
        writeOut(E1000_RDT, old_cur);
    }
    return buffer;
}
