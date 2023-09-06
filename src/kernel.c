#include "C/stdint.h"
#include "C/stdlib.h"
#include "debug/display.h"
#include "hal/tss.h"
#include "interrupts/pit.h"
#include "interrupts/syscall.h"

// extern void jump_usermode();

__attribute__((section("kernel_main"))) void kmain(void) {
  print_string((uint8_t *)"Hello Kernel\n");

  // int32_t stack = 0;
  // __asm__ __volatile__("movl %%ESP, %0" : "=r"(stack));
  // tss_set_stack(0x10, stack);
  // jump_usermode();

  while (1) {
    __asm__ __volatile__("hlt\n\t");
  }
}

// void test_usermode(void) {
//   __asm__ __volatile__("int $0x80" : : "a"(SYSCALL_TEST0));
//
//   while (1) {
//   }
// }
