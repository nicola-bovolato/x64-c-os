#include "mm.h"
#include "../cpu/cpu.h"
#include "../drivers/tty.h"
#include "../lib/sort.h"
#include "../log.h"
#include "frameallocator/basic.h"
#include "frameallocator/temp.h"
#include "multiboot2.h"
#include "paging/paging.h"
#include "paging/remap.h"


#define MAX_USED_REGIONS 20

static inline int compare_mem_regions(const void* a, const void* b);


void init_mm(void* multiboot_header) {
    init_multiboot_info(multiboot_header);

    mem_region_t vga_mem_region = {
        .start    = (uint8_t*)VGA_MEM_START,
        .end      = (uint8_t*)VGA_MEM_END,
        .readable = true,
        .writable = true,
    };
    mem_region_t multiboot_mem_region = get_multiboot_mem_region();


    // Initialize frame allocators
    mem_region_t system_memory = get_system_mem_region();

    mem_region_t used_mem_regions[MAX_USED_REGIONS];
    size_t       used_mem_regions_size = get_used_mmap_regions(used_mem_regions);

    used_mem_regions[used_mem_regions_size++] = vga_mem_region;
    used_mem_regions[used_mem_regions_size++] = multiboot_mem_region;
    used_mem_regions[used_mem_regions_size++] = get_kernel_mem_region();
    qsort(used_mem_regions, used_mem_regions_size, sizeof(mem_region_t), compare_mem_regions);

    init_frame_allocator(system_memory, used_mem_regions, used_mem_regions_size);
    init_temp_allocator();


    // Remap kernel

    // Reuse the previously declared array and put inside of it the kernel's elf sections
    used_mem_regions_size                     = get_allocated_elf_sections(used_mem_regions);
    used_mem_regions[used_mem_regions_size++] = vga_mem_region;
    used_mem_regions[used_mem_regions_size++] = multiboot_mem_region;
    qsort(used_mem_regions, used_mem_regions_size, sizeof(mem_region_t), compare_mem_regions);

    // Avoid triggering cpu exceptions when marking a page as not executable
    enable_nxe_bit();
    void* old_table_addr = (void*)read_cr3();
    void* new_table_addr = remap_kernel(used_mem_regions, used_mem_regions_size);
    // Remove write access to not writable pages
    enable_write_protect_bit();

    write_cr3((uint64_t)new_table_addr);

    // Transform the old p4 table in a guard page (by unmapping it)
    // The kernel stack is right under the old page tables, so:
    // - a stack overflow triggers a page fault
    // - we increased the kernel stack size by:
    //    - 1   table3 (4KiB)
    //    - 1   table2 (4KiB)
    //    - 512 table1 (2MiB)
    unmap_page(
        (page_t){.fields.address = (size_t)old_table_addr / PAGE_SIZE}, deallocate_frame, true
    );

    LOG("MMU initialized!\n");
}

static inline int compare_mem_regions(const void* a, const void* b) {
    if (((mem_region_t*)a)->start > ((mem_region_t*)b)->start) return 1;
    if (((mem_region_t*)a)->start < ((mem_region_t*)b)->start) return -1;
    return 0;
}
