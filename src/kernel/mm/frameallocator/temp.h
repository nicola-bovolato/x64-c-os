#ifndef FRAME_ALLOCATOR_TEMP_H
#define FRAME_ALLOCATOR_TEMP_H


void        init_temp_allocator();
const void* allocate_temp_frame();
void        deallocate_temp_frame(const void* frame);

#endif
