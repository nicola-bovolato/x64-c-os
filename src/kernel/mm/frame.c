#include "frame.h"
#include "../panic.h"
#include <stdint.h>

#define MAX_USED_REGIONS 10

mem_region_t used_regions[MAX_USED_REGIONS];
size_t       used_regions_size = 0;

uint8_t* free_frame    = (uint8_t*)-1;
uint8_t* end_of_memory = 0x0;

void init_frame_allocator(mem_region_t mem) {
    free_frame    = mem.start;
    end_of_memory = mem.end;
}

void frame_allocator_add_used_memory_region(mem_region_t mem) {
    used_regions[used_regions_size] = mem;
    used_regions_size++;
    if (used_regions_size > MAX_USED_REGIONS)
        panic("Used memory regions exceed maximum allowed");
}

void* allocate_frame() {
    if (free_frame + FRAME_SIZE >= end_of_memory) panic("No free frames");

    // If the frame pointer overlaps a reseved memory region, it will be
    // repositioned to end of that region
    for (size_t i = 0; i < used_regions_size; i++) {
        // 1: check if the current frame pointer is in the reserved region
        // 2: check if frame will overlap with the reserved region at some point
        // 3: check if frame will completely cover the reserved region
        if ((free_frame >= used_regions[i].start
             && free_frame <= used_regions[i].end)
            || (free_frame + FRAME_SIZE > used_regions[i].start
                && free_frame + FRAME_SIZE <= used_regions[i].end)
            || (free_frame <= used_regions[i].start
                && free_frame + FRAME_SIZE >= used_regions[i].end)) {
            free_frame  = (uint8_t*)((size_t)used_regions[i].end & FRAME_MASK);
            free_frame += FRAME_SIZE;
        }
    }

    void* start_address = free_frame;

    free_frame += FRAME_SIZE;

    return start_address;
}

// to implement
void deallocate_frame(void* address) {}
