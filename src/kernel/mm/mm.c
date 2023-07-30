#include "mm.h"
#include "../drivers/tty.h"
#include "frameallocator/basic.h"
#include "frameallocator/temp.h"
#include "multiboot2.h"
#include "paging/remap.h"


void init_mm(void* multiboot_header) {
    init_multiboot_info(multiboot_header);

    // Initialize frame allocators
    mem_region_t system_memory = get_system_mem_region();

    mem_region_t used_mem_regions[20];
    size_t       used_mem_regions_size = get_used_mem_regions(used_mem_regions);

    used_mem_regions[used_mem_regions_size++]
        = (mem_region_t){.start = (uint8_t*)VGA_MEM_START, .end = (uint8_t*)VGA_MEM_END};
    used_mem_regions[used_mem_regions_size++] = get_multiboot_mem_region();
    used_mem_regions[used_mem_regions_size++] = get_kernel_mem_region();

    init_frame_allocator(system_memory, used_mem_regions, used_mem_regions_size);
    init_temp_allocator();


    // Remap kernel

    // Reuse the array and put inside elf sections
    // used_mem_regions_size = get_used_elf_sections(used_mem_regions);
    // used_mem_regions[used_mem_regions_size++]
    //     = (mem_region_t){.start = (uint8_t*)VGA_MEM_START, .end = (uint8_t*)VGA_MEM_END};
    // used_mem_regions[used_mem_regions_size++] = get_multiboot_mem_region();
    // remap_kernel(used_mem_regions, used_mem_regions_size);
    remap_kernel();
}
