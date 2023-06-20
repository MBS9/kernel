#include "kernel.h"

extern void isr_wrapper();

int ox, oy;
int keep = 1;
int new = 0;
extern int cursorX;
extern int cursorY;

void interrupt_handler(void)
{
    char keyCode = get_input_keycode();
    ox = cursorX;
    oy = cursorY;
    if (keyCode == KEY_DOWN && cursorY < 3){
        cursorY = cursorY+1;
        new = 1;
    } else if (keyCode == KEY_UP && cursorY > 0) {
        cursorY = cursorY -1;
        new = 1;
    } else if (keyCode == KEY_LEFT && cursorX >0) {
        cursorX -=1;
        new = 1;
    } else if (keyCode == KEY_RIGHT && cursorX < 7) {
        cursorX += 1;
        new = 1;
    } else if (keyCode == KEY_SPACE) {
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