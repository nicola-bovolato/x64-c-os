#include "multiboot2.h"
#include "../panic.h"
#include <stdbool.h>

typedef void* (*get_ptr_t)(multiboot_tag_t*);

static uint32_t* multiboot_info_ptr = (uint32_t*)-1;

static inline void* get_meminfo_end_ptr(multiboot_tag_t* tag);
static inline void* get_elf_sections_ptr(multiboot_tag_t* tag);
static inline void* get_elf_sections_number_as_ptr(multiboot_tag_t* tag);
static inline void* get_ptr_from_tag(uint32_t tag_type, get_ptr_t get_ptr);

void init_multiboot_info(uint32_t* address) {
    if ((((uint64_t)address) & 7) != 0)
        panic("Multiboot address is not aligned");
    if (address == (uint32_t*)-1) panic("Multiboot address is not initialized");

    multiboot_info_ptr = address;
}

uint32_t* get_multiboot_info_ptr() { return multiboot_info_ptr; }

mem_region_t get_mem_region() {
    uint8_t* mem_end = get_ptr_from_tag(
        MULTIBOOT_TAG_TYPE_BASIC_MEMINFO, get_meminfo_end_ptr
    );
    return (mem_region_t){0, mem_end};
}
mem_region_t get_multiboot_mem_region() {
    return (mem_region_t
    ){(uint8_t*)multiboot_info_ptr,
      (uint8_t*)multiboot_info_ptr + *(multiboot_info_ptr)};
}

size_t get_elf_sections_number() {
    return (size_t)get_ptr_from_tag(
        MULTIBOOT_TAG_TYPE_ELF_SECTIONS, get_elf_sections_number_as_ptr
    );
}

multiboot_elf_section_t* get_elf_sections() {
    return (multiboot_elf_section_t*)get_ptr_from_tag(
        MULTIBOOT_TAG_TYPE_ELF_SECTIONS, get_elf_sections_ptr
    );
}

mem_region_t get_kernel_mem_region() {
    multiboot_elf_section_t* section   = get_elf_sections();
    int                      remaining = get_elf_sections_number();

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

static inline void* get_elf_sections_number_as_ptr(multiboot_tag_t* tag) {
    return (void*)(size_t)((multiboot_tag_elf_sections_t*)tag)->num;
}

static inline void* get_elf_sections_ptr(multiboot_tag_t* tag) {
    return (void*)((multiboot_tag_elf_sections_t*)tag)->sections;
}

static inline void* get_meminfo_end_ptr(multiboot_tag_t* tag) {

    multiboot_tag_basic_meminfo_t* mem = (multiboot_tag_basic_meminfo_t*)tag;

    return (void*)((size_t)mem->mem_upper * 1024);
}

// To avoid repeating code this function iterates through tags, and returns the
// get_ptr_t function value on a specific tag
static inline void* get_ptr_from_tag(uint32_t tag_type, get_ptr_t get_ptr) {

    // skips the first 8 bytes of the header: (total_size and reserved)
    multiboot_tag_t* tag = (multiboot_tag_t*)(multiboot_info_ptr + 2);

    while (tag->type != MULTIBOOT_TAG_TYPE_END) {
        if (tag->type == tag_type) return get_ptr(tag);
        tag = (multiboot_tag_t*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    }

    panic("Multiboot tag not found!");
}
