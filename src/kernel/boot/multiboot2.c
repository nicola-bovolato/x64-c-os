#include "multiboot2.h"

#include "../lib/printf.h"

#include <stdbool.h>

static uint32_t* multiboot_info_address = (uint32_t*) -1;

static inline bool is_aligned();
static inline bool is_initialized();
static inline bool valid_address();

static inline void print_tag_memory                     (struct multiboot_tag* tag);
static inline void print_tag_mmap                       (struct multiboot_tag* tag);
static inline void print_tag_mmap_available             (struct multiboot_tag* tag);
static inline void print_tag_elf_section                (struct multiboot_tag* tag);
static inline void print_tag_elf_section_used           (struct multiboot_tag* tag);
static inline void print_tag_elf_section_kernel_min_max (struct multiboot_tag* tag);
static inline void print_filtered_tags                  (uint32_t tag_type, void (*print_tag)(struct multiboot_tag*));

void set_multiboot_info_address(uint32_t* address)
{
    multiboot_info_address = address;
}

void print_multiboot_info_mmap(){
    if(valid_address()) {
        printf("Memory map:\n");
        print_filtered_tags(MULTIBOOT_TAG_TYPE_MMAP, print_tag_mmap);
    }
}

void print_multiboot_info_mmap_available(){
    if(valid_address()) {
        printf("Memory map (available only):\n");
        print_filtered_tags(MULTIBOOT_TAG_TYPE_MMAP, print_tag_mmap_available);
    }
}

void print_multiboot_info_elf_sections(){
    if(valid_address()) {
        printf("Elf sections:\n");
        print_filtered_tags(MULTIBOOT_TAG_TYPE_ELF_SECTIONS, print_tag_elf_section);
    }
}

void print_multiboot_info_elf_sections_used(){
    if(valid_address()) {
        printf("Elf sections (used only):\n");
        print_filtered_tags(MULTIBOOT_TAG_TYPE_ELF_SECTIONS, print_tag_elf_section_used);
    }
}

void print_multiboot_info_memory(){
    if(valid_address()) {
        printf("Memory info:\n");
        print_filtered_tags(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO, print_tag_memory);
    }
}

void print_multiboot_info_kernel_memory_region(){
    if(valid_address()) {
        printf("Kernel memory region:\n");
        print_filtered_tags(MULTIBOOT_TAG_TYPE_ELF_SECTIONS, print_tag_elf_section_kernel_min_max);
    }
}

void print_multiboot_info_multiboot_memory_region(){
    if(valid_address()) {
        printf("Multiboot memory region:\n");

        uint8_t* end_address = (uint8_t*) multiboot_info_address + *(multiboot_info_address);
        printf("\tmultiboot_start: %p, multiboot_end: %p, size: %uB\n", multiboot_info_address, end_address, (size_t)end_address - (size_t)multiboot_info_address);
    }
}

static inline bool is_aligned(){
    return (((uint64_t) multiboot_info_address) & 7) == 0;
}

static inline bool is_initialized(){
    return multiboot_info_address != (uint32_t*) -1;
}

static inline bool valid_address(){
    bool valid = is_initialized() && is_aligned();
    if(!valid) printf("Invalid multiboot address\n");
    return valid;
}

static inline void print_tag_memory(struct multiboot_tag* tag){

    struct multiboot_tag_basic_meminfo *mem = ((struct multiboot_tag_basic_meminfo *) tag);

    printf("\tmem_lower: %uKB, mem_higher: %uKB\n", mem->mem_lower, mem->mem_upper);
}

static inline void print_tag_mmap(struct multiboot_tag* tag){

    struct multiboot_mmap_entry *mmap = (struct multiboot_mmap_entry *) ((struct multiboot_tag_mmap *)tag)->entries;

    while((uint8_t*)mmap < (uint8_t*)tag + tag->size) {
        printf("\taddress: %p, length: %p\n", mmap->addr, mmap->len);
        mmap++;
    }
}

static inline void print_tag_mmap_available(struct multiboot_tag* tag){

    struct multiboot_mmap_entry *mmap = (struct multiboot_mmap_entry *) ((struct multiboot_tag_mmap *)tag)->entries;

    while((uint8_t*)mmap < (uint8_t*)tag + tag->size) {
        if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
            printf("\taddress: %p, length: 0x%x\n", mmap->addr, mmap->len);
        mmap++;
    }
}

static inline void print_tag_elf_section(struct multiboot_tag* tag){

    struct multiboot_elf_section* section = (struct multiboot_elf_section *) ((struct multiboot_tag_elf_sections*)tag)->sections;
    int remaining = ((struct multiboot_tag_elf_sections*)tag)->num;

    while(remaining > 0){
        printf("\taddress: %p, size: %p, flags: %p\n", section->address, section->size, section->flags);
        remaining--;
        section++;
    }
}

static inline void print_tag_elf_section_used(struct multiboot_tag* tag){

    struct multiboot_elf_section* section = (struct multiboot_elf_section *) ((struct multiboot_tag_elf_sections*)tag)->sections;
    int remaining = ((struct multiboot_tag_elf_sections*)tag)->num;

    while(remaining > 0){
        if(section->type != MULTIBOOT_ELF_SECTION_UNUSED)
            printf("\taddress: %p, size: %p, flags: %p\n", section->address, section->size, section->flags);
        remaining--;
        section++;
    }
}

static inline void print_tag_elf_section_kernel_min_max(struct multiboot_tag* tag){

    struct multiboot_elf_section* section = (struct multiboot_elf_section *) ((struct multiboot_tag_elf_sections*)tag)->sections;
    int remaining = ((struct multiboot_tag_elf_sections*)tag)->num;

    uint8_t* min = (uint8_t*) -1;
    uint8_t* max = 0;

    while(remaining > 0){
        if(section->type != MULTIBOOT_ELF_SECTION_UNUSED){
            if(min > (uint8_t*)section->address)                    min = (uint8_t*) section->address;
            if(max < (uint8_t*)section->address + section->size)    max = (uint8_t*)(section->address + section->size);
        }
        remaining--;
        section++;
    }

    printf("\tkernel_start: %p, kernel_end: %p, size: %uKB\n", min, max, (size_t) (max - min) / 1024);
}

static inline void print_filtered_tags(uint32_t tag_type, void (*print_tag)(struct multiboot_tag*)){

    //skips the first 8 bytes of the header: (total_size and reserved)
    struct multiboot_tag *tag = (struct multiboot_tag *)(multiboot_info_address + 2);

    while(tag->type != MULTIBOOT_TAG_TYPE_END) {
        if(tag->type == tag_type) print_tag(tag);
        tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
    }
}
