#include "multiboot2.h"

#include <stdbool.h>

#include "../panic.h"

typedef uint8_t* (*get_ptr_t)  (multiboot_tag_t*);

static uint32_t* multiboot_info_ptr = (uint32_t*)-1;

static inline uint8_t* get_meminfo_max_ptr              (multiboot_tag_t* tag);
static inline uint8_t* get_elf_section_kernel_min_ptr   (multiboot_tag_t* tag);
static inline uint8_t* get_elf_section_kernel_max_ptr   (multiboot_tag_t* tag);
static inline uint8_t* get_ptr_from_tag                 (uint32_t tag_type, get_ptr_t get_ptr);

void init_multiboot_info(uint32_t* address) {
    if((((uint64_t) address) & 7) != 0) panic("Multiboot address is not aligned");
    if(address == (uint32_t*)-1)        panic("Multiboot address is not initalized");

    multiboot_info_ptr = address;
}

uint32_t* get_multiboot_info_ptr() {
    return multiboot_info_ptr;
}

uint8_t* get_multiboot_memory_start() {
    return (uint8_t*) 0;
}

uint8_t* get_multiboot_memory_end() {
    return get_ptr_from_tag(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO, get_meminfo_max_ptr);
}

uint8_t* get_multiboot_memory_kernel_start() {
    return get_ptr_from_tag(MULTIBOOT_TAG_TYPE_ELF_SECTIONS, get_elf_section_kernel_min_ptr);
}

uint8_t* get_multiboot_memory_kernel_end() {
    return get_ptr_from_tag(MULTIBOOT_TAG_TYPE_ELF_SECTIONS, get_elf_section_kernel_max_ptr);
}

uint8_t* get_multiboot_memory_multiboot_start() {
    return (uint8_t*) multiboot_info_ptr;
}

uint8_t* get_multiboot_memory_multiboot_end() {
    // the first 4 bytes pointed by the multiboot info address represent the multiboot header length
    return (uint8_t*)multiboot_info_ptr + *(multiboot_info_ptr);
}

static inline uint8_t* get_meminfo_max_ptr(multiboot_tag_t* tag){

    multiboot_tag_basic_meminfo_t *mem = (multiboot_tag_basic_meminfo_t *) tag;

    return (uint8_t*) ((size_t)mem->mem_upper * 1024);
}

static inline uint8_t* get_elf_section_kernel_min_ptr(multiboot_tag_t* tag){

    multiboot_elf_section_t* section = (multiboot_elf_section_t *) ((multiboot_tag_elf_sections_t*)tag)->sections;
    int remaining = ((multiboot_tag_elf_sections_t*)tag)->num;

    uint8_t* min = (uint8_t*) -1;

    while(remaining > 0){
        if(section->type != MULTIBOOT_ELF_SECTION_UNUSED){
            if(min > (uint8_t*)section->address) min = (uint8_t*) section->address;
        }
        remaining--;
        section++;
    }

    return min;
}

static inline uint8_t* get_elf_section_kernel_max_ptr(multiboot_tag_t* tag){

    multiboot_elf_section_t* section = (multiboot_elf_section_t *) ((multiboot_tag_elf_sections_t*)tag)->sections;
    int remaining = ((multiboot_tag_elf_sections_t*)tag)->num;

    uint8_t* max = (uint8_t*) 0;

    while(remaining > 0){
        if(section->type != MULTIBOOT_ELF_SECTION_UNUSED){
            if(max < (uint8_t*)section->address) max = (uint8_t*) (section->address + section->size);
        }
        remaining--;
        section++;
    }

    return max;
}

// To avoid repeating code this function iterates through tags, and returns the get_ptr_t function value on a specific tag
static inline uint8_t* get_ptr_from_tag(uint32_t tag_type, get_ptr_t get_ptr){

    //skips the first 8 bytes of the header: (total_size and reserved)
    multiboot_tag_t *tag = (multiboot_tag_t *)(multiboot_info_ptr + 2);

    while(tag->type != MULTIBOOT_TAG_TYPE_END) {
        if(tag->type == tag_type) return get_ptr(tag);
        tag = (multiboot_tag_t *)((uint8_t *)tag + ((tag->size + 7) & ~7));
    }

    return (uint8_t*)-1;
}
