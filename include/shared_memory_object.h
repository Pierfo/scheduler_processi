#ifndef SHARED_MEMORY_OBJECT
#define SHARED_MEMORY_OBJECT
#include "buffer.h"
#include "matrix.h"
#include "actions.h"

#define MATRIX_SIZE 7
#define BUFFER_SIZE 1001

struct shared_memory_object {
    buffer<matrix<MATRIX_SIZE>, BUFFER_SIZE> shared_buffer = buffer<matrix<MATRIX_SIZE>, BUFFER_SIZE>{};
    matrix_action_before_insertion<MATRIX_SIZE> action_before_insertion = matrix_action_before_insertion<MATRIX_SIZE>{};
    matrix_action_after_extraction<MATRIX_SIZE> action_after_extraction = matrix_action_after_extraction<MATRIX_SIZE>{};
};

#endif