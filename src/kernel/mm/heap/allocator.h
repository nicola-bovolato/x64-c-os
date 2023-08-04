#ifndef HEAPALLOCATOR_BASIC_H
#define HEAPALLOCATOR_BASIC_H

#include <stddef.h>

#define HEAP_SIZE (100 * 0x400) // 100KiB

void  init_heap_allocator(void* start_address);
void* allocate(size_t size);
void  deallocate(void* address);
#endif
