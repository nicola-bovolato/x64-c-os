#include "mm.h"
#include "frame.h"
#include "multiboot2.h"
#include "paging.h"


void init_mm(void* multiboot_header) {
    init_multiboot_info(multiboot_header);
    init_frame_allocator();
    init_paging();
}
