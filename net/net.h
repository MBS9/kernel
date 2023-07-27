#pragma once
#include <inttypes.h>
#include <stddef.h>

#define HARDWARE_ADRR_LEN 6
#define IP_ADRR_LEN 4
#define ETHERNET 0x1

#define PROTOCOL_IP 0x800
#define PROTOCOL_TEST 0x88b5
#define PROTOCOL_ARP 0x0806

#define IP_PROTOCOL_UDP 0x11
#define IP_PROTOCOL_UNASSIGNED 63
#define IP_PROTOCOL_ICMP 1

#define ARP_REQUEST 1

#define MIN_UDP_HEADER 8
#define IP_VERSION 4
#define IP_VERSION_OFFSET 4
#define MIN_IP_HEADER_LEN 5
#define IP_NET_CONTROL 0b111 << 5
#define IP_HIGH_RELIABILITY 1 << 3

volatile struct etherFrame
{
    uint8_t dest[6];
    uint8_t source[6];
    uint16_t length_type;
};

// Thank you https://wiki.osdev.org/Address_Resolution_Protocol
volatile struct arp
{
    uint16_t htype;                   // Hardware type
    uint16_t ptype;                   // Protocol type
    uint8_t hlen;                     // Hardware address length (Ethernet = 6)
    uint8_t plen;                     // Protocol address length (IPv4 = 4)
    uint16_t opcode;                  // ARP Operation Code
    uint8_t srchw[HARDWARE_ADRR_LEN]; // Source hardware address - hlen bytes (see above)
    uint8_t srcpr[IP_ADRR_LEN];       // Source protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
    uint8_t dsthw[HARDWARE_ADRR_LEN]; // Destination hardware address - hlen bytes (see above)
    uint8_t dstpr[IP_ADRR_LEN];       // Destination protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
};

volatile struct ip
{
    uint8_t version_ihl;
    uint8_t type;
    uint16_t len;
    uint16_t id;
    uint16_t flag_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint32_t source;
    uint32_t dest;
};

typedef volatile struct
{
    uint16_t sourcePort;
    uint16_t destPort;
    uint16_t length;
    uint16_t checksum;
    char data[];
} udp;

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func);
void nicTransmit(void *data, size_t packetLen, uint8_t options, uint8_t CSO, uint8_t CSS);
void *nicReadFrame();

int createUdpPacet(uint16_t sourcePort, uint16_t destPort, uint8_t *sourceIp, uint8_t *destIp, uint8_t *destMac, char *data, uint16_t dataLen, void **frameAddr);
int setupEthernetFrame(uint8_t *dest, uint16_t type, struct etherFrame *packet);
int createArpPacket(uint8_t *srcpr, uint8_t *dstpr, void **bufferPtr);
int setupIpPacket(uint8_t *sourceIp, uint8_t *destIp, uint8_t *destMac, uint16_t len, uint8_t protocol, void *frameAddr);

#define RECIEVE_BUFFER_SIZE 1048

// Taken from QEMU Source
volatile struct rx_desc
{
    uint64_t buffer_addr; /* Address of the descriptor's data buffer */
    uint16_t length;      /* Length of data DMAed into data buffer */
    uint16_t csum;        /* Packet checksum */
    uint8_t status;       /* Descriptor status */
    uint8_t errors;       /* Descriptor Errors */
    uint16_t special;
};

// Taken from QEMU source
volatile struct tx_desc
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
#define INTEL_ETHER_TCTL_RTLC 1 << 24

#define INTEL_ETHER_CTRL_FD 1
#define INTEL_ETHER_CTRL_ASDE 1 << 5
#define INTEL_ETHER_CTRL_SLU 1 << 6
#define INTEL_ETHER_CTRL_LRST 1 << 3
#define INTEL_ETHER_CTRL_RESET 1 << 26

#define TX_CTRL_IDE 1 << 7
#define TX_CTRL_VLE 1 << 6
#define TX_CTRL_RPS 1 << 4
#define TX_CTRL_RS 1 << 3
#define TX_CTRL_IC 1 << 2
#define TX_CTRL_IFCS 1 << 1
#define TX_CTRL_EOP 1

#define EEPROM_DONE 1 << 4
#define EEPROM_ADRR_SHIFT 8
#define EEPROM_EXIST 1 << 8

#define RCTL_EN 1 << 1
#define RCTL_LPE 1 << 5
#define RCTL_BAM 1 << 15
#define RCTL_SECRC 1 << 26

#define RCTL_BSIZE_2048 0b00 << 16
#define RCTL_BSIZE_1048 0b01 << 16
#define RCTL_BSIZE_512 0b10 << 16
#define RCTL_BSIZE_256 0b11 << 16
