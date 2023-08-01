#ifndef MEM_H
#define MEM_H


#include <stddef.h>


void* memset(void* destination, char value, size_t length);
void* memcpy(void* destination, const void* source, size_t length);
void* memmove(void* destination, const void* source, size_t length);

#endif
