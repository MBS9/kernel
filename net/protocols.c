#include <kernel.h>
#include "net.h"
#include <lib/lib.h>

extern uint8_t mac[6];
uint8_t broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast

int setupEthernetFrame(uint8_t *dest, uint16_t length, uint16_t type, struct etherFrame *packet)
{
    memcpy(packet->dest, dest, sizeof(packet->dest));
    memcpy(packet->source, &mac, sizeof(mac));
    packet->length_type = __builtin_bswap16(type);
    return sizeof(struct etherFrame);
}

int createArpPacket(uint8_t *srcpr, uint8_t *dstpr, void** bufferPtr)
{
    const int bufferSize = sizeof(struct arp) + sizeof(struct etherFrame);
    void *buffer = calloc(1, bufferSize);
    *bufferPtr = buffer;
    const int frameSize = setupEthernetFrame(&broadcast, sizeof(struct arp), PROTOCOL_ARP, (struct etherFrame *)buffer);
    struct arp *packet = (struct arp *)((uint8_t*)buffer + frameSize);
    packet->htype = __builtin_bswap16(ETHERNET);
    packet->ptype = __builtin_bswap16(PROTOCOL_IP);
    packet->hlen = HARDWARE_ADRR_LEN;
    packet->plen = IP_ADRR_LEN;
    packet->opcode = __builtin_bswap16(ARP_REQUEST);
    memset(packet->dsthw, '\0', HARDWARE_ADRR_LEN);
    memcpy(&packet->srchw, &mac, sizeof(mac));
    memcpy(&packet->srcpr, srcpr, IP_ADRR_LEN);
    memcpy(&packet->dstpr, dstpr, IP_ADRR_LEN);
    return bufferSize;
}

int createIpPacket(uint32_t source, uint32_t destIp, uint8_t* destMac, char *data, uint16_t len, void** frameAddr)
{
    const int totalLen = sizeof(struct etherFrame) + sizeof(struct ip) + len;
    void *frame = calloc(1, totalLen);
    *frameAddr = frame;
    const int frameSize = setupEthernetFrame(destMac, sizeof(struct ip) + len, PROTOCOL_IP, (struct etherFrame*)frame);
    struct ip *packet = (struct ip *)((uint8_t*)frame + frameSize);
    packet->dest = destIp;
    packet->source = source;
    packet->len = __builtin_bswap16(len + sizeof(struct ip));
    packet->version_ihl = IP_VERSION << IP_VERSION_OFFSET | MIN_IP_HEADER_LEN;
    packet->flag_offset = __builtin_bswap16(0);
    packet->type = IP_NET_CONTROL | IP_HIGH_RELIABILITY;
    packet->id = __builtin_bswap16(1); // TODO: fix this
    memcpy(&packet->options_data, data, len);
    return totalLen;
}
