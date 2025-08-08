#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <pthread.h>

//Inizializza un buffer vuoto
template<typename T, int N>
buffer<T, N>::buffer()
    : start{0}, end{0}, change_happened{true} {
    //Inizializza monitor_attr
    pthread_mutexattr_init(&monitor_attr);
    //Fa sì che il mutex definito con monitor_attr possa essere acceduto da più processi
    pthread_mutexattr_setpshared(&monitor_attr, PTHREAD_PROCESS_SHARED);
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
}

//Inserisce un elemento alla fine del buffer
template<typename T, int N>
void buffer<T, N>::insert(T elem) {
    //Ottiene il mutex
    pthread_mutex_lock(&buffer_access);

    bool was_empty = is_empty();
    //Se il buffer è pieno allora mette il processo nella coda di attesa relativa a buffer_full
    while(is_full()) {
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
    
    //Libera il mutex
    pthread_mutex_unlock(&buffer_access);
}

//Estrae un elemento dall'inizio del buffer
template<typename T, int N>
T buffer<T, N>::extract() {
    //Ottiene il mutex
    pthread_mutex_lock(&buffer_access);

    bool was_full = is_full();
    //Se il buffer è vuoto allora mette il processo nella coda di attesa relativa a buffer_empty
    while(is_empty()) {
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

    //Libera il mutex
    pthread_mutex_unlock(&buffer_access);
    return elem;
}

//Calcola la percentuale di riempimento del buffer
template<typename T, int N>
double buffer<T, N>::calculate_fill_percentage() {
    //Ottiene il mutex
    pthread_mutex_lock(&buffer_access);
    
    //Se la percentuale di riempimento non è variata dalla sua ultima lettura, aspetta che si verifichi un inserimente o estrazione
    while(!change_happened) {
        pthread_cond_wait(&no_change_happened, &buffer_access);
    }

    //Calcola il numero di elementi salvati e lo coonverte in double
    int nof_elements = 0;

    for(int i = start; i != end; i = increment(i)) {
        nof_elements++;
    }

    double nof_elements_double = nof_elements * 1.0;

    change_happened = false;

    //Libera il mutex
    pthread_mutex_unlock(&buffer_access);

    //Calcola la percentuale di riempimento
    return nof_elements_double / (N - 1);
}

//Distrugge il buffer
template<typename T, int N>
buffer<T, N>::~buffer() {
    //Distrugge il mutex e il relativo attributo
    pthread_mutex_destroy(&buffer_access);
    pthread_mutexattr_destroy(&monitor_attr);

    //Distrugge tutte le variabili condition e il relativo attributo
    pthread_cond_destroy(&buffer_empty);
    pthread_cond_destroy(&buffer_full);
    pthread_cond_destroy(&no_change_happened);
    pthread_condattr_destroy(&cond_attr);
}

#endif