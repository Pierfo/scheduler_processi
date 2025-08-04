#include <unistd.h>
#include "buffer.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <sched.h>
#include <math.h>
#include <sys/resource.h>
#include <fstream>
#include "shared_memory_object.h"
#include "pause.h"

void boost(pid_t a, pid_t b, int prio) {
    struct sched_param a_sched_param;
    struct sched_param b_sched_param;

    sched_getparam(a, &a_sched_param);
    sched_getparam(b, &b_sched_param);

    if(prio < a_sched_param.sched_priority) return;

    a_sched_param.sched_priority = sched_get_priority_max(SCHED_RR);
    b_sched_param.sched_priority = 0;

    sched_setscheduler(a, SCHED_RR, &a_sched_param);
    sched_setscheduler(b, SCHED_OTHER, &b_sched_param);

    setpriority(PRIO_PROCESS, b, (int)floor(prio * 19 / 99));
}

void decrement_priority(pid_t p) {
    if(sched_getscheduler(p) == SCHED_OTHER) return;
    
    struct sched_param p_sched_param;
    sched_getparam(p, &p_sched_param);
    
    p_sched_param.sched_priority--;
    sched_setscheduler(p, SCHED_RR, &p_sched_param);

    return;
}

void log(std::fstream& logfile, double perc, pid_t a, pid_t b) {
    struct sched_param a_sched_param;
    struct sched_param b_sched_param;

    sched_getparam(a, &a_sched_param);
    sched_getparam(b, &b_sched_param);

    logfile << "buffer: " << perc << "%, insert: " << a_sched_param.sched_priority << ", remove: " << b_sched_param.sched_priority << std::endl;
}

int main(int argc, char* argv[]) {
    pid_t insert_proc = (pid_t)std::stoi(std::string{argv[1]});
    pid_t remove_proc = (pid_t)std::stoi(std::string{argv[0]});

    struct sched_param monitor_sched_param;

    sched_getparam(0, &monitor_sched_param);

    monitor_sched_param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    sched_setscheduler(0, SCHED_FIFO, &monitor_sched_param);

    int shared_memory_fd = shm_open("buffer", O_RDWR, 0600);
    ftruncate(shared_memory_fd, sizeof(shared_memory_object));
    shared_memory_object * shared_memory = (shared_memory_object*)mmap(NULL, sizeof(shared_memory_object), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    auto shared_buff = &shared_memory->shared_buffer;
    if(errno) {
        perror("");
    }

    //for(int i = 0; i < 100; i++) write(1, "=", 1);

    write(1, "Buffer level: ", 14);
    std::string end_part {};

    std::fstream logfile {"../log.txt"};
    
    while(true) {
        double percentage = (shared_buff->calculate_fill_percentage() * 100);

        if(percentage <= 25) {
            boost(insert_proc, remove_proc, (int)floor((25 - percentage) * (99.0 / 25.0)));
            //printf("\033[92m");
        }

        else if(percentage >= 75) {
            boost(remove_proc, insert_proc, (int)floor((percentage - 75) * (99.0 / 25.0)));
            //printf("\033[93m");
        }

        else {
            decrement_priority(remove_proc);
            decrement_priority(insert_proc);
        }

        for(char c : end_part) {
            write(1, "\b", 1);
            write(1, " ", 1);
            write(1, "\b", 1);
        }
        
        std::stringstream end {};
        end << percentage << "%";
        end_part = end.str();
        
        write(1, end_part.c_str(), end_part.size());

        log(logfile, percentage, insert_proc, remove_proc);
    }
}