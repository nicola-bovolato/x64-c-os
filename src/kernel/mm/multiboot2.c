#include "multiboot2.h"

#include "../lib/printf.h"

#include <stdbool.h>

typedef void     (*print_tag_t)(multiboot_tag_t*);
typedef uint8_t* (*get_ptr_t)  (multiboot_tag_t*);

static uint32_t* multiboot_info_address = NULL;

static inline void print_tag_memory                     (multiboot_tag_t* tag);
static inline void print_tag_mmap                       (multiboot_tag_t* tag);
static inline void print_tag_mmap_available             (multiboot_tag_t* tag);
static inline void print_tag_elf_section                (multiboot_tag_t* tag);
static inline void print_tag_elf_section_used           (multiboot_tag_t* tag);
static inline void print_filtered_tags                  (uint32_t tag_type, print_tag_t print_tag);

static inline uint8_t* get_meminfo_max_ptr              (multiboot_tag_t* tag);
static inline uint8_t* get_elf_section_kernel_min_ptr   (multiboot_tag_t* tag);
static inline uint8_t* get_elf_section_kernel_max_ptr   (multiboot_tag_t* tag);
static inline uint8_t* get_ptr_from_tag                 (uint32_t tag_type, get_ptr_t get_ptr);

void init_multiboot_info(uint32_t* address)
{
    if(((uint64_t) address) & 0xf == 0) panic("Multiboot address is not aligned");
    if(address == NULL)                 panic("Multiboot address is not initalized");

    multiboot_info_address = address;
}

uint32_t* get_multiboot_info_address()
{
    return multiboot_info_address;
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
    return (uint8_t*) multiboot_info_address;
}

uint8_t* get_multiboot_memory_multiboot_end() {
    // the first 4 bytes pointed by the multiboot info address represent the multiboot header length
    return (uint8_t*)multiboot_info_address + *(multiboot_info_address);
}

void print_multiboot_info_mmap(){
    printf("Memory map:\n");
    print_filtered_tags(MULTIBOOT_TAG_TYPE_MMAP, print_tag_mmap);
}

void print_multiboot_info_mmap_available(){
    printf("Memory map (available only):\n");
    print_filtered_tags(MULTIBOOT_TAG_TYPE_MMAP, print_tag_mmap_available);
}

void print_multiboot_info_elf_sections(){
    printf("Elf sections:\n");
    print_filtered_tags(MULTIBOOT_TAG_TYPE_ELF_SECTIONS, print_tag_elf_section);
}

void print_multiboot_info_elf_sections_used(){
    printf("Elf sections (used only):\n");
    print_filtered_tags(MULTIBOOT_TAG_TYPE_ELF_SECTIONS, print_tag_elf_section_used);
}

void print_multiboot_info_memory(){
    printf("Memory info:\n");
    print_filtered_tags(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO, print_tag_memory);
}

void print_multiboot_info_kernel_memory_region(){
    printf("Kernel memory region:\n");

    uint8_t* start = get_multiboot_memory_kernel_start();
    uint8_t* end   = get_multiboot_memory_kernel_end();

    printf("\tkernel_start: %p, kernel_end: %p, size: %uKiB\n", start, end, (size_t) (end - start) / 1024);
}

void print_multiboot_info_multiboot_memory_region(){
    printf("Multiboot memory region:\n");

    uint8_t* end_address = (uint8_t*) multiboot_info_address + *(multiboot_info_address);
    printf("\tmultiboot_start: %p, multiboot_end: %p, size: %uB\n", multiboot_info_address, end_address, (size_t)end_address - (size_t)multiboot_info_address);
}

static inline void print_tag_memory(multiboot_tag_t* tag){

    multiboot_tag_basic_meminfo_t *mem = (multiboot_tag_basic_meminfo_t *) tag;

    printf("\tmem_lower: %uKB, mem_higher: %uKB\n", mem->mem_lower, mem->mem_upper);
}

static inline void print_tag_mmap(multiboot_tag_t* tag){

    multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t *) ((multiboot_tag_mmap_t *)tag)->entries;

    while((uint8_t*)mmap < (uint8_t*)tag + tag->size) {
        printf("\taddress: %p, length: %p\n", mmap->addr, mmap->len);
        mmap++;
    }
}

static inline void print_tag_mmap_available(multiboot_tag_t* tag){

    multiboot_mmap_entry_t *mmap = (multiboot_mmap_entry_t *) ((multiboot_tag_mmap_t *)tag)->entries;

    while((uint8_t*)mmap < (uint8_t*)tag + tag->size) {
        if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
            printf("\taddress: %p, length: 0x%x\n", mmap->addr, mmap->len);
        mmap++;
    }
}

static inline void print_tag_elf_section(multiboot_tag_t* tag){

    multiboot_elf_section_t* section = (multiboot_elf_section_t *) ((multiboot_tag_elf_sections_t*)tag)->sections;
    int remaining = ((multiboot_tag_elf_sections_t*)tag)->num;

    while(remaining > 0){
        printf("\taddress: %p, size: %p, flags: %p\n", section->address, section->size, section->flags);
        remaining--;
        section++;
    }
}

static inline void print_tag_elf_section_used(multiboot_tag_t* tag){

    multiboot_elf_section_t* section = (multiboot_elf_section_t *) ((multiboot_tag_elf_sections_t*)tag)->sections;
    int remaining = ((multiboot_tag_elf_sections_t*)tag)->num;

    while(remaining > 0){
        if(section->type != MULTIBOOT_ELF_SECTION_UNUSED)
            printf("\taddress: %p, size: %p, flags: %p\n", section->address, section->size, section->flags);
        remaining--;
        section++;
    }
}

// To avoid repeating code this function iterates through tags, and executes a printing function on a specific type tag when reached
static inline void print_filtered_tags(uint32_t tag_type, print_tag_t print_tag){

    //skips the first 8 bytes of the header: (total_size and reserved)
    multiboot_tag_t *tag = (multiboot_tag_t *)(multiboot_info_address + 2);

    while(tag->type != MULTIBOOT_TAG_TYPE_END) {
        if(tag->type == tag_type) print_tag(tag);
        tag = (multiboot_tag_t *)((uint8_t *)tag + ((tag->size + 7) & ~7));
    }
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
    multiboot_tag_t *tag = (multiboot_tag_t *)(multiboot_info_address + 2);

    while(tag->type != MULTIBOOT_TAG_TYPE_END) {
        if(tag->type == tag_type) return get_ptr(tag);
        tag = (multiboot_tag_t *)((uint8_t *)tag + ((tag->size + 7) & ~7));
    }

    return (uint8_t*) -1;
}
