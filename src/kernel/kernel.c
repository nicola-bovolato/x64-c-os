#include "./drivers/tty.h"
#include "./lib/printf.h"
#include "./mm/mm.h"
#include "log.h"


#if defined(__linux__)
#error "Not using a cross compiler"
#endif

#if !defined(__x86_64__)
#error "The kernel needs to be compiled with an x86_64-elf compiler"
#endif


void kernel_main(void* multiboot_header) {
    LOG("Kernel booted!\n");

    init_mm(multiboot_header);
}
