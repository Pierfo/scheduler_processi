#include <unistd.h>
#include "buffer.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include "shared_memory_object.h"
#include "pause.h"

int main(int argc, char* argv[]) {
    int shared_memory_fd = shm_open("buffer", O_RDWR, 0600);
    ftruncate(shared_memory_fd, sizeof(shared_memory_object));
    shared_memory_object * shared_memory = (shared_memory_object*)mmap(NULL, sizeof(shared_memory_object), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    auto shared_buff = &shared_memory->shared_buffer;
    if(errno) {
        perror("");
    }

    write(1, "The buffer is ", 14);
    std::string end_part {};
    
    while(true) {
        double percentage = (shared_buff->calculate_fill_percentage() * 100);
        
        for(char c : end_part) {
            write(1, "\b", 1);
            write(1, " ", 1);
            write(1, "\b", 1);
        }
        
        std::stringstream end {};
        end << percentage << "% full";
        end_part = end.str();
        
        write(1, end_part.c_str(), end_part.size());
    }
}