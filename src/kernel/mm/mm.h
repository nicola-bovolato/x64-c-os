#ifndef MM_H
#define MM_H

#include <stdint.h>

typedef struct {
    uint8_t* start;
    uint8_t* end;
} mem_region_t;


void init_mm(void* multiboot_header);

#endif
