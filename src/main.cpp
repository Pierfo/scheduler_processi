#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <math.h>
#include <sys/time.h>
#include "shared_memory_object.h"
#include "pause.h"

#define INTERVAL 2
#define WARMUP_TIME 10
#define SWITCHOFF_TIME 1

/*
    Avvia l'esecuzione dei processi di inserimento ed estrazione dei dati e di monitoraggio del livello del buffer, 
    inoltre costruisce l'area di memoria che sarà condivisa fra i tre processi figli.
    Il programma richiede che si passino due argomenti da linea di comando, i quali indicano rispettivamente per quanti
    secondi e nanosecondi questo deve eseguire (ed esempio "sudo ./main 2 500000000" indica che il programma deve
    eseguire per 2 secondi e 500000000 nanosecondi).
    Il programma deve essere avviato in modalità sudo.
*/
int main(int argc, char * argv[], char * env[]) {   
    if(argc != 2) {
        printf("Correct syntax: \"sudo %s n_measurement\"\n", argv[0]);
        return 0;
    }

    int nof_iterations = -1;

    try {
        nof_iterations = std::stoi(std::string{argv[1]});
    }

    catch(std::invalid_argument e) {
        printf("Error: \"%s\" is not a number\n", argv[1]);
        return 0;
    }

    catch(std::out_of_range e) {
        printf("Error: \"%s\" is too large\n", argv[1]);
        return 0;
    }

    if(nof_iterations < 0) {
        printf("Error: \"%d\" is not positive\n", nof_iterations);
        return 0;
    }

    std::string mode;
    
    system("uname -a > out.txt");
    std::fstream out {"out.txt"};
    std::string line {};
    getline(out, line);

    if(line.find("generic") != std::string::npos) {
        mode = "generic";
    }

    else if(line.find("realtime") != std::string::npos) {
        mode = "realtime";
    }

    else {
        printf("Could not find preemption mode\n");
        return 0;
    }

    out.close();
    system("rm out.txt");

    mlockall(MCL_FUTURE);

    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    CPU_SET(1, &set);
    //sched_setaffinity(getpid(), sizeof(set), &set);

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
        printf("Error: permission denied, try with \"sudo %s %s\"\n", argv[0], argv[1]);
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

    if(errno) {
        perror("");
    }

    //Conterrà gli argomenti da passare ai tre processi
    char arguments_buffer[150];
    int arguments_index = 0;

    pid_t pid = fork();

    if(pid == 0) {
        char * arguments[2];
        arguments[0] = arguments_buffer + arguments_index;
        strcpy(arguments[0], "./remove_from_buffer");
        arguments_index += strlen("./remove_from_buffer") + 1;
        arguments[1] = (char*)NULL;

        execve("../build_remove_from_buffer/remove_from_buffer", (char* const*)arguments, env);

        if(errno) {
            perror("");
        }
    }

    else {
        children.push_back(pid);
    }

    pid = fork();
    
    if(pid == 0) {        
        char * arguments[2];
        arguments[0] = arguments_buffer + arguments_index;
        strcpy(arguments[0], "./insert_into_buffer");
        arguments_index += strlen("./insert_into_buffer") + 1;
        arguments[1] = (char*)NULL;
        
        execve("../build_insert_into_buffer/insert_into_buffer", (char* const*)arguments, env);

        if(errno) {
            perror("");
        }
    }

    else {
        children.push_back(pid);
    }

    //pid = fork();
    pid_t recover_pid = 0;
    
    /*if(pid == 0) {        
        char * arguments[2];
        arguments[0] = arguments_buffer + arguments_index;
        strcpy(arguments[0], "./recover");
        arguments_index += strlen("./recover") + 1;
        arguments[1] = (char*)NULL;
        
        execve("../build_recover/recover", (char* const*)arguments, env);

        if(errno) {
            perror("");
        }
    }

    else {
        children.push_back(pid);

        recover_pid = pid;
    }*/

    for(int i = 0; i < 10; i++) {
        pid = fork();
        
        if(pid == 0) {        
            char * arguments[3];
            arguments[0] = arguments_buffer + arguments_index;
            strcpy(arguments[0], "./parasite");
            arguments_index += strlen("./parasite") + 1;
            arguments[1] = arguments_buffer + arguments_index;
            arguments_index += sprintf(arguments_buffer + arguments_index, "%d", (i % 2)) + 1;
            arguments[2] = (char*)NULL;
            
            execve("../build_parasite/parasite", (char* const*)arguments, env);

            if(errno) {
                perror("");
            }
        }

        else {
            children.push_back(pid);
        }
    }

    pid_t parent_pid = getpid();

    pid = fork();

    if(pid == 0) {
        char * arguments[4];
        arguments[0] = arguments_buffer + arguments_index;
        strcpy(arguments[0], "./monitor_buffer_level");
        arguments_index += strlen("./monitor_buffer_level") + 1;

        for(int i = 0; i < 2; i++) {
            arguments[i+1] = arguments_buffer + arguments_index;
            sprintf(arguments[i+1], "%d", children[i]);
            arguments_index += strlen(arguments[i+1]) + 1;
        }

        arguments[3] = (char*)NULL;
        execve("../build_monitor_buffer_level/monitor_buffer_level", (char * const* )arguments, env);

        if(errno) {
            perror("");
        }
    }

    else {
        children.push_back(pid);
    }

    char command[50];
    sprintf(command, "echo \"\" >> ../%s.csv", mode.c_str());
    system(command);

    struct timeval start;
    struct timeval end;

    int measurement = 1;

    auto& buffer = ((shared_memory_object*)shared_memory)->shared_buffer;

    pause_h::sleep(WARMUP_TIME, 0);


    while(nof_iterations) {
        std::cout << "\n\nSTART MEASUREMENT NUMBER " << measurement << "\n" << std::endl;
        //Sospende la propria esecuzione per il periodo di tempo definito dall'utente
        pause_h::sleep(INTERVAL, 0);
        
        buffer.switch_off();
        pause_h::sleep(SWITCHOFF_TIME, 0);
        for(pid_t p : children) {
            if(p != recover_pid) {
                kill(p, SIGSTOP);
            }
        }
        buffer.switch_on();

        double initial_percentage = buffer.calculate_fill_percentage();
        
        ((shared_memory_object*)shared_memory)->priority_boost = children[1];

        for(pid_t p : children) {
            kill(p, SIGCONT);
        }
        
        gettimeofday(&start, NULL);
        
        buffer.wait_until_full();
        
        gettimeofday(&end, NULL);

        buffer.switch_off();
        pause_h::sleep(SWITCHOFF_TIME, 0);

        for(pid_t p : children) {
            if(p != recover_pid) {
                kill(p, SIGSTOP);
            }
        }
        
        double start_val = start.tv_sec + start.tv_usec / (double)1000000;
        double end_val = end.tv_sec + end.tv_usec / (double)1000000;
        
        double elapsed_time = end_val - start_val;
        double speed = (1 - initial_percentage) * buffer.size() / elapsed_time;
        
        std::cout << "\nactually elapsed time " << elapsed_time << std::endl;

        std::cout << "\n\n" << elapsed_time << " " << initial_percentage << " " << speed << "\n" << std::endl;

        char command[75];
        sprintf(command, "echo \"%f,%f,,%f\" >> ../%s.csv", elapsed_time, initial_percentage, speed, mode.c_str());
        system(command);
    
        std::cout << "\nemptying buffer" << std::endl;

        ((shared_memory_object*)shared_memory)->priority_boost = children[0];
        buffer.switch_on();
        
        kill(children[0], SIGCONT);
        kill(children[children.size() - 1], SIGCONT);

        buffer.wait_until_empty();
        
        pause_h::sleep(1, 0);

        //kill(children[0], SIGSTOP);
        kill(children[children.size() - 1], SIGSTOP);

        nof_iterations--;
        measurement++;

        if(nof_iterations) {
            std::cout << "\n" << WARMUP_TIME << " seconds cooldown" << std::endl;
            pause_h::sleep(WARMUP_TIME, 0);
        }

        ((shared_memory_object*)shared_memory)->priority_boost = 0;
        for(pid_t p : children) {
            kill(p, SIGCONT);

            if(errno) {perror("");}
        }
    }

    //Uccide tutti i processi figli
    for(pid_t p : children) {
        kill(p, SIGKILL);
    }

    munlockall();

    //Dealloca l'area di memoria condivisa
    shm_unlink("buffer");

    printf("\n");

    return 0;
}