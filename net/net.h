#include <inttypes.h>
#include <stddef.h>
#pragma once

volatile struct etherPacket
{
    uint8_t dest[6];
    uint8_t source[6];
    uint16_t length_type;
    char data[];
};

#define HARDWARE_ADRR_LEN 6
#define IP_ADRR_LEN 4
#define ETHERNET 0x1

#define PROTOCOL_IP 0x800
#define PROTOCOL_TEST 0x88b5
#define PROTOCOL_ARP 0x0806

#define ARP_REQUEST 1

// Thank you https://wiki.osdev.org/Address_Resolution_Protocol
struct arp
{
    uint16_t htype; // Hardware type
    uint16_t ptype; // Protocol type
    uint8_t  hlen; // Hardware address length (Ethernet = 6)
    uint8_t  plen; // Protocol address length (IPv4 = 4)
    uint16_t opcode; // ARP Operation Code
    uint8_t  srchw[HARDWARE_ADRR_LEN]; // Source hardware address - hlen bytes (see above)
    uint8_t  srcpr[IP_ADRR_LEN]; // Source protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
    uint8_t  dsthw[HARDWARE_ADRR_LEN]; // Destination hardware address - hlen bytes (see above)
    uint8_t  dstpr[IP_ADRR_LEN]; // Destination protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
};

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func);
void nicTransmit(void* data, size_t packetLen);

struct arp* createArpPacket(uint8_t* srcpr, uint8_t* dstpr);

struct etherPacket* createEthernetFrame(uint8_t* dest, uint16_t length, uint16_t type, void* buffer);

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

#define RING_ELEMENT_NO 8

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
#define INTEL_ETHER_TCTL_RTLC 1<<24

#define INTEL_ETHER_CTRL_FD 1
#define INTEL_ETHER_CTRL_ASDE 1 << 5
#define INTEL_ETHER_CTRL_SLU 1 << 6
#define INTEL_ETHER_CTRL_LRST 1 << 3
#define INTEL_ETHER_CTRL_RESET 1<<26

#define TX_CTRL_IDE 1 << 7
#define TX_CTRL_VLE 1 << 6
#define TX_CTRL_RPS 1 << 4
#define TX_CTRL_RS 1 << 3
#define TX_CTRL_IC 1 << 2
#define TX_CTRL_IFCS 1 << 1
#define TX_CTRL_EOP 1

#define EEPROM_DONE 1<<4
#define EEPROM_ADRR_SHIFT 8
#define EEPROM_EXIST 1<<8