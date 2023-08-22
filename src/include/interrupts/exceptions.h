#include "C/stdint.h"
#include "hal/idt.h"

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

__attribute__((interrupt)) void page_fault_handler(int_frame_t *frame,
                                                   uint32_t error_code);

#endif // !EXCEPTIONS_H
