#ifndef PAGING_H
#define PAGING_H


#include "../frameallocator/frame.h"
#include "page.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


void identity_map(
    page_table_t*    table4_ptr,
    uint16_t         page_flags,
    uint8_t*         frame_ptr,
    allocate_frame_t allocate_frame
);
void map_page_to_frame(
    page_table_t*    table4_ptr,
    page_t           page,
    uint16_t         page_flags,
    uint8_t*         frame_ptr,
    allocate_frame_t allocate_frame
);
void unmap_page(
    page_table_t* table4_ptr, page_t page, deallocate_frame_t deallocate_frame, bool panic_on_empty
);
void* get_physical_address(page_table_t* table4_ptr, void* virtual);

#endif
