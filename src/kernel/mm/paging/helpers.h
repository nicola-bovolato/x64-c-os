#ifndef PAGING_HELPERS_H
#define PAGING_HELPERS_H

#include "../frame/allocator.h"
#include "page.h"
#include <stddef.h>

size_t get_table4_index(page_t entry);
size_t get_table3_index(page_t entry);
size_t get_table2_index(page_t entry);
size_t get_table1_index(page_t entry);

void zero_table_entries(page_table_t* table);

page_table_t* next_table(const page_table_t* table, size_t index);
page_table_t*
get_or_create_next_table(page_table_t* table, size_t index, allocate_frame_t allocate_frame);

#endif
