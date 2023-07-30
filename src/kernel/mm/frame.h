#ifndef FRAME_H
#define FRAME_H

#include "mm.h"
#include <stddef.h>

#define PAGE_SIZE 0x1000
// if PAGE_SIZE = 0x1000, FRAME_MASK = 0xFFFFFFFFFFFFF000
#define FRAME_MASK (~((size_t)PAGE_SIZE - 1))

void  init_frame_allocator();
void* allocate_frame();
void  deallocate_frame(void* frame);

#endif
