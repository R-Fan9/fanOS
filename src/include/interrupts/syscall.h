#include "C/stdint.h"

#ifndef SYSCALL_H
#define SYSCALL_H

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define MAX_SYSCALLS 5

typedef enum {

  SYSCALL_TEST0 = 0,
  SYSCALL_SLEEP = 1,
  SYSCALL_MALLOC = 2,
  SYSCALL_FREE = 3,
  SYSCALL_PRINT = 4,

} syscall_numbers;

// registers pushed onto stack when a syscall function is called from the
// syscall dispatcher
typedef struct {
  uint32_t esp;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
  uint32_t esi;
  uint32_t edi;
  uint32_t ebp;
  uint32_t ds;
  uint32_t es;
  uint32_t fs;
  uint32_t gs;
  uint32_t eax;
} __attribute__((packed)) syscall_regs_t;

__attribute__((naked)) void syscall_dispatcher(void);

#endif // !SYSCALL_H
