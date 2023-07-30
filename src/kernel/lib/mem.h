#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

void* memset(void* destination, uint8_t value, size_t length);
void* memcpy(void* destination, const void* source, size_t length);
void* memmove(void* destination, const void* source, size_t length);

#endif
