#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "kernel.h"
#include "net/net.h"

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

static volatile struct limine_kernel_address_request adrr_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0};

static volatile struct limine_boot_time_request time_request = {
    .id = LIMINE_BOOT_TIME_REQUEST,
    .revision = 0};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0};

uint64_t kernelBaseVMem;
uint64_t kernelBasePMem;
uint64_t hhdm_offset;

extern uint8_t broadcast[];
extern uint8_t gatewayMac[6];
uint8_t gatewayIp[] = {10, 0, 2, 2};

const char *google = "\6google\3com";

// Halt and catch fire function.
static void
hcf(void)
{
    asm("cli");
    for (;;)
    {
        asm("hlt");
    }
}
int pixelwidth;
int pitch;
int height;
uint32_t *fb;

int cursorX = 0;
int cursorY = 0;

extern PSF_font *font;

struct limine_framebuffer *framebuffer;

uint64_t getPhysicalMemKernel(void *object)
{
    return (((uint64_t)object) - kernelBaseVMem) + kernelBasePMem;
}

uint64_t getPhysicalMemHeap(void *object)
{
    return (uint64_t)object - hhdm_offset;
}

uint64_t getVirtualMemHeap(void *object)
{
    return (uint64_t)object + hhdm_offset;
}

uint64_t getVirtualMemKernel(void *object)
{
    return (uint64_t)object + kernelBaseVMem - kernelBasePMem;
}

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void)
{
    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
    {
        hcf();
    }

    // Fetch the first framebuffer.
    framebuffer = framebuffer_request.response->framebuffers[0];
    pixelwidth = framebuffer->width;
    pitch = framebuffer->pitch;
    fb = framebuffer->address;
    height = framebuffer->height;
    kernelBasePMem = adrr_request.response->physical_base;
    kernelBaseVMem = adrr_request.response->virtual_base;
    hhdm_offset = hhdm_request.response->offset;
    uint64_t largestLength = 0;
    uint64_t largestBase;
    for (int i = 0; i < memmap_request.response->entry_count; i++)
    {
        if (memmap_request.response->entries[i]->type == LIMINE_MEMMAP_USABLE && memmap_request.response->entries[i]->length > largestLength)
        {
            largestBase = memmap_request.response->entries[i]->base;
            largestLength = memmap_request.response->entries[i]->length;
        }
    }
    init_mem((void *)(largestBase + hhdm_offset));
    psf_init();
    print("Hit enter to continue boot...", 29);
    waitForUser();
    memset(fb, '\0',  pitch * framebuffer->height);
    cursorX = 0;
    cursorY = 0;
    checkAllBuses();
    uint8_t exampleOurIp[] = {10, 0, 2, 15};
    uint8_t cloudflare[] = {1, 1, 1, 1};
    void *frame;
    int len = createArpPacket(&exampleOurIp, &gatewayIp, &frame);
    nicTransmit(frame, len, 0x00, 0, 0);
    free(frame);
    void* responseFrame;
    while ((len = nicReadFrame(&responseFrame)) == 0x00)
        sleep(0xFF);
    struct etherFrame *eFrame = (struct etherFrame *)responseFrame;
    if (__builtin_bswap16(eFrame->length_type) != PROTOCOL_ARP)
    {
        print("No ARP", 6);
        hcf();
    }
    print("ARP Response", 12);
    struct arp *arpPacket = (struct arp *)((uint8_t *)responseFrame + sizeof(struct etherFrame));
    memcpy(&gatewayMac, &arpPacket->srchw, sizeof(gatewayMac));
    free(frame);
    void* pingFrame;
    len = createPing(&exampleOurIp, &cloudflare, &gatewayMac, &pingFrame);
    nicTransmit(pingFrame, len, 0x00, 0, 0);
    free(pingFrame);
    void *udpFrame;
    const int dnsLen = sizeof(dnsHeader) + 4 * sizeof(dnsQuestion);
    uint8_t *dns = dnsQuery(google);
    len = createUdpPacet(100, 53, &exampleOurIp, &cloudflare, &gatewayMac, dns, dnsLen, &udpFrame);
    nicTransmit(udpFrame, len, 0, 0, 0);
    free(udpFrame);
    while ((len = nicReadFrame(udpFrame)) == 0x00)
        sleep(0xFF);
    uint8_t googleIp[4];
    memcpy(&googleIp, (char *)udpFrame + len - 4, 4);
    len = createUdpPacet(100, 1, &exampleOurIp, &googleIp, &gatewayMac, "Hey Google!", 11, &udpFrame);
    nicTransmit(udpFrame, len, 0, 0, 0);
    hcf();
}

void waitForUser()
{
    while (get_input_keycode() != KEY_ENTER)
        continue;
}
