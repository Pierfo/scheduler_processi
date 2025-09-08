#ifndef BUFFER
#define BUFFER

#include <pthread.h>

template<typename T, int N>
/*
    Classe che gestisce un buffer circolare contenente oggetti di tipo T e di dimensione pari a N; il buffer può contenere 
    fino a un massimo di N-1 elementi. 
    Solo un processo per volta può accedere al buffer.
*/
class buffer {
    public:
    buffer(); //Inizializza un buffer vuoto
    void insert(T elem); //Inserisce un elemento alla fine del buffer
    T extract(); //Estrae un elemento dall'inizio del buffer
    double calculate_fill_percentage(); //Calcola la percentuale di riempimento del buffer
    int size() {return (N - 1);}; //Restituisce la dimensione del buffer
    void wait_until_full();
    void wait_until_empty();
    void empty_out();
    void switch_off();
    void switch_on();
    double full_since = 0;
    ~buffer(); //Distrugge il buffer
    
    private:
    int start; //La posizione dell'elemento che sarà restituito dalla prossima chiamata a extract()
    int end; //La posizione in cui sarà inserito l'argomento della prossima chiamata a insert()
    int process_accessing;
    bool is_on;
    T buf[N]; //Il buffer
    bool change_happened; //Monitora se la percentuale di riempimento ha subito delle modifiche dall'ultima sua lettura
    pthread_mutexattr_t monitor_attr; //Attributo che serve a definire il mutex buffer_access
    pthread_mutex_t buffer_access; //Mutex che garantisce l'accesso al buffer a solo un processo alla volta
    pthread_cond_t buffer_empty; //Variabile condition per quando il buffer è vuoto
    pthread_cond_t buffer_full; //Variabile condition per quando il buffer è pieno
    pthread_cond_t buffer_not_full;
    pthread_cond_t buffer_not_empty;
    pthread_cond_t theres_still_someone;
    pthread_cond_t no_change_happened; //Variabile condition per quando la percentuale di riempimento non ha subito modifiche dall'ultima sua lettura
    pthread_condattr_t cond_attr; //Attributo che serve a definire le variabili condition
    
    int increment(int v) {return (v + 1) % N;}; //Incrementa di un'unità in maniera circolare
    bool is_empty() {return start == end;} //Verifica se il buffer è vuoto
    bool is_full() {return start == increment(end);} //Verifica se il buffer è pieno
};

#include "buffer.hpp"

#endif