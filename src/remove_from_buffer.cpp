#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "shared_memory_object.h"

/*
    Estrae i dati dal buffer e, a ogni estrazione, esegue la funzione specificata dal campo action_after_extraction 
    all'interno dello struct shared_memory_object, il quale è condiviso fra i vari processi 
*/
int main(int argc, char * argv[]) {
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
    
    while(true) {
        //Estrae un elemento dal buffer
        auto value = shared_buff->extract();

        //Esegue la funzione definita in shared_memory_object.action_after_extraction
        shared_memory->action_after_extraction(value);
    }
}