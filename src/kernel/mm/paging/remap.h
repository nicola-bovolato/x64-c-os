#ifndef PAGING_REMAP_H
#define PAGING_REMAP_H


#include "../memregion.h"
#include <stddef.h>

void* remap_kernel(mem_region_t to_map[], size_t to_map_size);

#endif
