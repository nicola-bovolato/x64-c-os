#include "mm.h"
#include "frameallocator/basic.h"
#include "frameallocator/temp.h"
#include "multiboot2.h"
#include "paging/remap.h"


void init_mm(void* multiboot_header) {
    init_multiboot_info(multiboot_header);

    // TODO: memory areas here, no hidden deps
    init_frame_allocator();
    init_temp_allocator();

    remap_kernel();
}
