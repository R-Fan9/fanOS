#include "syscall.h"
#include "C/stdbool.h"
#include "C/stdint.h"
#include "debug/display.h"
#include "interrupts/pit.h"
#include "memory/malloc.h"

uint32_t syscall_test0(syscall_regs_t regs);
uint32_t syscall_sleep(syscall_regs_t regs);
uint32_t syscall_malloc(syscall_regs_t regs);
uint32_t syscall_free(syscall_regs_t regs);

void *syscalls[MAX_SYSCALLS] = {
    syscall_test0,
    syscall_sleep,
    syscall_malloc,
    syscall_free,
};

__attribute__((naked)) void syscall_dispatcher(void) {

  __asm__ __volatile__(
      ".intel_syntax noprefix\n"

      ".equ MAX_SYSCALLS, 4\n" // have to define again, inline asm does not see
                               // the #define

      "cmp eax, MAX_SYSCALLS-1\n" // syscalls table is 0-based
      "ja invalid_syscall\n"      // invalid syscall number, skip and return

      "push eax\n"
      "push gs\n"
      "push fs\n"
      "push es\n"
      "push ds\n"
      "push ebp\n"
      "push edi\n"
      "push esi\n"
      "push edx\n"
      "push ecx\n"
      "push ebx\n"
      "push esp\n"
      "call [syscalls+eax*4]\n"
      "add esp, 4\n" // do not overwrite esp
      "pop ebx\n"
      "pop ecx\n"

      // do not overwrite EDX, as some syscalls e.g. malloc() will need it
      "add esp, 4\n"

      "pop esi\n"
      "pop edi\n"
      "pop ebp\n"
      "pop ds\n"
      "pop es\n"
      "pop fs\n"
      "pop gs\n"
      "add esp, 4\n" // save eax value in case; don't overwrite it
      "iretd\n"

      "invalid_syscall:\n"
      "mov eax, -1\n" // return -1 to indicate error
      "iretd\n"

      ".att_syntax");
}

uint32_t syscall_test0(syscall_regs_t regs) {
  clear_screen();
  print_string((uint8_t *)"test0 syscall; syscall # (EAX): ");
  print_hex(regs.eax);
  return EXIT_SUCCESS;
}

uint32_t syscall_sleep(syscall_regs_t regs) {
  uint32_t ticks = regs.ebx;

  // TODO - sleep() from PIT is not working here, could be due to syscall int
  // 0x80 is used, which prevents other interrupts (i.e. timer - IRQ0) from
  // executing
  for (uint32_t i = 0; i < ticks * 100000; i++) {
  }

  return EXIT_SUCCESS;
}

uint32_t syscall_malloc(syscall_regs_t regs) {
  uint32_t bytes = regs.ebx;

  if (is_malloc_inited() == false) {
    malloc_init(bytes);
  }

  void *ptr = malloc_next_block(bytes);
  merge_free_blocks();

  __asm__ __volatile__("mov %0, %%EAX" : : "r"(ptr));

  return EXIT_SUCCESS;
}

uint32_t syscall_free(syscall_regs_t regs) {
  void *ptr = (void *)regs.ebx;
  malloc_free(ptr);
  return EXIT_SUCCESS;
}
