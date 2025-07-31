#ifndef PAUSE
#define PAUSE
#include <time.h>

namespace pause_h {
    int sleep(int seconds, int nanoseconds) {        
        struct timespec t;
        t.tv_sec = seconds;
        t.tv_nsec = nanoseconds;

        return nanosleep(&t, NULL);
    }
}

#endif