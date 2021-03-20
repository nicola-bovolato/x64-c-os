#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

#define FRAME_SIZE 0x1000
#define FRAME_MASK (~((size_t)FRAME_SIZE - 1)) // if FRAME_SIZE = 0x1000, FRAME_MASK = 0xFFFFFFFFFFFFF000

void  init_frame_allocator(void* end_of_memory);
void  frame_allocator_add_used_memory_region(void* mem_start, void* mem_end);
void* allocate_frame();
void  deallocate_frame(void* frame);

#endif
