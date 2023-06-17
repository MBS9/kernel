#pragma once
#include <inttypes.h>

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func);
void nicTransmit(char** data);

#define RING_ELEMENT_CONTENT_SIZE 30
#define RING_ELEMENT_NO 8

struct ringElement {
    volatile uint16_t status;
    uint16_t cmd;
    struct ringElement* link;
    uint32_t arrayAdrr;
    uint16_t size;
    uint8_t threshold;
    uint8_t tbdNumber;
    char content[RING_ELEMENT_CONTENT_SIZE];
};


#define INTEL_ETHER_CSR_IO_BASE_REG 20
#define INTEL_ETHER_PORT_RESET 0
#define INTEL_ETHER_CSR_TRANSMIT 0b0000000100010000<<16
#define INTEL_ETHER_RING_TRANSMIT 0b0000000000010100
#define INTEL_ETHER_RING_TRANSMIT_FINAL 0b1000000000000000

#define INTEL_ETHER_CSR_PORT 8
#define INTEL_ETHER_CSR_GENERAL_POINTER 4

#define INTEL_ETHER_CBL_OK 0b0010000000000000