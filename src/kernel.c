#include "C/stdint.h"
#include "debug/display.h"

__attribute__((section("kernel_main"))) void kmain(void){

  // test page fault
  // uint32_t *ptr = (uint32_t *)0xA0000000;
  // print_hex(*ptr);

  while (1) {
    __asm__ __volatile__("hlt\n\t");
  }
}
