#ifndef PAGING_H
#define PAGING_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define PAGE_ENTRIES 512

typedef union {
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

void* get_physical_address(void* virtual_address);
void  map_page_to_frame(page_entry_t page, uint8_t* frame_ptr);
void  unmap_page(page_entry_t page);

#endif
