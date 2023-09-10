#include "stdint.h"

#ifndef STRING_H
#define STRING_H

uint8_t *strchr(const uint8_t *str, const uint8_t c);
uint8_t *strcpy(uint8_t *dst, const uint8_t *src);
int32_t strcmp(const uint8_t *str1, const uint8_t *str2);
int32_t strncmp(const uint8_t *str1, const uint8_t *str2, uint8_t len);
void *memset(void *buffer, const uint8_t byte, const uint32_t len);
void *memcpy(void *dst, const void *src, const uint32_t count);

#endif // !STRING_H
