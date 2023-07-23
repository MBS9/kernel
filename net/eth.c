#include <kernel.h>
#include "net.h"
#include <lib/lib.h>

extern uint8_t mac[6];

struct etherPacket* createEthernetFrame(uint8_t* dest, uint16_t length, uint16_t type, void* buffer) {
    void* frame = calloc(1, sizeof(struct etherPacket)+length);
    struct etherPacket* packet = frame;
    packet->sfd = ETHER_SFD;
    for (int i = 0; i < ETHER_PREAMBLE_LEN; i++)
        packet->preamble[i] = ETHER_PREAMBLE_CONTENT;
    memcpy(packet->dest, dest, sizeof(packet->dest));
    memcpy(packet->data, buffer, length);
    memcpy(packet->source, &mac, sizeof(mac));
    packet->length_type = type;
    return packet;
}
