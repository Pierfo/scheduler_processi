#include <fcntl.h>
#include <unistd.h>
#include "pause.h"
#include <sched.h>
#include <sys/resource.h>

#define SIZE 471184

int main() {
    while(1) {
        //struct sched_param par;
        //par.sched_priority = sched_get_priority_min(SCHED_FIFO);
        //sched_setscheduler(0, SCHED_FIFO, &par);
        setpriority(PRIO_PROCESS, 0, 5);

        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(0, &set);
        sched_setaffinity(getpid(), sizeof(set), &set);

        int fd = open("../google2.txt", O_RDWR);
        char buf[SIZE];
    
        while(read(fd, buf, SIZE) > 0);
        close(fd);

        pause_h::sleep(0, 1000);
    }
}