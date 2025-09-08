#ifndef CAPTURE_TIME
#define CAPTURE_TIME

#include <sys/time.h>

double capture_time() {
    struct timeval t;

    gettimeofday(&t, NULL);

    return (t.tv_sec + ((double)t.tv_usec/1000000));
}

#endif