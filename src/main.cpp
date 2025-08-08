#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <iostream>
#include <vector>
#include <string>
#include "shared_memory_object.h"
#include "pause.h"

/*
    Avvia l'esecuzione dei processi di inserimento ed estrazione dei dati e di monitoraggio del livello del buffer, 
    inoltre costruisce l'area di memoria che sarà condivisa fra i tre processi figli.
    Il programma richiede che si passino due argomenti da linea di comando, i quali indicano rispettivamente per quanti
    secondi e nanosecondi questo deve eseguire (ed esempio "sudo ./main 2 500000000" indica che il programma deve
    eseguire per 2 secondi e 500000000 nanosecondi).
    Il programma deve essere avviato in modalità sudo.
*/
int main(int argc, char * argv[], char * env[]) {   
    if(argc != 3) {
        std::cout << "Sintassi corretta: \"sudo "<< argv[0] << " sec nsec\"" << std::endl;
        return 0;
    }

    int seconds = -1;
    int nanoseconds = -1;

    try {
        //Ottiene i secondi e nanosecondi per cui il programma dev'essere eseguito
        seconds = std::stoi(std::string{argv[1]});
        nanoseconds = std::stoi(std::string{argv[2]});
    }

    catch(std::invalid_argument e) {
        std::cout << "Errore: \"" << (seconds == -1 ? argv[1] : argv[2]) << "\" non è un numero" << std::endl;
        return 0;
    }

    catch(std::out_of_range e) {
        std::cout << "Errore: \"" << (seconds == -1 ? argv[1] : argv[2]) << "\" è un valore troppo grande" << std::endl;
        return 0;
    }

    if(seconds < 0 || nanoseconds < 0) {
        std::cout << "Errore: \"" << (seconds < 0 ? seconds : nanoseconds) << "\" non è un numero positivo" << std::endl;
        return 0;
    }

    if(nanoseconds > 999999999) {
        std::cout << "Errore: il secondo parametro non può superare 999999999" << std::endl;
        return 0;
    }

    //Ottiene una copia dei propri parametri di scheduling
    struct sched_param par;
    sched_getparam(0, &par);

    //Applica a sé stesso la politica di scheduling SCHED_FIFO con priorità massima, questo serve per assicurarsi che 
    //il processo possa risvegliarsi non appena termina il periodo di inattività
    par.sched_priority = sched_get_priority_max(SCHED_FIFO);
    sched_setscheduler(0, SCHED_FIFO | SCHED_RESET_ON_FORK, &par);

    //Se errno assume valore EPERM allora il processo non ha avuto l'autorizzazione per eseguire l'ultima istruzione, 
    //dunque il programma non è stato eseguito in modalità sudo
    if(errno == EPERM) {
        std::cout << "Autorizzazione non concessa, prova con \"sudo " << argv[0] << " " << argv[1] << " " << argv[2] << "\"" << std::endl;
        return 0;
    }

    pause_h::sleep(0, 5000);

    //Istanzia lo struct shared_memory_object
    struct shared_memory_object shared_memory_object_instance;
    //Crea una nuova area di memoria che sarà condivisa con gli altri processi
    int shared_memory_fd = shm_open("buffer", O_RDWR | O_CREAT, 0600);
    //Tronca l'area di memoria creata così da avere dimensione pari a quella di struct shared_memory_object
    ftruncate(shared_memory_fd, sizeof(shared_memory_object));
    //L'area di memoria creata viene mappata nel proprio spazio degli indirizzi logici
    void * shared_memory = mmap(NULL, sizeof(shared_memory_object), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);
    //Copia l'istanza di struct shared_memory_object nell'area di memoria condivisa
    memcpy(shared_memory, (void*)&shared_memory_object_instance, sizeof(shared_memory_object));

    //Vettore che conterrà i pid dei processi figli
    std::vector<pid_t> children {};

    //Stampa l'eventuale messaggio di errore
    if(errno) {
        perror("");
    }

    //Crea un nuovo processo
    pid_t pid = fork();

    if(pid == 0) {
        //Se è il processo figlìo allora esegue il programma di estrazione dei dati dal buffer
        execve("../build_remove_from_buffer/remove_from_buffer", argv, env);

        if(errno) {
            perror("");
        }
    }

    else {
        //Se è il processo genitore allora inserisce il pid del processo figlio nel vettore
        children.push_back(pid);
    }

    pid = fork();
    
    if(pid == 0) {
        //Se è il processo figlio allora esegue il programma di inserimento dei dati nel buffer
        execve("../build_insert_into_buffer/insert_into_buffer", argv, env);

        if(errno) {
            perror("");
        }
    }

    else {
        children.push_back(pid);
    }

    //Prepara l'area di memoria dove salvare i pid dei due processi creati. Tale area sarà poi passata come
    //argomento al programma di monitoraggio del livello del buffer
    char** args = (char**)malloc(3*sizeof(char*));
    
    for(int i = 0; i < children.size(); i++) {
        //Converte ciascun pid in stringa e poi la copia in args
        std::string arg {std::to_string(children[i])};
        args[i] = (char*)malloc(arg.size() + 1);

        for(int j = 0; j < arg.size(); j++) {
            args[i][j] = arg[j];
        }        

        args[i][arg.size()] = 0;
    }

    //Quando si passano degli argomenti, l'ultimo puntatore dev'essere NULL
    args[2] = NULL;

    pid = fork();

    if(pid == 0) {
        //Se è il processo figlio allora esegue il programma di monitoraggio del livello del buffer, passando args 
        //come argomento
        execve("../build_monitor_buffer_level/monitor_buffer_level", args, env);

        if(errno) {
            perror("");
        }
    }

    else {
        children.push_back(pid);
    }

    //Sospende la propria esecuzione per il periodo di tempo definito dall'utente
    pause_h::sleep(seconds, nanoseconds);

    //Uccide tutti i processi figli
    for(pid_t p : children) {
        kill(p, SIGKILL);
    }

    for(int i = 0; i < 3; i++) {
        free(args[i]);
    }

    free(args);

    //Dealloca l'area di memoria condivisa
    shm_unlink("buffer");

    std::cout << std::endl;
    
    return 0;
}