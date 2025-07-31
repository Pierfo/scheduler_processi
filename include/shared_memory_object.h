#ifndef SHARED_MEMORY_OBJECT
#define SHARED_MEMORY_OBJECT
#include "buffer.h"

struct shared_memory_object {
    buffer<1000001> shared_buffer;
};

#endif