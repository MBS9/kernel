#include "kernel.h"
#include <inttypes.h>

void wait_for_io(uint32_t timer_count)
{
  while(1){
    asm volatile("nop");
    timer_count--;
    if(timer_count <= 0)
      break;
    }
}

void sleep(uint32_t timer_count)
{
  wait_for_io(timer_count);
}

char get_input_keycode()
{
  char ch = 0;
  ch = inb(KEYBOARD_PORT);
  if(ch == KEY_DOWN || ch == KEY_UP) {
    sleep(0x6FFFFFF);
  } else if (ch > 0) {
    sleep(0x00FFFFF);
  }
  return ch;
}
