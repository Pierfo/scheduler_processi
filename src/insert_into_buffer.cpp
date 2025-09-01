#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "shared_memory_object.h"

/*
    Inserisce i dati nel buffer; prima di ogni inserimento, esegue la funzione specificata nello struct 
    shared_memory_object condiviso fra i vari processi 
*/
int main(int argc, char * argv[]) {

    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    sched_setaffinity(getpid(), sizeof(set), &set);

    //Apre un collegamento con la memoria condivisa dal processo main
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
        //Esegue la funzione definita in shared_memory_object.action_before_insertion
        auto obj = shared_memory->action_before_insertion();

        //Inserisce l'elemento nel buffer
        shared_buff->insert(obj);
    }
}