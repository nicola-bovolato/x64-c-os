#ifndef MEMREGION_H
#define MEMREGION_H

#include <stdbool.h>
#include <stdint.h>


typedef struct {
    uint8_t* start;
    uint8_t* end;
    bool     readable;
    bool     writable;
    bool     executable;
} mem_region_t;

#endif
