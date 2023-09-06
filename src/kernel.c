#include "C/stdint.h"
#include "C/stdlib.h"
#include "debug/display.h"
#include "hal/tss.h"
#include "interrupts/pit.h"
#include "interrupts/syscall.h"

__attribute__((section("kernel_main"))) void kmain(void) {
  __asm__ __volatile__("int $0x80" : : "a"(SYSCALL_TEST0));

  while (1) {
  }
}
