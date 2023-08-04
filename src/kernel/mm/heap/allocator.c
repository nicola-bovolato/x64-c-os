#include "allocator.h"
#include "../../lib/mem.h"
#include "../../log.h"
#include <stdbool.h>
#include <stdint.h>


typedef struct free_mem_region_t {
    struct free_mem_region_t* next;
    bool                      used;
    size_t                    size;
} free_mem_region_t;

static inline void*
mark_used_region(free_mem_region_t* node, free_mem_region_t* next_node, size_t size);
static inline void mark_free_region(
    free_mem_region_t* node, free_mem_region_t* prev_node, free_mem_region_t* next_node
);


static free_mem_region_t* root;
static size_t             free_bytes = HEAP_SIZE - sizeof(free_mem_region_t);


void init_heap_allocator(void* start_address) {
    root       = start_address;
    root->next = NULL;
    root->used = false;
    root->size = free_bytes;
    DEBUG("Heap initialized (start = %p, size = %p)\n", root, free_bytes);
}

void* allocate(size_t size) {
    if (size > free_bytes) return NULL;

    free_mem_region_t* node = root;

    while (node != NULL) {
        if (node->size >= size && !node->used) {
            return mark_used_region(node, node->next, size);
        }

        node = node->next;
    }

    return NULL;
}

void deallocate(void* address) {
    // is the address in the heap memory region?
    if ((size_t)address < (size_t)root || (size_t)address > (size_t)root + HEAP_SIZE) return;


    free_mem_region_t* prev_node = NULL;
    free_mem_region_t* node      = root;
    while (node != NULL && (size_t)node + sizeof(free_mem_region_t) + node->size < (size_t)address
    ) {
        prev_node = node;
        node      = node->next;
    }

    // is the address already in a free region?
    if (!node->used) return;

    mark_free_region(node, prev_node, node->next);
}


static inline void*
mark_used_region(free_mem_region_t* node, free_mem_region_t* next_node, size_t size) {
    free_mem_region_t* used_node = node;

    // is there enough space to insert another node?
    if (node->size > size + sizeof(free_mem_region_t)) {

        // add the used_node at the end of the current node
        used_node       = (free_mem_region_t*)((size_t)node + sizeof(free_mem_region_t) + node->size
                                         - sizeof(free_mem_region_t) - size);
        used_node->size = size;
        DEBUG("New node added: (start = %p, size = %p)\n", used_node, used_node->size);

        node->size -= sizeof(free_mem_region_t) + used_node->size;
        free_bytes -= sizeof(free_mem_region_t);

        node->next      = used_node;
        used_node->next = next_node;
    }
    // mark the node as used
    used_node->used  = true;
    free_bytes      -= used_node->size;
    DEBUG("Free space after alloc: %p\n", free_bytes);

    memset((void*)((size_t)used_node + sizeof(free_mem_region_t)), 0, used_node->size);
    return (void*)((size_t)used_node + sizeof(free_mem_region_t));
}

static inline void mark_free_region(
    free_mem_region_t* node, free_mem_region_t* prev_node, free_mem_region_t* next_node
) {
    // TODO: same for next node
    node->used  = false;
    free_bytes += node->size;

    // if the next node is free merge it
    if (next_node != NULL && !next_node->used) {
        node->size += next_node->size + sizeof(free_mem_region_t);
        node->next  = next_node->next;

        free_bytes += sizeof(free_mem_region_t);
        DEBUG("Next node removed: (start = %p, size = %p)\n", next_node, next_node->size);
    }

    // if the previous node is also free merge them together
    if (prev_node != NULL && !prev_node->used) {
        prev_node->size += node->size + sizeof(free_mem_region_t);
        prev_node->next  = next_node;

        free_bytes += sizeof(free_mem_region_t);
        DEBUG("Node removed: (start = %p, size = %p)\n", node, node->size);
    }

    DEBUG("Free space after dealloc: %p\n", free_bytes);
}
