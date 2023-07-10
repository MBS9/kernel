#pragma once
#include <inttypes.h>
#include <stddef.h>

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func);
void nicTransmit(void* data, size_t packetLen);

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
volatile struct ringElement {
    uint64_t buffer_addr;       /* Address of the descriptor's data buffer */
    union {
        uint32_t data;
        struct {
            uint16_t length;    /* Data buffer length */
            uint8_t cso;        /* Checksum offset */
            uint8_t cmd;        /* Descriptor control */
        } flags;
    } lower;
    union {
        uint32_t data;
        struct {
            uint8_t status;     /* Descriptor status */
            uint8_t css;        /* Checksum start */
            uint16_t special;
        } fields;
    } upper;
};

struct arp
{
    volatile uint16_t htype; // Hardware type
    volatile uint16_t ptype; // Protocol type
    volatile uint8_t  hlen; // Hardware address length (Ethernet = 6)
    volatile uint8_t  plen; // Protocol address length (IPv4 = 4)
    volatile uint16_t opcode; // ARP Operation Code
    volatile uint8_t  srchw[6]; // Source hardware address - hlen bytes (see above)
    volatile uint8_t  srcpr[4]; // Source protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
    volatile uint8_t  dsthw[6]; // Destination hardware address - hlen bytes (see above)
    volatile uint8_t  dstpr[4]; // Destination protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
};

#define INTEL_ETHER_CSR_IO_BASE_REG 0x18
#define INTEL_ETHER_CSR_MEM_REG 0x10
#define INTEL_ETHER_PORT_RESET 0
#define INTEL_ETHER_CSR_TRANSMIT 0b0000000100010000<<16
#define INTEL_ETHER_RING_TRANSMIT 0b100
#define INTEL_ETHER_RING_TRANSMIT_FINAL 0b1000000000000000

#define INTEL_ETHER_CSR_PORT 8
#define INTEL_ETHER_CSR_GENERAL_POINTER 4

#define INTEL_ETHER_CBL_OK 0b0010000000000000

#define INTEL_ETHER_TCTL_EN 0b1<<1
#define INTEL_ETHER_TCTL_PSP 0b1<<3
#define INTEL_ETHER_TCTL_CT_OFF 4
#define INTEL_ETHER_TCTL_COLD_OFF 12

#define INTEL_ETHER_CTRL_FD 1
#define INTEL_ETHER_CTRL_ASDE 1<<5
#define INTEL_ETHER_CTRL_SLU 1<<6
#define INTEL_ETHER_CTRL_LRST 1<<3

