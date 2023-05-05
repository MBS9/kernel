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
    init_mem(framebuffer->address+pitch*framebuffer->height + 20);
    psf_init();
    print("Welcome to My OS", 16);
    print("Hello", 5);
    while (1) {
        if (get_input_keycode() == KEY_A) {
            putCharAuto('a');
        } else if (get_input_keycode() == KEY_BACKSPACE) {
            if (cursorX != 0) {
                cursorX--;
            } else if (cursorX == 0 && cursorY != 0) {
                cursorY --;
                cursorX = framebuffer->width/font->width;
            }
            putchar((unsigned short)'a', cursorX, cursorY, 0x000000, 0x000000);
        }
    }
    hcf();
}