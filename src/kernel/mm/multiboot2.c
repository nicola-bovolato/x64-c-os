#include "multiboot2.h"
#include "../log.h"
#include <stdbool.h>

static uint32_t* multiboot_info_ptr = (uint32_t*)-1;

static inline multiboot_tag_t*              get_tag(uint32_t tag_type);
static inline multiboot_tag_elf_sections_t* get_elf_sections();

void init_multiboot_info(uint32_t* address) {
    if ((((uint64_t)address) & 7) != 0)
        PANIC("Multiboot address is not aligned (%d)", address);
    if (address == (uint32_t*)-1) PANIC("Multiboot address is not initialized");

    multiboot_info_ptr = address;
}

size_t get_used_mem_regions_number() {
    multiboot_tag_mmap_t* memmap
        = (multiboot_tag_mmap_t*)get_tag(MULTIBOOT_TAG_TYPE_MMAP);
    return ((size_t)memmap->size - (sizeof memmap))
         / (size_t)memmap->entry_size;
}

void get_used_mem_regions(mem_region_t* used_regions) {
    multiboot_tag_mmap_t* memmap
        = (multiboot_tag_mmap_t*)get_tag(MULTIBOOT_TAG_TYPE_MMAP);

    for (size_t i = 0; i < get_used_mem_regions_number(); i++) {
        multiboot_mmap_entry_t* entry = &memmap->entries[i];
        used_regions[i].start         = (uint8_t*)entry->base_addr;
        used_regions[i].end = (uint8_t*)(entry->base_addr + entry->length);
    }
}

mem_region_t get_system_mem_region() {
    multiboot_tag_basic_meminfo_t* meminfo = (multiboot_tag_basic_meminfo_t*)
        get_tag(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);
    mem_region_t mem_region
        = {.start = 0x0, .end = (uint8_t*)(meminfo->mem_upper * 1024)};
    return mem_region;
}

mem_region_t get_multiboot_mem_region() {
    return (mem_region_t
    ){.start = (uint8_t*)multiboot_info_ptr,
      .end   = (uint8_t*)multiboot_info_ptr + *(multiboot_info_ptr)};
}

mem_region_t get_kernel_mem_region() {

    multiboot_tag_elf_sections_t* sections_tag = get_elf_sections();
    int                           remaining    = sections_tag->num;
    multiboot_elf_section_t*      section      = sections_tag->sections;

    uint8_t* min = (uint8_t*)-1;
    uint8_t* max = 0;

    while (remaining > 0) {
        if (section->type != MULTIBOOT_ELF_SECTION_UNUSED) {
            if (min > (uint8_t*)section->address)
                min = (uint8_t*)section->address;
            if (max < (uint8_t*)section->address)
                max = (uint8_t*)(section->address + section->size);
        }
        remaining--;
        section++;
    }

    return (mem_region_t){min, max};
}

static inline multiboot_tag_elf_sections_t* get_elf_sections() {
    return (multiboot_tag_elf_sections_t*)get_tag(
        MULTIBOOT_TAG_TYPE_ELF_SECTIONS
    );
}

// To avoid repeating code this function iterates through tags, and returns the
// first tag of the specified type
static inline multiboot_tag_t* get_tag(uint32_t tag_type) {

    // skips the first 8 bytes of the header: (total_size and reserved)
    multiboot_tag_t* tag = (multiboot_tag_t*)(multiboot_info_ptr + 2);

    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        if (tag->type == tag_type) return tag;
        tag = (multiboot_tag_t*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    }

    PANIC("Multiboot tag (%d) not found!", tag_type);
    return (void*)-1;
}
