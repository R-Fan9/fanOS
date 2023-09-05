#include "user.h"

__attribute__((naked)) void enter_usermode(void) {

  __asm__ __volatile__(
      ".intel_syntax noprefix\n"

      "cli\n"

      "mov ax, 0x23\n" // user mode data selector is 0x20 (GDT entry 3), with
                       // RPL 3
      "mov ds, ax\n"
      "mov es, ax\n"
      "mov fs, ax\n"
      "mov gs, ax\n"

      "push 0x23\n" // SS
      "push esp\n"  // ESP
      "pushfd\n"    // EFLAGS

      "pop eax\n"
      "or eax, 0x200\n" // enable IF in EFLAGS
      "push eax\n"

      "push 0x1B\n"    // CS, user mode code selector is 0x18, with RPL 3
      "lea eax, [a]\n" // EIP first
      "push eax\n"
      "iretd\n"

      "a:\n"
      "add esp, 4");
}
