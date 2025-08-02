#ifndef SHARED_MEMORY_OBJECT
#define SHARED_MEMORY_OBJECT
#include "buffer.h"
#include "matrix.h"

/**
    ATTENZIONE AI LIMITI DI SPAZIO
*/

struct shared_memory_object {
    buffer<matrix<10>, 10000> shared_buffer;
};

#endif