#include "mem.h"


void* memset(void* destination, uint8_t value, size_t length) {
    uint8_t* dest = (uint8_t*)destination;

    while (length-- > 0) *dest++ = value;

    return destination;
}

void* memcpy(void* destination, const void* source, size_t length) {
    uint8_t*       dest = (uint8_t*)destination;
    const uint8_t* src  = (uint8_t*)source;

    while (length-- > 0) *dest++ = *src++;

    return destination;
}

void* memmove(void* destination, const void* source, size_t length) {
    uint8_t*       dest = (uint8_t*)destination;
    const uint8_t* src  = (uint8_t*)source;

    // memmove checks if source and destination buffers overlap
    // if the buffers don't overlap the direction it copies from doesn't matter

    // if the source buffer starts before the destination's start copying from the end
    if (src < dest) {
        for (int i = length - 1; i >= 0; i--) dest[i] = src[i];
    }
    else return memcpy(destination, source, length);


    return destination;
}
