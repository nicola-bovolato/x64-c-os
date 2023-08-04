#include "malloc.h"
#include "../mm/heap/allocator.h"


void* malloc(size_t size) { return allocate(size); }

void free(void* address) { deallocate(address); }
