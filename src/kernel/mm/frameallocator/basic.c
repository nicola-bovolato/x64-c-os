#include "basic.h"
#include "../../lib/sort.h"
#include "../../log.h"
#include "../memregion.h"
#include "../multiboot2.h"
#include "../paging/page.h"
#include <stdint.h>


// if PAGE_SIZE = 0x1000, FRAME_MASK = 0xFFFFFFFFFFFFF000
#define FRAME_MASK (~((size_t)PAGE_SIZE - 1))

#define MAX_USED_REGIONS 20

static mem_region_t used_regions[MAX_USED_REGIONS];
static size_t       used_regions_size = 0;

static uint8_t* next_free_frame = (uint8_t*)-1;
static uint8_t* end_of_memory   = 0x0;


static inline int compare_mem_regions(const void* a, const void* b);


// required to use all of the folowing functions
void init_frame_allocator() {
    mem_region_t system_memory = get_system_mem_region();
    next_free_frame            = system_memory.start;
    end_of_memory              = system_memory.end;

    used_regions_size = get_used_mem_regions(used_regions);
    used_regions[used_regions_size++]
        = (mem_region_t){.start = (uint8_t*)VGA_MEM_START, .end = (uint8_t*)VGA_MEM_END};
    used_regions[used_regions_size++] = get_multiboot_mem_region();
    used_regions[used_regions_size++] = get_kernel_mem_region();
    if (used_regions_size > MAX_USED_REGIONS)
        PANIC(
            "Used memory regions (%d) exceed maximum allowed (%d)",
            used_regions_size,
            MAX_USED_REGIONS
        );

    qsort(used_regions, used_regions_size, sizeof used_regions[0], compare_mem_regions);
#ifdef DEBUG
    DEBUG("Used memory regions:\n");
    for (size_t i = 0; i < used_regions_size; i++)
        DEBUG("\t%d) start = %p, end = %p\n", i, used_regions[i].start, used_regions[i].end);
#endif
}


// Selects a free frame based excluding used memory regions and returns it
// Increases the next free frame pointer
void* allocate_frame() {
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
void deallocate_frame(void* address) {}

static inline int compare_mem_regions(const void* a, const void* b) {
    if (((mem_region_t*)a)->start > ((mem_region_t*)b)->start) return 1;
    if (((mem_region_t*)a)->start < ((mem_region_t*)b)->start) return -1;
    return 0;
}
