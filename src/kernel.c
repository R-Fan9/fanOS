#include "C/stdint.h"
#include "debug/display.h"
#include "memory/physical_mmngr.h"
#include "memory/virtual_mmngr.h"

physical_addr virt_to_phys(virtual_addr addr);

__attribute__((section("kernel_main"))) void kmain(void) {
  // clear_screen();

  while (1) {
    __asm__ __volatile__("hlt\n\t");
  }
}
