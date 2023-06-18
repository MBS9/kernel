#pragma once
#include <inttypes.h>
#include <stddef.h>

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func);
void nicTransmit(void* data[], size_t packetLen);

#define RING_ELEMENT_NO 8

struct ringElement {
    volatile uint16_t status;
    volatile uint16_t cmd;
    volatile struct ringElement* link;
    volatile uint32_t arrayAdrr;
    volatile uint16_t size;
    volatile uint8_t threshold;
    volatile uint8_t tbdNumber;
    volatile void* content;
};

struct arp
{
    uint16_t htype; // Hardware type
    uint16_t ptype; // Protocol type
    uint8_t  hlen; // Hardware address length (Ethernet = 6)
    uint8_t  plen; // Protocol address length (IPv4 = 4)
    uint16_t opcode; // ARP Operation Code
    uint8_t  srchw[6]; // Source hardware address - hlen bytes (see above)
    uint8_t  srcpr[4]; // Source protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
    uint8_t  dsthw[6]; // Destination hardware address - hlen bytes (see above)
    uint8_t  dstpr[4]; // Destination protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
};

#define INTEL_ETHER_CSR_IO_BASE_REG 20
#define INTEL_ETHER_PORT_RESET 0
#define INTEL_ETHER_CSR_TRANSMIT 0b0000000100010000<<16
#define INTEL_ETHER_RING_TRANSMIT 0b100
#define INTEL_ETHER_RING_TRANSMIT_FINAL 0b1000000000000000

#define INTEL_ETHER_CSR_PORT 8
#define INTEL_ETHER_CSR_GENERAL_POINTER 4

#define INTEL_ETHER_CBL_OK 0b0010000000000000