#ifndef FRAME_H
#define FRAME_H

#include "memlayout.h"
#include <stddef.h>

#define FRAME_SIZE 0x1000
// if FRAME_SIZE = 0x1000, FRAME_MASK = 0xFFFFFFFFFFFFF000
#define FRAME_MASK (~((size_t)FRAME_SIZE - 1))

void  init_frame_allocator();
void* allocate_frame();
void  deallocate_frame(void* frame);

#endif
