#ifndef FRAME_ALLOCATOR_BASIC_H
#define FRAME_ALLOCATOR_BASIC_H

#include <stddef.h>


void  init_frame_allocator();
void* allocate_frame();
void  deallocate_frame(void* frame);

#endif
