#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_ENTRIES 512

typedef union {
#define PAGE_FLAG_PRESENT        0x1
#define PAGE_FLAG_WRITABLE       0x2
#define PAGE_FLAG_USER_ACCESIBLE 0x4
#define PAGE_FLAG_WRITE_THROUGH  0x8
#define PAGE_FLAG_NO_CACHE       0x10
#define PAGE_FLAG_ACCESSED       0x20
#define PAGE_FLAG_DIRTY          0x40
#define PAGE_FLAG_HUGE_PAGE      0x80
#define PAGE_FLAG_GLOBAL         0x100
#define PAGE_FLAG_MASK           0x1ff
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
} page_t;

typedef struct {
    page_t entries[PAGE_ENTRIES];
} page_table_t;

void init_paging();

#endif
