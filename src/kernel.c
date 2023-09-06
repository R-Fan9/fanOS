#include "C/stdint.h"
#include "C/stdlib.h"
#include "debug/display.h"
#include "hal/tss.h"
#include "hal/user.h"
#include "interrupts/pit.h"
#include "interrupts/syscall.h"

void test_syscall();

__attribute__((section("kernel_main"))) void kmain(void) {
  print_string((uint8_t *)"Hello Kernel\n");

  // int32_t stack = 0;
  // __asm__ __volatile__("movl %%ESP, %0" : "=r"(stack));
  // tss_set_stack(0x10, stack);
  // enter_usermode();

  // test_syscall();

  while (1) {
    __asm__ __volatile__("hlt\n\t");
  }
}

void test_syscall() {

  __asm__ __volatile__("int $0x80" : : "a"(SYSCALL_TEST0));

  // for (uint32_t i = 0; i < 100; i++) {
  //   print_dec(i);
  // __asm__ __volatile__("int $0x80" : : "a"(SYSCALL_SLEEP), "b"(20));
  // }

  while (1)
    ;
}
