#include "./drivers/tty.h"
#include "./lib/printf.h"
#include "./mm/frame.h"
#include "./mm/multiboot2.h"
#include "./mm/paging.h"
#include "panic.h"

#if defined(__linux__)
#error "Not using a cross compiler"
#endif

#if !defined(__x86_64__)
#error "The kernel needs to be compiled with an x86_64-elf compiler"
#endif

void kernel_main(uint32_t* multiboot_header) {

    clear_screen();
    set_color(VGA_WHITE, VGA_GREEN);
    printf("OK!\n");

    set_color(VGA_WHITE, VGA_BLACK);


    init_multiboot_info(multiboot_header);

    init_frame_allocator(get_mem_region());
    frame_allocator_add_used_memory_region(get_kernel_mem_region());
    frame_allocator_add_used_memory_region(get_multiboot_mem_region());


    printf("Page allocation test:\n");

    uint64_t     address = 42 * 512 * 512 * 4096L; // 42th P3 entry
    page_entry_t page    = {.bits = 0};
    page.fields.address  = address / FRAME_SIZE;


    void* frame = allocate_frame();

    printf("Frame addr: %p\n", frame);
    printf(
        "Initial pointed page addr (should be (*void) -1): %p\n",
        get_physical_address((void*)address)
    );

    map_page_to_frame(page, frame);
    printf(
        "After mapping: %p == %p (frame) \n",
        get_physical_address((void*)address),
        frame
    );

    printf("Contents of the page (random): %p \n", *((size_t*)page.bits));

    unmap_page(page);
    printf("After unmapping: %p\n", get_physical_address((void*)address));

    // Reading after unmapping will cause a page fault
    // printf("Contents of the page: %p \n", *((size_t*)page.bits));
}
