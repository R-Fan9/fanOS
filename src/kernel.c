#include "C/stdint.h"
#include "debug/display.h"
#include "memory/physical_mmngr.h"
#include "memory/virtual_mmngr.h"

__attribute__((section("kernel_main"))) void kmain(void) {
  // clear_screen();

  while (1) {
    __asm__ __volatile__("hlt\n\t");
  }
}
