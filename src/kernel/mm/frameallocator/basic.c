#include "basic.h"
#include "../../lib/mem.h"
#include "../../log.h"
#include "../paging/page.h"
#include <stdint.h>


// if PAGE_SIZE = 0x1000, FRAME_MASK = 0xFFFFFFFFFFFFF000
#define FRAME_MASK (~((size_t)PAGE_SIZE - 1))

#define MAX_USED_REGIONS 20

static mem_region_t used_regions[MAX_USED_REGIONS];
static size_t       used_regions_size = 0;

static uint8_t* next_free_frame = (uint8_t*)-1;
static uint8_t* end_of_memory   = 0x0;


// required to use the other functions
void init_frame_allocator(
    mem_region_t system_memory, const mem_region_t _used_regions[], size_t _used_regions_size
) {
    if (_used_regions_size > MAX_USED_REGIONS)
        PANIC(
            "Used memory regions (%d) exceed maximum allowed (%d)",
            used_regions_size,
            MAX_USED_REGIONS
        );

    next_free_frame   = system_memory.start;
    end_of_memory     = system_memory.end;
    used_regions_size = _used_regions_size;
    memcpy(used_regions, _used_regions, sizeof(mem_region_t) * used_regions_size);

#ifdef DEBUG
    DEBUG("Used memory regions:\n");
    for (size_t i = 0; i < used_regions_size; i++)
        DEBUG("\t%d) start = %p, end = %p\n", i, used_regions[i].start, used_regions[i].end);
#endif
}


// Selects a free frame based excluding used memory regions and returns it
// Increases the next free frame pointer
const void* allocate_frame() {
    if (next_free_frame + PAGE_SIZE >= end_of_memory)
        PANIC("No free frames (%x > %x)", next_free_frame, end_of_memory);

    // If the frame pointer overlaps a reseved memory region
    // it will be repositioned to end of that region
    for (size_t i = 0; i < used_regions_size; i++) {
        // 1: check if the current frame pointer is in the reserved region
        // 2: check if frame will overlap with the reserved region at some point
        // 3: check if frame will completely cover the reserved region
        if ((next_free_frame >= used_regions[i].start && next_free_frame <= used_regions[i].end)
            || (next_free_frame + PAGE_SIZE > used_regions[i].start
                && next_free_frame + PAGE_SIZE <= used_regions[i].end)
            || (next_free_frame <= used_regions[i].start
                && next_free_frame + PAGE_SIZE >= used_regions[i].end)) {
            next_free_frame  = (uint8_t*)((size_t)used_regions[i].end & FRAME_MASK);
            next_free_frame += PAGE_SIZE;
        }
    }

    void* start_address = next_free_frame;

    next_free_frame += PAGE_SIZE;

    return start_address;
}

// to implement
void deallocate_frame(const void* address) {}
