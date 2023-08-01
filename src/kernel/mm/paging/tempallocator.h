#ifndef PAGING_TEMP_ALLOCATOR_H
#define PAGING_TEMP_ALLOCATOR_H


void        init_temp_allocator();
const void* allocate_temp_frame();
void        deallocate_temp_frame(const void* frame);

#endif
