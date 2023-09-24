#include <kernel.h>
#include "net.h"
#include <lib/lib.h>

extern uint8_t mac[6];
uint8_t gatewayMac[6];
uint8_t broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast

uint32_t addChecksum(uint32_t acc, uint16_t data)
{
    acc += __builtin_bswap16(data);
    if (acc >= 0x10000)
        acc -= 0xFFFF;
    return acc;
}

uint16_t finishChecksum(uint32_t acc)
{
    return __builtin_bswap16((uint16_t) ~(acc & 0xFFFF));
}

int setupEthernetFrame(uint8_t *dest, uint16_t type, struct etherFrame *packet)
{
    memcpy(&packet->dest, dest, sizeof(packet->dest));
    memcpy(&packet->source, &mac, sizeof(mac));
    packet->length_type = __builtin_bswap16(type);
    return sizeof(struct etherFrame);
}

int createArpPacket(uint8_t *srcpr, uint8_t *dstpr, void **bufferPtr)
{
    const int bufferSize = sizeof(struct arp) + sizeof(struct etherFrame);
    void *buffer = calloc(1, bufferSize);
    *bufferPtr = buffer;
    const int frameSize = setupEthernetFrame((uint8_t*)&broadcast, PROTOCOL_ARP, (struct etherFrame *)buffer);
    struct arp *packet = (struct arp *)((uint8_t *)buffer + frameSize);
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

int setupIpPacket(uint8_t *sourceIp, uint8_t *destIp, uint8_t *destMac, uint16_t len, uint8_t protocol, void *frameAddr)
{
    const int totalLen = sizeof(struct etherFrame) + sizeof(struct ip);
    void *frame = frameAddr;
    const int frameSize = setupEthernetFrame(destMac, PROTOCOL_IP, (struct etherFrame *)frame);
    struct ip *packet = (struct ip *)((uint8_t *)frame + frameSize);
    memcpy(&packet->dest, destIp, IP_ADRR_LEN);
    memcpy(&packet->source, sourceIp, IP_ADRR_LEN);
    packet->len = __builtin_bswap16(len + sizeof(struct ip));
    packet->version_ihl = IP_VERSION << IP_VERSION_OFFSET | MIN_IP_HEADER_LEN;
    packet->flag_offset = __builtin_bswap16(0);
    packet->type = IP_NET_CONTROL | IP_HIGH_RELIABILITY;
    packet->id = __builtin_bswap16(1); // TODO: fix this
    packet->ttl = 30;                  // 30 Secs till destruction
    packet->protocol = protocol;
    uint16_t *header = (uint16_t *)packet;
    uint32_t acc = 0xFFFF;
    for (size_t i = 0; i < MIN_IP_HEADER_LEN * 2; i++)
        acc = addChecksum(acc, header[i]);
    packet->header_checksum = finishChecksum(acc);
    return totalLen;
}

int createUdpPacet(uint16_t sourcePort, uint16_t destPort, uint8_t *sourceIp, uint8_t *destIp, uint8_t *destMac, char *data, uint16_t dataLen, void **frameAddr)
{
    const int totalLen = sizeof(struct etherFrame) + sizeof(struct ip) + sizeof(udp) + dataLen;
    void *frame = calloc(1, totalLen);
    *frameAddr = frame;
    const int ipLen = setupIpPacket(sourceIp, destIp, destMac, sizeof(udp) + dataLen, IP_PROTOCOL_UDP, frame);
    udp *packet = (udp *)((uint8_t *)frame + ipLen);
    packet->destPort = __builtin_bswap16(destPort);
    packet->sourcePort = __builtin_bswap16(sourcePort);
    packet->length = __builtin_bswap16(MIN_UDP_HEADER + dataLen);
    memcpy(&packet->data, data, dataLen);
    uint16_t *udpPacket = (uint16_t *)((uint8_t *)frame + sizeof(struct etherFrame) + sizeof(struct ip));
    uint32_t acc = 0x0000;
    struct ip *ipHeader = (struct ip *)((uint8_t *)frame + sizeof(struct etherFrame));
    acc = addChecksum(acc, ((uint16_t *)&(ipHeader->source))[0]);
    acc = addChecksum(acc, ((uint16_t *)&(ipHeader->source))[1]);
    acc = addChecksum(acc, ((uint16_t *)&(ipHeader->dest))[0]);
    acc = addChecksum(acc, ((uint16_t *)&(ipHeader->dest))[1]);
    acc = addChecksum(acc, (uint16_t)(ipHeader->protocol));
    acc = addChecksum(acc, (uint16_t)(ipHeader->len));
    for (size_t i = 0; i < (totalLen - sizeof(udp) - dataLen) / 2; i++)
        acc = addChecksum(acc, udpPacket[i]);
    packet->checksum = finishChecksum(acc);
    return totalLen;
}
