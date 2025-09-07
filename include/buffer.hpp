#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <pthread.h>
#include <string.h>
#include <iostream>

//Inizializza un buffer vuoto
template<typename T, int N>
buffer<T, N>::buffer()
    : start{0}, end{0}, change_happened{true} {
    //Inizializza monitor_attr
    pthread_mutexattr_init(&monitor_attr);
    //Fa sì che il mutex definito con monitor_attr possa essere acceduto da più processi
    pthread_mutexattr_setpshared(&monitor_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&monitor_attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutexattr_setprotocol(&monitor_attr, PTHREAD_PRIO_INHERIT);
    //Inizializza il mutex buffer_access
    pthread_mutex_init(&buffer_access, &monitor_attr);

    //Inizializza cond_attr
    pthread_condattr_init(&cond_attr);
    //Fa sì che le variabili condition definite con cond_attr possano essere accedute da più processi
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    //Inizializza le variabili condition
    pthread_cond_init(&buffer_empty, &cond_attr);
    pthread_cond_init(&buffer_full, &cond_attr);
    pthread_cond_init(&no_change_happened, &cond_attr);
    pthread_cond_init(&buffer_not_full, &cond_attr);
    pthread_cond_init(&theres_still_someone, &cond_attr);
    pthread_cond_init(&buffer_not_empty, &cond_attr);

    explicit_bzero(buf, N);
    process_accessing = 0;

    is_on = true;
}

//Inserisce un elemento alla fine del buffer
template<typename T, int N>
void buffer<T, N>::insert(T elem) {
    ////std::cout << "start inserting" << std::endl;
    //Ottiene il mutex
    pthread_mutex_lock(&buffer_access);

    process_accessing++;

    bool was_empty = is_empty();
    //Se il buffer è pieno allora mette il processo nella coda di attesa relativa a buffer_full
    while(is_full() || !is_on) {
        if(!is_on) {
            pthread_mutex_unlock(&buffer_access);

            return;
        }

        pthread_cond_signal(&buffer_not_full);
        pthread_cond_wait(&buffer_full, &buffer_access);
    }
    
    //Inserisce l'elemento nel buffer
    buf[end] = elem;
    end = increment(end);

    //Risveglia il processo di monitoraggio della percentuale di riempimento del buffer, se quest'ultimo è in attesa
    if(!change_happened) {
        change_happened = true;
        pthread_cond_signal(&no_change_happened);
    }
    
    //Se il buffer era vuoto prima dell'inserimento allora risveglia un processo nella coda di attesa relativa a 
    //buffer_empty, se presente
    if(was_empty) {
        pthread_cond_signal(&buffer_empty);
    }

    process_accessing--;
    pthread_cond_signal(&theres_still_someone);
    
    //Libera il mutex
    pthread_mutex_unlock(&buffer_access);

    ////std::cout << "done inserting" << std::endl;

}

//Estrae un elemento dall'inizio del buffer
template<typename T, int N>
T buffer<T, N>::extract() {
    ////std::cout << "\tstart extracting" << std::endl;
    //Ottiene il mutex
    pthread_mutex_lock(&buffer_access);

    process_accessing++;

    bool was_full = is_full();
    //Se il buffer è vuoto allora mette il processo nella coda di attesa relativa a buffer_empty
    while(is_empty() || !is_on) {
        if(!is_on) {
            pthread_mutex_unlock(&buffer_access);

            return T{};
        }

        pthread_cond_signal(&buffer_not_empty);
        pthread_cond_wait(&buffer_empty, &buffer_access);
    }

    //Estrae l'elemento dal buffer
    T elem = buf[start];
    start = increment(start);
    
    //Risveglia il processo di monitoraggio della percentuale di riempimento del buffer, se quest'ultimo è in attesa
    if(!change_happened) {
        change_happened = true;
        pthread_cond_signal(&no_change_happened);
    }

    //Se il buffer era pieno prima dell'estrazione allora risveglia un processo nella coda di attesa relativa a 
    //buffer_full, se presente
    if(was_full) {
        pthread_cond_signal(&buffer_full);
    }

    process_accessing--;
    pthread_cond_signal(&theres_still_someone);

    //Libera il mutex
    pthread_mutex_unlock(&buffer_access);
    ////std::cout << "\tdone extracting" << std::endl;
    return elem;
}

//Calcola la percentuale di riempimento del buffer
template<typename T, int N>
double buffer<T, N>::calculate_fill_percentage() {
    //std::cout << "\t\tstart calculating" << std::endl;
    //Ottiene il mutex
    pthread_mutex_lock(&buffer_access);

    process_accessing++;
    
    //Se la percentuale di riempimento non è variata dalla sua ultima lettura, aspetta che si verifichi un inserimento o estrazione
    /*while(!change_happened || !is_on) {
        if(!is_on) {
            pthread_mutex_unlock(&buffer_access);

            return;
        }
        pthread_cond_wait(&no_change_happened, &buffer_access);
    }*/

    int nof_elements = 0;

    for(int i = start; i != end; i = increment(i)) {
        nof_elements++;
    }

    double nof_elements_double = nof_elements * 1.0;

    change_happened = false;

    process_accessing--;
    pthread_cond_signal(&theres_still_someone);

    //Libera il mutex
    pthread_mutex_unlock(&buffer_access);
    //std::cout << "\t\tdone calculating" << std::endl;
    return nof_elements_double / (N - 1);
}

template<typename T, int N>
void buffer<T, N>::wait_until_full() {
    //std::cout << "\t\t\tstart waiting" << std::endl;
    pthread_mutex_lock(&buffer_access);

    process_accessing++;

    while(!is_full() || !is_on) {
        if(!is_on) {
            pthread_mutex_unlock(&buffer_access);

            return;
        }
        
        pthread_cond_wait(&buffer_not_full, &buffer_access);
    }

    process_accessing--;
    pthread_cond_signal(&theres_still_someone);

    pthread_mutex_unlock(&buffer_access);
    //std::cout << "\t\t\tdone waiting" << std::endl;
}

template<typename T, int N>
void buffer<T, N>::wait_until_empty() {
    //std::cout << "\t\t\tstart waiting" << std::endl;
    pthread_mutex_lock(&buffer_access);

    process_accessing++;

    while(!is_empty() || !is_on) {
        if(!is_on) {
            pthread_mutex_unlock(&buffer_access);

            return;
        }
        
        pthread_cond_wait(&buffer_not_empty, &buffer_access);
    }

    process_accessing--;
    pthread_cond_signal(&theres_still_someone);

    pthread_mutex_unlock(&buffer_access);
    //std::cout << "\t\t\tdone waiting" << std::endl;
}

template<typename T, int N>
void buffer<T, N>::empty_out() {
    //std::cout << "\t\t\t\tstart emptying" << std::endl;
    pthread_mutex_lock(&buffer_access);

    while(process_accessing != 0 || !is_on) {
        if(!is_on) {
            pthread_mutex_unlock(&buffer_access);

            return;
        }
        //std::cout << "there are still " << process_accessing << "processes" << std::endl;
        pthread_cond_wait(&theres_still_someone, &buffer_access);
    }

    //std::cout << "there are still " << process_accessing << "processes" << std::endl;

    process_accessing++;

    while(!is_empty()) {
        extract();
    }

    process_accessing--;

    pthread_mutex_unlock(&buffer_access);

    //std::cout << "\t\t\t\tdone emptying" << std::endl;
}

template<typename T, int N>
void buffer<T, N>::switch_off() {
    //std::cout << "\t\t\t\t\tstart switching off" << std::endl;
    pthread_mutex_lock(&buffer_access);
    
    is_on = false;

    pthread_cond_broadcast(&buffer_empty);
    pthread_cond_broadcast(&buffer_full);
    pthread_cond_broadcast(&buffer_not_full);
    pthread_cond_broadcast(&theres_still_someone);
    pthread_cond_broadcast(&no_change_happened);

    pthread_mutex_unlock(&buffer_access);
    //std::cout << "\t\t\t\t\tdone switching off" << std::endl;
}

template<typename T, int N>
void buffer<T, N>::switch_on() {
    is_on = true;
}

//Distrugge il buffer
template<typename T, int N>
buffer<T, N>::~buffer() {
    switch_off();
    //Distrugge il mutex e il relativo attributo
    pthread_mutex_destroy(&buffer_access);
    pthread_mutexattr_destroy(&monitor_attr);

    //Distrugge tutte le variabili condition e il relativo attributo
    pthread_cond_destroy(&buffer_empty);
    pthread_cond_destroy(&buffer_full);
    pthread_cond_destroy(&no_change_happened);
    pthread_cond_destroy(&buffer_not_full);
    pthread_cond_destroy(&theres_still_someone);
    pthread_cond_destroy(&buffer_not_empty);
    pthread_condattr_destroy(&cond_attr);
}

#endif