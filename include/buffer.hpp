#ifndef BUFFER_HPP
#define BUFFER_HPP
#include <cstdlib>
#include <iostream>
#include <errno.h>

template<typename T, int N>
buffer<T, N>::buffer()
    : start{0}, end{0}, change_happened{true} {
    //
    pthread_mutexattr_init(&monitor_attr);
    pthread_mutexattr_setpshared(&monitor_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_settype(&monitor_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&buffer_access, &monitor_attr);

    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&buffer_empty, &cond_attr);
    pthread_cond_init(&buffer_full, &cond_attr);
    pthread_cond_init(&no_change_happened, &cond_attr);
}

template<typename T, int N>
void buffer<T, N>::insert(T elem) {
    pthread_mutex_lock(&buffer_access);

    while(start == increment(end)) {
        pthread_cond_wait(&buffer_full, &buffer_access);
    }
    
    buf[end] = elem;
    end = increment(end);

    if(!change_happened) {
        change_happened = true;
        pthread_cond_signal(&no_change_happened);
    }
    
    if(end == increment(start)) {
        pthread_cond_signal(&buffer_empty);
    }
    
    pthread_mutex_unlock(&buffer_access);
}

template<typename T, int N>
T buffer<T, N>::extract() {
    pthread_mutex_lock(&buffer_access);

    while(start == end) {
        pthread_cond_wait(&buffer_empty, &buffer_access);
    }

    T elem = buf[start];
    start = increment(start);
    
    if(!change_happened) {
        change_happened = true;
        pthread_cond_signal(&no_change_happened);
    }

    if(start == increment(increment(end))) {
        pthread_cond_signal(&buffer_full);
    }

    pthread_mutex_unlock(&buffer_access);
    return elem;
}

template<typename T, int N>
double buffer<T, N>::calculate_fill_percentage() {
    pthread_mutex_lock(&buffer_access);
    
    while(!change_happened) {
        pthread_cond_wait(&no_change_happened, &buffer_access);
    }

    int nof_elements = 0;

    for(int i = start; i != end; i = increment(i)) {
        nof_elements++;
    }

    double nof_elements_double = nof_elements * 1.0;

    change_happened = false;

    pthread_mutex_unlock(&buffer_access);

    return nof_elements_double / (N - 1);
}

template<typename T, int N>
buffer<T, N>::~buffer() {
    pthread_mutex_destroy(&buffer_access);
    pthread_mutexattr_destroy(&monitor_attr);

    pthread_cond_destroy(&buffer_empty);
    pthread_cond_destroy(&buffer_full);
    pthread_cond_destroy(&no_change_happened);
    pthread_condattr_destroy(&cond_attr);
}

#endif