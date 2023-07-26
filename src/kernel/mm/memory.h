#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#define FRAME_SIZE 0x1000
// if FRAME_SIZE = 0x1000, FRAME_MASK = 0xFFFFFFFFFFFFF000
#define FRAME_MASK (~((size_t)FRAME_SIZE - 1))

typedef struct {
    uint8_t* start;
    uint8_t* end;
} mem_region_t;

void  init_frame_allocator(mem_region_t mem);
void  frame_allocator_add_used_memory_region(mem_region_t mem);
void* allocate_frame();
void  deallocate_frame(void* frame);

#endif
