#pragma once
#include <inttypes.h>

void nicAttach(uint16_t bus, uint16_t slot, uint16_t func);

struct cb {
    volatile uint16_t status;
    uint16_t cmd;
    uint32_t link;
    uint32_t arrayAdrr;
    uint16_t size;
    uint16_t magic;

};