#include "helpers.h"
#include "../../log.h"


inline size_t get_table4_index(page_t entry) { return ((size_t)entry.fields.address >> 27) & 0777; }

inline size_t get_table3_index(page_t entry) { return ((size_t)entry.fields.address >> 18) & 0777; }

inline size_t get_table2_index(page_t entry) { return ((size_t)entry.fields.address >> 9) & 0777; }

inline size_t get_table1_index(page_t entry) { return ((size_t)entry.fields.address >> 0) & 0777; }

inline void zero_table_entries(page_table_t* table) {
    for (int i = 0; i < PAGE_ENTRIES; i++) table->entries[i].bits = 0;
}

inline page_table_t* next_table(page_table_t* table, size_t index) {
    page_t* entry = &(table->entries[index]);
    if (!entry->fields.present) return (void*)-1;
    if (entry->fields.huge_page)
        PANIC("Huge pages not supported! (Table entry %d is a huge page)", entry->bits);

    return (page_table_t*)(((size_t)table << 9) | (index << 12));
}

inline page_table_t*
get_or_create_next_table(page_table_t* table, size_t index, allocate_frame_t allocate_frame) {

    // if the next table does not exist, allocate a frame for a new one
    if (next_table(table, index) == (void*)-1) {

        DEBUG("(get_or_create_next_table) Creating next table\n");
        table->entries[index].fields.address  = (size_t)allocate_frame() / PAGE_SIZE;
        table->entries[index].fields.present  = true;
        table->entries[index].fields.writable = true;
        zero_table_entries(next_table(table, index));
    }

    return next_table(table, index);
}
