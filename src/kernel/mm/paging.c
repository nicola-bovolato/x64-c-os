#include "paging.h"
#include "../debug.h"
#include "../panic.h"
#include "frame.h"

#define LOWER_HALF_TOP_ADDRESS     (0x0000800000000000 - 1)
#define HIGHER_HALF_BOTTOM_ADDRESS 0xffff800000000000

// The last entry of the page map level4 table is mapped to itself in boot.asm
page_table_t* const table4_ptr = (page_table_t*)0xfffffffffffff000;

static inline size_t get_table4_index(page_entry_t entry);
static inline size_t get_table3_index(page_entry_t entry);
static inline size_t get_table2_index(page_entry_t entry);
static inline size_t get_table1_index(page_entry_t entry);

static inline void zero_table_entries(page_table_t* table);
static inline void flush_tlb_page(page_entry_t page);

static inline page_table_t* next_table(page_table_t* table, size_t index);
static inline page_table_t*
get_or_create_next_table(page_table_t* table, size_t index);

void* get_physical_address(void* virtual) {

    if ((size_t) virtual > LOWER_HALF_TOP_ADDRESS
        && (size_t) virtual < HIGHER_HALF_BOTTOM_ADDRESS) {
        // Invalid address
        panic("Virtual address not in allowed memory");
    }

    size_t offset = (size_t) virtual % FRAME_SIZE;

    page_entry_t page   = {.bits = 0};
    page.fields.address = (size_t) virtual / FRAME_SIZE;

    page_table_t* table3_ptr = next_table(table4_ptr, get_table4_index(page));
    if (table3_ptr == (void*)-1) {
        debug("(get_physical_address) Page table 3 is empty");
        return (void*)-1;
    }

    page_table_t* table2_ptr = next_table(table3_ptr, get_table3_index(page));
    if (table2_ptr == (void*)-1) {
        debug("(get_physical_address) Page table 2 is empty");
        return (void*)-1;
    }

    page_table_t* table1_ptr = next_table(table2_ptr, get_table2_index(page));
    if (table1_ptr == (void*)-1) {
        debug("(get_physical_address) Page table 1 is empty");
        return (void*)-1;
    }

    page_entry_t table1_entry = table1_ptr->entries[get_table1_index(page)];
    if (!table1_entry.fields.present) {
        debug("(get_physical_address) Page table 1 entry is empty");
        return (void*)-1;
    }

    return (void*)(table1_entry.fields.address * FRAME_SIZE + offset);
}

void map_page_to_frame(page_entry_t page, uint8_t* frame_ptr) {

    page_table_t* table3_ptr
        = get_or_create_next_table(table4_ptr, get_table4_index(page));
    page_table_t* table2_ptr
        = get_or_create_next_table(table3_ptr, get_table3_index(page));
    page_table_t* table1_ptr
        = get_or_create_next_table(table2_ptr, get_table2_index(page));

    size_t table1_index = get_table1_index(page);

    if (table1_ptr->entries[table1_index].bits != 0)
        panic("Page is already in use");
    table1_ptr->entries[table1_index].fields.present = true;
    table1_ptr->entries[table1_index].fields.address
        = (size_t)frame_ptr / FRAME_SIZE;
}

void unmap_page(page_entry_t page) {
    page_table_t* table3_ptr = next_table(table4_ptr, get_table4_index(page));
    if (table3_ptr == (void*)-1) {
        debug("(unmap_page) Page table 3 is empty");
        return;
    }

    page_table_t* table2_ptr = next_table(table3_ptr, get_table3_index(page));
    if (table2_ptr == (void*)-1) {
        debug("(unmap_page) Page table 2 is empty");
        return;
    }

    page_table_t* table1_ptr = next_table(table2_ptr, get_table2_index(page));
    if (table1_ptr == (void*)-1) {
        debug("(unmap_page) Page table 1 is empty");
        return;
    }

    int table1_index = get_table1_index(page);

    void* frame_ptr
        = (void*)((size_t)table1_ptr->entries[table1_index].fields.address
                  * FRAME_SIZE);

    table1_ptr->entries[table1_index].bits = 0;
    flush_tlb_page(page);

    deallocate_frame(frame_ptr);
}

static inline size_t get_table4_index(page_entry_t entry) {
    return (entry.fields.address >> 27) & 0777;
}

static inline size_t get_table3_index(page_entry_t entry) {
    return (entry.fields.address >> 18) & 0777;
}

static inline size_t get_table2_index(page_entry_t entry) {
    return (entry.fields.address >> 9) & 0777;
}

static inline size_t get_table1_index(page_entry_t entry) {
    return (entry.fields.address >> 0) & 0777;
}

static inline void zero_table_entries(page_table_t* table) {
    for (int i = 0; i < PAGE_ENTRIES; i++) table->entries[i].bits = 0;
}

static inline void flush_tlb_page(page_entry_t page) {
    __asm__ volatile("invlpg (%0)" ::"r"(page.bits) : "memory");
}

static inline page_table_t* next_table(page_table_t* table, size_t index) {
    page_entry_t entry = table->entries[index];
    if (!entry.fields.present) {
        return (void*)-1;
    }
    if (entry.fields.huge_page) panic("Huge pages not supported!");
    return (page_table_t*)(((size_t)table << 9) | (index << 12));
}

static inline page_table_t*
get_or_create_next_table(page_table_t* table, size_t index) {

    // if the next table does not exist, allocate a frame for a new one
    if (next_table(table, index) == (void*)-1) {
        if (table->entries[index].fields.huge_page)
            panic("Huge pages not supported");

        table->entries[index].fields.address
            = (size_t)allocate_frame() / FRAME_SIZE;
        table->entries[index].fields.present  = true;
        table->entries[index].fields.writable = true;
        zero_table_entries(next_table(table, index));
    }

    return next_table(table, index);
}
