#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "kernel.h"
 
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
    print("Welcome!", 8);
    print("Hit enter to start!", 19);
    while (get_input_keycode() != KEY_ENTER){
        continue;
    }
    print("Wait...", 7);
    memset(fb, '\0', pitch*framebuffer->height);
    cursorX = 0;
    cursorY = 0;
    checkAllBuses();
    hcf();
}