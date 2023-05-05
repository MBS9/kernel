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
    print("Hey There!", 10);
    print("Welcome!", 8);
    print("Hit enter to start!", 19);
    while (get_input_keycode() != KEY_ENTER){
        continue;
    }
    print("Wait...", 7);
    memset(fb, '\0', pitch*framebuffer->height);
    for (cursorY = 0; cursorY<4; cursorY++) {
        cursorX = 0;
        for (int i = 0; i <8; i++) {
            putCharAuto('.', 0xFFFFFF, 0xFFFFFF);
        }
    }    
    cursorX = 0;
    cursorY = 4;
    print("Enjoy!", 6);
    cursorX = 0;
    cursorY = 0;
    int ox, oy;
    int keep = 1;
    int new = 0;
    while (1)
    {
        ox = cursorX;
        oy = cursorY;
        if (get_input_keycode() == KEY_DOWN && cursorY < 3){
            cursorY = cursorY+1;
            new = 1;
        } else if (get_input_keycode() == KEY_UP && cursorY > 0) {
            cursorY = cursorY -1;
            new = 1;
        } else if (get_input_keycode() == KEY_LEFT && cursorX >0) {
            cursorX -=1;
            new = 1;
        } else if (get_input_keycode() == KEY_RIGHT && cursorX < 7) {
            cursorX += 1;
            new = 1;
        } else if (get_input_keycode() == KEY_SPACE) {
            keep = 1;
        }
        if (!keep && new) {
            putchar('.', ox, oy, 0xFFFFFF, 0xFFFFFF);
        }
        if (keep && new) {
            putchar('x', ox, oy, 0x0000FF, 0xFFFF00);
        }
        if (new) {
            putchar('X', cursorX, cursorY, 0x0000FF, 0xFFFF00);
            new = 0;
            keep = 0;
        }
    }
    
    /*
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
    */
    hcf();
}