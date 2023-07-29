#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_ENTRIES 512

typedef union {
#define PAGE_FLAG_PRESENT        0b000000001
#define PAGE_FLAG_WRITABLE       0b000000010
#define PAGE_FLAG_USER_ACCESIBLE 0b000000100
#define PAGE_FLAG_WRITE_THROUGH  0b000001000
#define PAGE_FLAG_NO_CACHE       0b000010000
#define PAGE_FLAG_ACCESSED       0b000100000
#define PAGE_FLAG_DIRTY          0b001000000
#define PAGE_FLAG_HUGE_PAGE      0b010000000
#define PAGE_FLAG_GLOBAL         0b100000000
#define PAGE_FLAG_MASK           0b111111111
    struct {
        bool present         : 1;
        bool writable        : 1;
        bool user_accessible : 1;
        bool write_trough    : 1;
        bool no_cache        : 1;
        bool accessed        : 1;
        bool dirty           : 1;
        bool huge_page       : 1;
        bool global          : 1;
        int                  : 3;
        uint64_t address     : 40;
        int                  : 11;
        bool no_execute      : 1;
    } fields;
    uint64_t bits;
} page_entry_t;

typedef struct {
    page_entry_t entries[PAGE_ENTRIES];
} page_table_t;

void init_paging(uint32_t* multiboot_header);

// void* get_physical_address(void* virtual_address);
// void  map_page_to_frame(page_entry_t page, uint8_t* frame_ptr);
// void  unmap_page(page_entry_t page);

#endif
