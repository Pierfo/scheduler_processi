#include <iostream>
#include "buffer.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "shared_memory_object.h"
#include "pause.h"

int main(int argc, char * argv[]) {
    int value = 1;
    
    int shared_memory_fd = shm_open("buffer", O_RDWR, 0600);
    ftruncate(shared_memory_fd, sizeof(shared_memory_object));
    shared_memory_object * shared_memory = (shared_memory_object*)mmap(NULL, sizeof(shared_memory_object), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    auto shared_buff = &shared_memory->shared_buffer;
    if(errno) {
        perror("");
    }
    
    while(true) {
        //pause_h::sleep(0, 500000000);
        shared_buff->insert(value);
    }
}