#include "tty.h"
#include "printf.h"
#include "multiboot2.h"

#include <stddef.h>

#if defined(__linux__)
#error "Not using a cross compiler"
#endif

#if !defined(__x86_64__)
#error "The kernel needs to be compiled with an x86_64-elf compiler"
#endif


void kernel_main(uint32_t* multiboot_header){
    clear_screen();
    set_color(VGA_WHITE, VGA_GREEN);
    printf("OK!\n");

    set_color(VGA_WHITE, VGA_BLACK);

    set_multiboot_info_address(multiboot_header);
    print_multiboot_info_kernel_memory_region();
    print_multiboot_info_multiboot_memory_region();
    print_multiboot_info_mmap_available();
    print_multiboot_info_elf_sections_used();
}
