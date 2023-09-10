#include "stdint.h"

#ifndef STDLIB_H
#define STDLIB_H

void *malloc(const uint32_t size);
void free(const void *ptr);
void exit(const uint32_t status);

#endif // !STDLIB_H
