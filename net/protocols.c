#include <kernel.h>
#include "net.h"
#include <lib/lib.h>

extern uint8_t mac[6];
uint8_t gatewayMac[6];
uint8_t broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast

uint32_t addChecksum(uint32_t acc, uint16_t data)
{
    acc += (uint32_t)data;
    return acc;
}

uint16_t finishChecksum(uint32_t acc)
{
    while (acc >> 16)
        acc = (acc & 0xFFFF) + (acc >> 16);
    return (uint16_t) ~(acc & 0xFFFF);
}

uint32_t digestBuffer(uint16_t *buffer, int n, uint32_t acc)
{
    for (size_t i = 0; i < n; i++)
    {
        acc = addChecksum(acc, buffer[i]);
    }
    return acc;
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
    const int frameSize = setupEthernetFrame((uint8_t *)&broadcast, PROTOCOL_ARP, (struct etherFrame *)buffer);
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

int createPing(uint8_t *sourceIp, uint8_t *destIp, uint8_t *destMac, void **frameAddr)
{
    const int totalLen = sizeof(icmp_ping) + ICMP_PING_DATA_LEN + sizeof(struct ip) + sizeof(struct etherFrame);
    void *frame = calloc(1, totalLen);
    *frameAddr = frame;
    const int ipLen = setupIpPacket(sourceIp, destIp, destMac, sizeof(icmp_ping) + ICMP_PING_DATA_LEN, IP_PROTOCOL_ICMP, frame);
    icmp_ping *ping = (uint8_t *)frame + ipLen;
    ping->type = 8;
    memset(&ping->data, ICMP_PING_DATA_LEN, 'A');
    uint32_t acc = 0;
    acc = digestBuffer((uint16_t *)ping, (sizeof(icmp_ping) + ICMP_PING_DATA_LEN) / 2, acc);
    ping->checksum = finishChecksum(acc);
    return totalLen;
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
    packet->ttl = IP_TTL;
    packet->protocol = protocol;
    uint16_t *header = (uint16_t *)packet;
    uint32_t acc = 0xFFFF;
    acc = digestBuffer(header, MIN_IP_HEADER_LEN * 2, acc);
    packet->header_checksum = finishChecksum(acc);
    return totalLen;
}

int createUdpPacet(uint16_t sourcePort, uint16_t destPort, uint8_t *sourceIp, uint8_t *destIp, uint8_t *destMac, char *data, uint16_t dataLen, void **frameAddr)
{
    const uint16_t udpSize = sizeof(udp) + dataLen;
    int padding = 0;
    if (udpSize & 1)
        padding = 1;
    const int totalLen = sizeof(struct etherFrame) + sizeof(struct ip) + udpSize;
    void *frame = calloc(1, totalLen + padding);
    *frameAddr = frame;
    const int ipLen = setupIpPacket(sourceIp, destIp, destMac, udpSize, IP_PROTOCOL_UDP, frame);
    udp *packet = (udp *)((uint8_t *)frame + ipLen);
    packet->destPort = __builtin_bswap16(destPort);
    packet->sourcePort = __builtin_bswap16(sourcePort);
    packet->length = __builtin_bswap16(udpSize);
    memcpy(&packet->data, data, dataLen);
    uint32_t acc = 0;
    struct ip *ipHeader = (struct ip *)((uint8_t *)frame + sizeof(struct etherFrame));

    acc = digestBuffer((uint16_t *)packet, (udpSize + padding) / 2, acc);
    acc = digestBuffer((uint16_t *)&ipHeader->source, 4, acc); // Source and dest ip
    acc = addChecksum(acc, __builtin_bswap16((uint16_t)ipHeader->protocol));
    acc = addChecksum(acc, __builtin_bswap16(udpSize));
    packet->checksum = finishChecksum(acc);
    return totalLen;
}

int checkIPChecksum(uint16_t *packet)
{
    uint32_t acc = 0;
    acc = digestBuffer(packet, MIN_IP_HEADER_LEN * 2, acc);
    return finishChecksum(acc) == 0;
}

int checkUdpChecksum(uint8_t *packet, uint16_t len)
{
    int padding = 0;
    if (len & 1)
        padding = 1;
    const struct ip *ipPacket = (struct ip *)(packet + sizeof(struct etherFrame));
    if (checkIPChecksum((uint16_t *)ipPacket) == 0)
        return 0;
    udp *udpPacket = (udp *)(packet + sizeof(struct etherFrame) + sizeof(struct ip));
    uint32_t acc = 0;
    acc = digestBuffer((uint16_t *)udpPacket, (len + padding) / 2, acc);
    struct ip *ipHeader = (struct ip *)(packet + sizeof(struct etherFrame));
    acc = digestBuffer((uint16_t *)&ipHeader->source, 4, acc); // Source and dest ip
    acc = addChecksum(acc, __builtin_bswap16((uint16_t)ipHeader->protocol));
    acc = addChecksum(acc, __builtin_bswap16(len));
    return finishChecksum(acc) == 0;
}

uint8_t *dnsQuery(char *inputDomain)
{
    const int inputLen = strlen(inputDomain, '\0') + 1;
    char *domain = calloc(inputLen + 1, 1);
    memcpy(domain + 1, inputDomain, inputLen);
    char *p = domain + 1;
    char *lenPtr = domain;
    char hlen = 0;
    while (*p)
    {
        if (*p == '.')
        {
            *lenPtr = hlen;
            lenPtr = p;
            hlen = -1;
        }
        p++;
        hlen++;
    }
    *lenPtr = hlen;

    const int dnsLen = sizeof(dnsHeader) + sizeof(dnsQuestion);
    dnsHeader *dns = calloc(dnsLen, 1);
    dnsQuestion *question = (uint8_t *)dns + sizeof(dnsHeader);
    dns->id = __builtin_bswap16(1);
    dns->info = __builtin_bswap16(DNS_QUERY | DNS_STD_QUERY | DNS_RD);
    dns->QdCount = __builtin_bswap16(1);
    memcpy(&question->label, domain, LABEL_LEN);
    question->qclass = __builtin_bswap16(QCLASS_INTERNET);
    question->qtype = __builtin_bswap16(QTYPE_A);
    return dns;
}

int parseDnsResponse(uint8_t *ip[4], uint8_t *packet)
{
    const udp *udpPacket = (udp *)(packet + sizeof(struct etherFrame) + sizeof(struct ip));
    if (checkUdpChecksum(packet, __builtin_bswap16(udpPacket->length)) == 0)
        return 0;
    dnsHeader *dns = (dnsHeader *)((char *)udpPacket + sizeof(udp));
    if (dns->AnCount == 0)
        return 0;
    dnsAnswer *answer = (dnsAnswer *)((char *)dns + sizeof(dnsHeader) + sizeof(dnsQuestion));
    if (answer->type != __builtin_bswap16(QTYPE_A))
        return 0;
    if (answer->class != __builtin_bswap16(QCLASS_INTERNET))
        return 0;
    if (answer->rdlength != __builtin_bswap16(4))
        return 0;
    memcpy(ip, &answer->rdata, __builtin_bswap16(answer->rdlength));
    return 1;
}
