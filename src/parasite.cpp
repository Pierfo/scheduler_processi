#include <fcntl.h>
#include <unistd.h>
#include "pause.h"
#include <sched.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <iostream>

#define SIZE 1026631
#define MAX_PERIOD 10

int main(int argc, char * argv[]) {
    while(1) {
        struct sched_param par;
        par.sched_priority = sched_get_priority_min(SCHED_FIFO);
        sched_setscheduler(0, SCHED_FIFO, &par);
        //setpriority(PRIO_PROCESS, 0, 1);

        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(0, &set);
        sched_setaffinity(getpid(), sizeof(set), &set);

        int fd = open("../file.txt", O_RDWR | O_NONBLOCK);
        char buf[SIZE];
    
        //std::cout << "\t\tstart reading " << getpid() << std::endl;
        while(read(fd, buf, SIZE) > 0);
        //std::cout << "\t\tdone reading " << getpid() << std::endl;
        close(fd);

        pause_h::sleep(0, MAX_PERIOD);
    }
}