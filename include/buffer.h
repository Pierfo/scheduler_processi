#ifndef BUFFER
#define BUFFER
#include <pthread.h>
#include <sys/types.h>

template<int N>
class buffer {
    public:
    buffer();
    void insert(int elem);
    int extract();
    double calculate_fill_percentage();
    ~buffer();
    
    private:
    int start, end; //[start, end)
    int buf[N];
    bool change_happened;
    pthread_mutexattr_t monitor_attr;
    pthread_mutex_t monitor_access;
    pthread_cond_t buffer_empty;
    pthread_cond_t buffer_full;
    pthread_condattr_t cond_attr;
    pthread_cond_t no_change_happened;
    
    int increment(int v) {return (v + 1) % N;};
};

#include "buffer.hpp"

#endif