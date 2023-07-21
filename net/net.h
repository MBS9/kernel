#include <inttypes.h>
#include <stddef.h>
#pragma once

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func);
void nicTransmit(void *data, size_t packetLen);

#define RING_ELEMENT_NO 8

/*
struct ringElement
{
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint8_t cso;
    volatile uint8_t cmd;
    volatile uint8_t status;
    volatile uint8_t css;
    volatile uint16_t special;
};*/

// Taken from QEMU source
volatile struct ringElement
{
    uint64_t buffer_addr; /* Address of the descriptor's data buffer */
    union
    {
        uint32_t data;
        struct
        {
            uint16_t length; /* Data buffer length */
            uint8_t cso;     /* Checksum offset */
            uint8_t cmd;     /* Descriptor control */
        } flags;
    } lower;
    union
    {
        uint32_t data;
        struct
        {
            uint8_t status; /* Descriptor status */
            uint8_t css;    /* Checksum start */
            uint16_t special;
        } fields;
    } upper;
};

#define INTEL_ETHER_CSR_IO_BASE_REG 0x18
#define INTEL_ETHER_CSR_MEM_REG 0x10
#define INTEL_ETHER_PORT_RESET 0
#define INTEL_ETHER_CSR_TRANSMIT 0b0000000100010000 << 16
#define INTEL_ETHER_RING_TRANSMIT 0b100
#define INTEL_ETHER_RING_TRANSMIT_FINAL 0b1000000000000000

#define INTEL_ETHER_CSR_PORT 8
#define INTEL_ETHER_CSR_GENERAL_POINTER 4

#define INTEL_ETHER_CBL_OK 0b0010000000000000

#define INTEL_ETHER_TCTL_EN 0b1 << 1
#define INTEL_ETHER_TCTL_PSP 0b1 << 3
#define INTEL_ETHER_TCTL_CT_OFF 4
#define INTEL_ETHER_TCTL_COLD_OFF 12

#define INTEL_ETHER_CTRL_FD 1
#define INTEL_ETHER_CTRL_ASDE 1 << 5
#define INTEL_ETHER_CTRL_SLU 1 << 6
#define INTEL_ETHER_CTRL_LRST 1 << 3
#define TX_CTRL_IDE 1 << 7
#define TX_CTRL_VLE 1 << 6
#define TX_CTRL_RPS 1 << 4
#define TX_CTRL_RS 1 << 3
#define TX_CTRL_IC 1 << 2
#define TX_CTRL_IFCS 1 << 1
#define TX_CTRL_EOP 1
