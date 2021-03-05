#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stddef.h>

void *memset (void *destination, uint8_t value, size_t length);
void *memcpy (const void *source, void *destination, size_t length);
void *memmove(const void *source, void *destination, size_t length);

#endif
