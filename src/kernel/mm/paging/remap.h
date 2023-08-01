#ifndef PAGING_REMAP_H
#define PAGING_REMAP_H


#include "../memregion.h"
#include "page.h"
#include <stddef.h>

const page_table_t* remap_kernel(const mem_region_t to_map[], const size_t to_map_size);

#endif
