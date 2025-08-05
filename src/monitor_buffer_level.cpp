#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <sched.h>
#include <math.h>
#include <vector>
#include <sys/resource.h>
#include "shared_memory_object.h"
#include "pause.h"

void boost(pid_t a, struct sched_param& a_sched_param, pid_t b, struct sched_param& b_sched_param, int prio) {
    if(prio < a_sched_param.sched_priority) return;

    a_sched_param.sched_priority = prio;
    b_sched_param.sched_priority = 0;

    sched_setscheduler(a, SCHED_RR, &a_sched_param);
    sched_setscheduler(b, SCHED_OTHER, &b_sched_param);

    setpriority(PRIO_PROCESS, b, (int)floor(prio * 19 / 90));
}

void decrement_priority(pid_t p, struct sched_param& p_sched_param) {
    if(p_sched_param.sched_priority <= sched_get_priority_min(SCHED_RR)) return;
    
    p_sched_param.sched_priority--;
    sched_setscheduler(p, SCHED_RR, &p_sched_param);

    return;
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

    struct sched_param insert_sched_param;
    struct sched_param remove_sched_param;
    sched_getparam(insert_proc, &insert_sched_param);
    sched_getparam(remove_proc, &remove_sched_param);

    std::string end_part {};
    write(1, "Livello di riempimento: ", 24);
    
    while(true) {
        double percentage = (shared_buff->calculate_fill_percentage() * 100);

        if(percentage <= 25) {
            boost(insert_proc, insert_sched_param, remove_proc, remove_sched_param, (int)floor((25 - percentage) * (90.0 / 25.0)));
        }

        else if(percentage >= 75) {
            boost(remove_proc, remove_sched_param, insert_proc, insert_sched_param, (int)floor((percentage - 75) * (90.0 / 25.0)));
        }

        else {
            decrement_priority(remove_proc, remove_sched_param);
            decrement_priority(insert_proc, insert_sched_param);
        }

        for(char c: end_part) {
            write(1, "\b", 1);
        }

        std::string percentage_string = std::to_string(percentage) + std::string("%");

        while(percentage_string.size() < 15) {
            percentage_string.push_back(' ');
        }

        std::string insert_sched_param_string = std::to_string(insert_sched_param.sched_priority);

        while(insert_sched_param_string.size() < 2) {
            insert_sched_param_string.push_back(' ');
        }

        std::string remove_sched_param_string = std::to_string(remove_sched_param.sched_priority);

        while(remove_sched_param_string.size() < 2) {
            remove_sched_param_string.push_back(' ');
        }

        end_part = percentage_string + std::string{"     insert_into_buffer: "} 
            + insert_sched_param_string + std::string{" remove_from_buffer: "}
            + remove_sched_param_string;

        write(1, end_part.c_str(), end_part.size());
    }
}