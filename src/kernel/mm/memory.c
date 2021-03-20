#include "memory.h"

#include "../panic.h"

#define MAX_USED_REGIONS 10

typedef struct {
    uint8_t* start;
    uint8_t* end;
} mem_region_t;

mem_region_t used_regions[MAX_USED_REGIONS];
size_t used_regions_size = 0;

uint8_t* free_frame = 0x0;
uint8_t* end_of_memory = 0x0;

void init_frame_allocator(void* mem_end) {
    end_of_memory = (uint8_t*) mem_end;
}

void frame_allocator_add_used_memory_region(void* mem_start, void* mem_end) {
    used_regions[used_regions_size] = (mem_region_t) {(uint8_t*) mem_start, (uint8_t*) mem_end};
    used_regions_size++;
    if(used_regions_size > MAX_USED_REGIONS) panic("Used memory regions exceed maximum allowed");
}

void* allocate_frame() {
    if(free_frame + FRAME_SIZE >= end_of_memory) panic("No free frames");

    // If the frame pointer overlaps a reseved memory region, it will be repositioned to end of that region
    for(size_t i = 0; i < used_regions_size; i++) {

        // 1: check if the current frame pointer is in the reserved region
        // 2: check if frame will overlap with the reserved region at some point
        // 3: check if frame will completely cover the reserved region
        if((free_frame >= used_regions[i].start && free_frame <= used_regions[i].end)
            || (free_frame + FRAME_SIZE > used_regions[i].start && free_frame + FRAME_SIZE <= used_regions[i].end)
            || (free_frame <= used_regions[i].start && free_frame + FRAME_SIZE >= used_regions[i].end))
        {
            free_frame = (uint8_t*)((size_t)used_regions[i].end & FRAME_MASK);
            free_frame += FRAME_SIZE;
        }
    }

    void* start_address = free_frame;

    free_frame += FRAME_SIZE;

    return start_address;
}

// to implement
void deallocate_frame(void* address) {}
