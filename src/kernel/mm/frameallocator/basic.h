#ifndef FRAME_ALLOCATOR_BASIC_H
#define FRAME_ALLOCATOR_BASIC_H

#include "../memregion.h"
#include <stddef.h>


void init_frame_allocator(
    mem_region_t system_memory, const mem_region_t _used_regions[], size_t _used_regions_size
);
const void* allocate_frame();
void        deallocate_frame(const void* frame);

#endif
