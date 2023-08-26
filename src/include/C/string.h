#include "stdint.h"

#ifndef STRING_H
#define STRING_H

int32_t strncmp(const uint8_t *str1, const uint8_t *str2, uint8_t len);
void *memset(void *buffer, const uint8_t byte, const uint32_t len);
void *memcpy(void *dst, const void *src, const uint32_t count);

#endif // !STRING_H
