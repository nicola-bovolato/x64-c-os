#ifndef FRAME_ALLOCATOR_TEMP_H
#define FRAME_ALLOCATOR_TEMP_H


void  init_temp_allocator();
void* allocate_temp_frame();
void  deallocate_temp_frame(void* frame);

#endif
