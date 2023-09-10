#include "stdlib.h"
#include "debug/display.h"
#include "interrupts/syscall.h"
#include "stdint.h"

// allocate uninitialized memory using syscall
void *malloc(const uint32_t size) {
  void *ptr = 0;

  __asm__ __volatile__("int $0x80"
                       : "=d"(ptr)
                       : "a"(SYSCALL_MALLOC), "b"(size));
  return ptr;
}

// free allocated memory at a pointer using syscall
void free(const void *ptr) {
  __asm__ __volatile__("int $0x80" : : "a"(SYSCALL_FREE), "b"(ptr));
}

// halts the system
void exit(const uint32_t status) {
  __asm__ __volatile__("int $0x80" : : "a"(SYSCALL_EXIT), "b"(status));
}
