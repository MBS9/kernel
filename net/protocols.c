#include <kernel.h>
#include "net.h"
#include <lib/lib.h>

extern uint8_t mac[6];

struct etherPacket* createEthernetFrame(uint8_t* dest, uint16_t length, uint16_t type, void* buffer) {
    void* frame = calloc(1, sizeof(struct etherPacket)+length);
    struct etherPacket* packet = (struct etherPacket*)frame;
    memcpy(packet->dest, dest, sizeof(packet->dest));
    memcpy(packet->data, buffer, length);
    memcpy(packet->source, &mac, sizeof(mac));
    packet->length_type = __builtin_bswap16(type);
    return packet;
}

struct arp* createArpPacket(uint8_t* srcpr, uint8_t* dstpr)
{
    struct arp* packet = (struct arp*)calloc(1, sizeof(struct arp));
    packet->htype = __builtin_bswap16(ETHERNET);
    packet->ptype = __builtin_bswap16(PROTOCOL_IP);
    packet->hlen = HARDWARE_ADRR_LEN;
    packet->plen = IP_ADRR_LEN;
    packet->opcode = __builtin_bswap16(ARP_REQUEST);
    memset(packet->dsthw, '\0', HARDWARE_ADRR_LEN);
    memcpy(&packet->srchw, &mac, sizeof(mac));
    memcpy(&packet->srcpr, srcpr, IP_ADRR_LEN);
    memcpy(&packet->dstpr, dstpr, IP_ADRR_LEN);
    return packet;
}
