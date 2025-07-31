#include "buffer.h"
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <vector>
#include "shared_memory_object.h"
#include "pause.h"

int main(int argc, char * argv[], char * env[]) {
    struct shared_memory_object shared_memory_object_instance;
    int shared_memory_fd = shm_open("buffer", O_RDWR | O_CREAT, 0600);
    ftruncate(shared_memory_fd, sizeof(shared_memory_object));
    void * shared_memory = mmap(NULL, sizeof(shared_memory_object), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    memcpy(shared_memory, (void*)&shared_memory_object_instance, sizeof(shared_memory_object));

    std::vector<pid_t> pids {};

    if(errno) {
        perror("");
    }

    pid_t pid = fork();

    if(pid == 0) {
        execve("../build_remove_from_buffer/remove_from_buffer", NULL, env);

        if(errno) {
            perror("");
        }
    }

    else {
        pids.push_back(pid);
    }

    pid = fork();
    
    if(pid == 0) {
        execve("../build_insert_into_buffer/insert_into_buffer", NULL, env);

        if(errno) {
            perror("");
        }
    }

    else {
        pids.push_back(pid);
    }

    pid = fork();

    if(pid == 0) {
        execve("../build_monitor_buffer_level/monitor_buffer_level", NULL, env);

        if(errno) {
            perror("");
        }
    }

    else {
        pids.push_back(pid);
    }

    //PASSARE L'INTERVALLO DI TEMPO DA RIGA DI COMANDO
    pause_h::sleep(20, 0);

    for(pid_t p : pids) {
        kill(p, SIGKILL);
    }

    shm_unlink("buffer");

    std::cout << std::endl;
    
    return 0;
}