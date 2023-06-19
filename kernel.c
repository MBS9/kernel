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

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

extern void setGdt(uint64_t limit, uint64_t base);

extern void reloadSegments();

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

uint64_t gdt[3];

int cursorX = 0;
int cursorY = 0;

extern PSF_font* font;
extern char _stext;
extern char _etext;
extern char _srodata;
extern char _erodata;
extern char _sdata;
extern char _edata;

struct limine_framebuffer* framebuffer;

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
    uint64_t largestBase = 0;
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE && memmap_request.response->entries[i]->length > largestLength) {
            largestBase = memmap_request.response->entries[i]->base;
            largestLength = memmap_request.response->entries[i]->length;
        }
    }
    if (largestBase == 0) {
        hcf();
        return;
    }
    init_mem((void*)largestBase);
    psf_init();
    gdt[0] = create_descriptor(0, 0, 0);
    gdt[1] = create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL0));
    gdt[2] = create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL0));
    print("Setting GDT", 11);
    sleep(0x3FFFFFF);
    setGdt(3*sizeof(uint64_t)-1, (uint64_t)&gdt);
    print("Reloading Segments", 18);
    sleep(0x3FFFFFF);
    reloadSegments();
    print("Welcome!", 8);
    print("Hit enter to start!", 19);
    waitForUser();
    print("Wait...", 7);
    memset(fb, '\0', pitch*framebuffer->height);
    cursorX = 0;
    cursorY = 0;
    checkAllBuses();
    void* test[RING_ELEMENT_NO];
    char temp[30];
    memset(&temp, (int)'A', 30);
    for (int i =0; i<RING_ELEMENT_NO; i++) {
        test[i] = &temp;
    }
    nicTransmit(test, 30);
    hcf();
}

void waitForUser() {
    while (get_input_keycode() != KEY_ENTER){
        continue;
    }
}
