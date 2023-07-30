#include "temp.h"
#include "../../log.h"
#include "basic.h"


// temp allocator is used to map one page at a time
// since a page requires 4 tables and the root will be already mapped 4-1 = 3 frames
#define MAX_TEMP_ALLOCATOR_FRAMES 3

typedef struct {
    void* frames[MAX_TEMP_ALLOCATOR_FRAMES];
} temp_allocator_t;


temp_allocator_t temp_allocator;


inline void init_temp_allocator() {
    for (int i = 0; i < MAX_TEMP_ALLOCATOR_FRAMES; i++) temp_allocator.frames[i] = allocate_frame();
}

inline void* allocate_temp_frame() {
    for (int i = 0; i < MAX_TEMP_ALLOCATOR_FRAMES; i++) {
        if (temp_allocator.frames[i] != (void*)-1) {
            void* frame              = temp_allocator.frames[i];
            temp_allocator.frames[i] = (void*)-1;
            return frame;
        }
    }
    PANIC("No available frames on temp allocator");
    return (void*)-1;
}

inline void deallocate_temp_frame(void* frame) {
    for (int i = 0; i < MAX_TEMP_ALLOCATOR_FRAMES; i++) {
        if (temp_allocator.frames[i] == (void*)-1) {
            temp_allocator.frames[i] = frame;
            return;
        }
    }
    PANIC("No allocated frames on temp allocator");
}
