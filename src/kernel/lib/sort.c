#include "sort.h"
#include "../log.h"
#include "mem.h"
#include <stdint.h>

static inline void swap(void* a, void* b, size_t size);
static void        _qsort(void* arr, size_t size, long start, long end, compare_t compare);

void qsort(void* arr, size_t length, size_t size, compare_t compare) {
    _qsort(arr, size, 0, (long)length - 1, compare);
}

static inline void swap(void* a, void* b, size_t size) {
    uint8_t buf[size];
    memcpy(buf, a, size);
    memcpy(a, b, size);
    memcpy(b, buf, size);
}

static void _qsort(void* arr, size_t size, long start, long end, compare_t compare) {
    if (start >= end) return;

    long  pivot_idx = end;
    void* pivot     = ((uint8_t*)arr + pivot_idx * size);

    long compared_idx = start;

    for (long i = start; i <= end; i++) {
        void* curr = ((uint8_t*)arr + i * size);
        if (compare(curr, pivot) < 0) {
            void* compared = ((uint8_t*)arr + compared_idx * size);
            swap(curr, compared, size);
            compared_idx++;
        }
    }

    void* compared = ((uint8_t*)arr + compared_idx * size);
    swap(pivot, compared, size);

    _qsort(arr, size, start, compared_idx - 1, compare);
    _qsort(arr, size, compared_idx + 1, end, compare);
}
