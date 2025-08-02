#include "buffer.h"
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <vector>
#include <string>
#include "shared_memory_object.h"
#include "pause.h"
#include "matrix.h"

int main(int argc, char * argv[], char * env[]) {   
    if(argc != 3) {
        std::cout << "Sintassi corretta: \"./main sec msec\"" << std::endl;
        return 0;
    }

    std::string seconds_string {argv[1]};
    std::string microseconds_string {argv[2]};

    int seconds = std::stoi(seconds_string);
    int microseconds = std::stoi(microseconds_string);

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
        execve("../build_remove_from_buffer/remove_from_buffer", argv, env);

        if(errno) {
            perror("");
        }
    }

    else {
        pids.push_back(pid);
    }

    pid = fork();
    
    if(pid == 0) {
        execve("../build_insert_into_buffer/insert_into_buffer", argv, env);

        if(errno) {
            perror("");
        }
    }

    else {
        pids.push_back(pid);
    }

    pid = fork();

    if(pid == 0) {
        execve("../build_monitor_buffer_level/monitor_buffer_level", argv, env);

        if(errno) {
            perror("");
        }
    }

    else {
        pids.push_back(pid);
    }

    pause_h::sleep(seconds, microseconds);

    for(pid_t p : pids) {
        kill(p, SIGKILL);
    }

    shm_unlink("buffer");

    std::cout << std::endl;
    
    return 0;
}