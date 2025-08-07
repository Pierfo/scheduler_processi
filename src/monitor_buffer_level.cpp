#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include <sched.h>
#include <math.h>
#include <sys/resource.h>
#include "shared_memory_object.h"

//Il massimo valore di priorità da attribuire ai processi di inserimento o rimozione
#define MAXIMUM_PRIORITY 80

//Aumenta la priorità del processo a, facendolo passare alla politica SCHED_RR, e diminuisce quella del processo b,
//facendolo passare alla politica di default SCHED_OTHER.
void boost_priority(pid_t a, struct sched_param& a_sched_param, pid_t b, struct sched_param& b_sched_param, int prio) {
    if(prio < a_sched_param.sched_priority) return;

    a_sched_param.sched_priority = prio;
    b_sched_param.sched_priority = 0;

    sched_setscheduler(a, SCHED_RR, &a_sched_param);
    sched_setscheduler(b, SCHED_OTHER, &b_sched_param);

    //Definisce il nice value per b
    setpriority(PRIO_PROCESS, b, (int)floor(prio * 19 / MAXIMUM_PRIORITY));
}

//Se la priorità del processo p è maggiore del valore minimo, allora diminuisce questa di un'unità
void decrement_priority(pid_t p, struct sched_param& p_sched_param) {
    if(p_sched_param.sched_priority <= sched_get_priority_min(SCHED_RR)) return;
    
    p_sched_param.sched_priority--;
    sched_setscheduler(p, SCHED_RR, &p_sched_param);

    return;
}

/*
    Monitora il livello di riempimento del buffer. Se questo è minore o uguale a 0.25 allora aumenta la priorità del
    processo che inserisce dati nel buffer, mentre se è maggiore o uguale a 0.75 allora aumenta la priorità del 
    processo che estrae dati dal buffer
*/
int main(int argc, char* argv[]) {
    //Ottiene i pid dei processi che inseriscono e che estraggono i dati dal buffer, forniti come argomenti al programma
    pid_t insert_proc = (pid_t)std::stoi(std::string{argv[1]});
    pid_t remove_proc = (pid_t)std::stoi(std::string{argv[0]});

    //Ottiene una copia dei propri parametri di scheduling
    struct sched_param monitor_sched_param;
    sched_getparam(0, &monitor_sched_param);

    //Conferisce a sé stesso la priorità massima
    monitor_sched_param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    sched_setscheduler(0, SCHED_FIFO, &monitor_sched_param);

    //Apre un collegamento con la memoria condivisa dal processo main
    int shared_memory_fd = shm_open("buffer", O_RDWR, 0600);
    //Tronca l'area di memoria rappresentata dal file descriptor shared_memory_fd in modo tale da avere dimensione pari 
    //alla dimensione di shared_memory_object
    ftruncate(shared_memory_fd, sizeof(shared_memory_object));
    //L'area di memoria condivisa viene mappata nel proprio spazio degli indirizzi logici
    shared_memory_object * shared_memory = (shared_memory_object*)mmap(NULL, sizeof(shared_memory_object), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    //Ottiene l'accesso al buffer condiviso
    auto shared_buff = &shared_memory->shared_buffer;

    //Stampa eventuali messaggi di errore
    if(errno) {
        perror("");
    }

    //Ottiene una copia dei parametri di scheduling dei processi che inseriscono e che estraggono dati dal buffer
    struct sched_param insert_sched_param;
    struct sched_param remove_sched_param;
    sched_getparam(insert_proc, &insert_sched_param);
    sched_getparam(remove_proc, &remove_sched_param);

    std::string end_part {};
    write(1, "Livello di riempimento: ", 24);
    
    while(true) {
        //Ottiene la percentuale di riempimento del buffer. Per come questa funzione è stata costruita, si ha che il
        //processo deve rimanere bloccato su tale istruzione finché non è stato effettuato almeno un inserimento o 
        //rimozione dall'ultima volta che è stata letta la percentuale. Così facendo ci si assicura che i processi di
        //inserimento e rimozione abbiano comunque la possibilità di eseguire nonostante il fatto che il processo di 
        //monitoraggio abbia la priorità massima
        double percentage = (shared_buff->calculate_fill_percentage() * 100);

        //Se la percentuale di riempimento è minore o uguale al 25% allora aumenta la priorità del processo di inserimento e 
        //diminuisce la priorità del processo di rimozione.
        //La nuova priorità che viene attribuita al processo di inserimento è definita dalla funzione
        //floor((25-percentage)*(MAXIMUM_PRIORITY/25.0))
        if(percentage <= 25) {
            boost_priority(insert_proc, insert_sched_param, remove_proc, remove_sched_param, (int)floor((25 - percentage) * (MAXIMUM_PRIORITY / 25.0)));
        }

        //Se la percentuale di riempimento è maggiore o uguale al 75% allora aumenta la priorità del processo di estrazione e
        //dimunuisce la priorità del processo di inserimento.
        //La nuova priorità che viene attribuita al processo di estrazione è definita dalla funzione
        //floor((percentage-75)*(MAXIMUM_PRIORITY/25.0))
        else if(percentage >= 75) {
            boost_priority(remove_proc, remove_sched_param, insert_proc, insert_sched_param, (int)floor((percentage - 75) * (MAXIMUM_PRIORITY / 25.0)));
        }

        //Se la percentuale di riempimento è compresa fra il 26% e il 74%, decrementa la priorità di entrambi i 
        //processi di un'unità 
        else {
            decrement_priority(remove_proc, remove_sched_param);
            decrement_priority(insert_proc, insert_sched_param);
        }

        //Cancella il messaggio stampato su standard input nella precedente iterazione del while loop
        for(char c: end_part) {
            write(1, "\b", 1);
        }

        //Converte la percentuale di riempimento in stringa e si assicura che questa abbia lunghezza 15
        std::string percentage_string = std::to_string(percentage) + std::string("%");

        while(percentage_string.size() < 15) {
            percentage_string.push_back(' ');
        }

        //Converte la priorità del processo di inserimento in stringa; se la priorità è nulla, allora converte in
        //stringa il nice value di tale processo
        std::string insert_sched_param_string = std::to_string((insert_sched_param.sched_priority) 
            ? insert_sched_param.sched_priority : (-getpriority(PRIO_PROCESS, insert_proc)));

        //Si assicura che la stringa abbia lunghezza 3
        while(insert_sched_param_string.size() < 3) {
            insert_sched_param_string.push_back(' ');
        }

        //Converte la priorità del processo di rimozione in stringa; se la priorità è nulla, allora converte in
        //stringa il nice value di tale processo
        std::string remove_sched_param_string = std::to_string((remove_sched_param.sched_priority)
            ? remove_sched_param.sched_priority : (-getpriority(PRIO_PROCESS, remove_proc)));

        //Si assicura che la stringa abbia lunghezza 3
        while(remove_sched_param_string.size() < 3) {
            remove_sched_param_string.push_back(' ');
        }

        end_part = percentage_string + std::string{"     insert_into_buffer: "} 
            + insert_sched_param_string + std::string{"          remove_from_buffer: "}
            + remove_sched_param_string;

        write(1, end_part.c_str(), end_part.size());
    }
}