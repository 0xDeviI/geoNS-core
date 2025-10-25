#include "core.h"


uchar is_debugging = 0;
uchar is_geons_running = 0;


void *memalloc(size_t size) {
    void *allocated_memory = calloc(1, size);
    if (!allocated_memory && size)
        return NULL;
    
    return allocated_memory;
}