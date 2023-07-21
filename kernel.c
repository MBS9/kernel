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
    .revision = 0
};

static volatile struct limine_kernel_address_request adrr_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static volatile struct limine_boot_time_request time_request = {
    .id = LIMINE_BOOT_TIME_REQUEST,
    .revision = 0
};

struct tm bootTime;
char bootTimeString[10];
uint64_t kernelBaseVMem;
uint64_t kernelBasePMem;

// Halt and catch fire function.
static void hcf(void) {
    asm ("cli");
    for (;;) {
        asm ("hlt");
    }
}
int pixelwidth;
int pitch;
int height;
uint32_t *fb;

int cursorX = 0;
int cursorY = 0;

extern PSF_font* font;

struct limine_framebuffer* framebuffer;

uint64_t getPhysicalMem(void* object){
    return (((uint64_t)object)-kernelBaseVMem)+kernelBasePMem;
}

// The following will be our kernel's entry point.
// If renaming _start() to something else, make sure to change the
// linker script accordingly.
void _start(void) {
    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
 
    // Fetch the first framebuffer.
    framebuffer = framebuffer_request.response->framebuffers[0];
    pixelwidth = framebuffer->width;
    pitch = framebuffer->pitch;
    fb = framebuffer->address;
    height = framebuffer->height;
    uint64_t largestLength = 0;
    uint64_t largestBase;
    for (int i = 0; i < memmap_request.response->entry_count; i++) {
        if (memmap_request.response->entries[i]->type == LIMINE_MEMMAP_USABLE && memmap_request.response->entries[i]->length > largestLength) {
            largestBase = memmap_request.response->entries[i]->base;
            largestLength = memmap_request.response->entries[i]->length;
        }
    }
    init_mem(largestBase);
    psf_init();
    print("Hit enter to continue boot...", 30);
    waitForUser();
    memset(fb, '\0', pitch*framebuffer->height);
    cursorX = 0;
    cursorY = 0;
    kernelBasePMem = adrr_request.response->physical_base;
    kernelBaseVMem = adrr_request.response->virtual_base;
    checkAllBuses();
    char* temp = calloc(50, sizeof(char));
    memset(temp, (int)'A', 50);
    nicTransmit(temp, 50);
    hcf();
}

void waitForUser() {
    while (get_input_keycode() != KEY_ENTER){
        continue;
    }
}
