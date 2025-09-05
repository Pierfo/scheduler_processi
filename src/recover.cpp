#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sched.h>
#include <iostream>
#include "shared_memory_object.h"
#include "pause.h"

int main() {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    sched_setaffinity(getpid(), sizeof(set), &set);

    struct sched_param par;
    par.sched_priority = sched_get_priority_max(SCHED_FIFO);
    sched_setscheduler(0, SCHED_FIFO, &par);

    //Apre un collegamento con la memoria condivisa costruita dal processo main
    int shared_memory_fd = shm_open("buffer", O_RDWR, 0600);
    //Tronca l'area di memoria rappresentata dal file descriptor shared_memory_fd in modo tale da avere dimensione pari 
    //alla dimensione di shared_memory_object
    ftruncate(shared_memory_fd, sizeof(shared_memory_object));
    //L'area di memoria condivisa viene mappata nel proprio spazio degli indirizzi logici
    shared_memory_object * shared_memory = (shared_memory_object*)mmap(NULL, sizeof(shared_memory_object), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    auto shared_buff = &shared_memory->shared_buffer;
    
    if(errno) {
        perror("");
    }
    
    double old_percentage = shared_buff->calculate_fill_percentage();

    while(true) {
        pause_h::sleep(15, 0);

        double new_percentage = shared_buff->calculate_fill_percentage();

        if(new_percentage - old_percentage < 0.00001 && new_percentage - old_percentage > -0.00001) {
            std::cout << "\n\nRECOVER\n" << std::endl;
            
            shared_buff->switch_off();

            pause_h::sleep(2, 0);

            shared_buff->switch_on();
        }
         
        old_percentage = new_percentage;
    }
}