#ifndef SORT_H
#define SORT_H

#include <stddef.h>


typedef int (*compare_t)(const void* a, const void* b);


void qsort(void* arr, size_t length, size_t item_size, compare_t compare);

#endif
