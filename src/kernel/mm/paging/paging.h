#ifndef PAGING_H
#define PAGING_H


#include "../frame/allocator.h"
#include "page.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void identity_map(uint64_t page_flags, const void* frame_ptr, allocate_frame_t allocate_frame);
void map_page_to_frame(
    page_t page, uint64_t page_flags, const void* frame_ptr, allocate_frame_t allocate_frame
);
void        unmap_page(page_t page, deallocate_frame_t deallocate_frame, bool panic_on_empty);
const void* get_physical_address(void* virtual);

#endif
