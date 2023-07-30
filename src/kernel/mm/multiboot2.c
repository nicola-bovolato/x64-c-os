#include "multiboot2.h"
#include "../log.h"
#include <stdbool.h>

static uint8_t* multiboot_info_ptr = (uint8_t*)-1;

static inline multiboot_tag_t* get_tag(uint32_t tag_type);

static inline size_t get_mem_regions_number(multiboot_tag_mmap_t* memmap);

// required to use all of the folowing functions
void init_multiboot_info(void* address) {
    if ((((uint64_t)address) & 7) != 0) PANIC("Multiboot address is not aligned (%d)", address);
    if (address == (void*)-1) PANIC("Multiboot address is not initialized");

    multiboot_info_ptr = (uint8_t*)address;
}

// Copies all used memory regions into the first parameter
// Ensure that when using it the array is big enough
size_t get_used_mem_regions(mem_region_t* used_regions) {
    multiboot_tag_mmap_t* memmap = (multiboot_tag_mmap_t*)get_tag(MULTIBOOT_TAG_TYPE_MMAP);
    size_t                used_regions_number = 0;

    for (size_t i = 0; i < get_mem_regions_number(memmap); i++) {
        multiboot_mmap_entry_t* entry = &memmap->entries[i];

        if (entry->type != MULTIBOOT_MEMORY_AVAILABLE) {
            used_regions[used_regions_number].start = (uint8_t*)entry->base_addr;
            used_regions[used_regions_number].end
                = (uint8_t*)(entry->base_addr + entry->length - 1);
            used_regions_number++;
        }
    }

    return used_regions_number;
}

// Returns the available system memory
mem_region_t get_system_mem_region() {
    multiboot_tag_mmap_t* memmap = (multiboot_tag_mmap_t*)get_tag(MULTIBOOT_TAG_TYPE_MMAP);

    // iterate through memory map
    uint8_t* highest_address = 0;

    DEBUG("Multiboot memory map:\n");
    for (size_t i = 0; i < get_mem_regions_number(memmap); i++) {
        multiboot_mmap_entry_t* entry = &memmap->entries[i];
        DEBUG(
            "\t%d: start = %p, length = %p, type = %p\n",
            i,
            entry->base_addr,
            entry->length,
            entry->type
        );

        uint8_t* end_region_addr = (uint8_t*)(entry->base_addr + entry->length - 1);
        if (end_region_addr > highest_address) highest_address = end_region_addr;
    }

    DEBUG("Total system memory: start = 0x0, end = %p\n", highest_address);
    return (mem_region_t){.start = 0x0, .end = highest_address};
}

// Returns the memory region used by the multiboot struct
mem_region_t get_multiboot_mem_region() {
    mem_region_t mem_region
        = {.start = (uint8_t*)multiboot_info_ptr,
           .end   = (uint8_t*)multiboot_info_ptr + (*multiboot_info_ptr) - 1};
    DEBUG(
        "Multiboot struct: start = %p, end = %p, size = %p\n",
        mem_region.start,
        mem_region.end,
        mem_region.end - mem_region.start + 1
    );
    return mem_region;
}

// Returns the memory region used by the kernel
mem_region_t get_kernel_mem_region() {

    multiboot_tag_elf_sections_t* sections_tag
        = (multiboot_tag_elf_sections_t*)get_tag(MULTIBOOT_TAG_TYPE_ELF_SECTIONS);

    multiboot_elf_section_t* section   = sections_tag->sections;
    size_t                   remaining = sections_tag->num;

    uint8_t* min = (uint8_t*)-1;
    uint8_t* max = 0;

    DEBUG("Elf sections:\n");
    while (remaining > 0) {
        if (section->type != MULTIBOOT_ELF_SECTION_UNUSED) {
            if (min > (uint8_t*)section->address) min = (uint8_t*)section->address;
            if (max < (uint8_t*)section->address)
                max = (uint8_t*)(section->address + section->size - 1);
        }
        DEBUG(
            "\t address = %p, size = %p, flags = %p\n",
            section->address,
            section->size,
            section->flags
        );

        section = (multiboot_elf_section_t*)(((uint8_t*)section) + sections_tag->entsize);
        remaining--;
    }

    mem_region_t mem_region = {.start = min, .end = max};

    DEBUG(
        "Kernel: start = %p, end = %p, size = %p\n",
        mem_region.start,
        mem_region.end,
        mem_region.end - mem_region.start + 1
    );

    return mem_region;
}

// Returns the number of memory regions
static inline size_t get_mem_regions_number(multiboot_tag_mmap_t* memmap) {
    return ((size_t)memmap->size - (sizeof memmap)) / (size_t)memmap->entry_size;
}

// To avoid repeating code this function iterates through tags
// and returns the first tag of the specified type
static inline multiboot_tag_t* get_tag(uint32_t tag_type) {

    // skips the first 8 bytes of the header: (total_size and reserved)
    multiboot_tag_t* tag = (multiboot_tag_t*)((uint8_t*)multiboot_info_ptr + 8);

    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        if (tag->type == tag_type) return tag;
        tag = (multiboot_tag_t*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    }

    PANIC("Multiboot tag (%d) not found!", tag_type);
    return (void*)-1;
}
