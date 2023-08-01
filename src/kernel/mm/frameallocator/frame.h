#ifndef FRAME_ALLOCATOR_FRAME_H
#define FRAME_ALLOCATOR_FRAME_H


typedef const void* (*allocate_frame_t)();
typedef void (*deallocate_frame_t)(const void* frame);

#endif
