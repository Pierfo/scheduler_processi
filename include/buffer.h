#ifndef BUFFER
#define BUFFER

//T è il tipo di dato che può essere inserito nel buffer mentre N è la sua capienza massima
template<typename T, int N>
/*
    Classe che gestisce un buffer circolare. Solo un processo per volta può accedere al buffer.
    
*/
class buffer {
    public:
    buffer();
    void insert(T elem); //Inserisce un elemento alla fine del buffer
    T extract(); //Estrae un elemento dall'inizio del buffer
    double calculate_fill_percentage(); //Calcola la percentuale di riempimento del buffer
    ~buffer();
    
    private:
    int start; //La posizione dell'elemento che sarà restituito dalla prossima chiamata a extract()
    int end; //La posizione in cui sarà inserito l'argomento della prossima chiamata a insert()
    T buf[N]; //Il buffer
    bool change_happened; //Monitora se la percentuale di riempimento ha subito delle modifiche dall'ultima sua lettura
    pthread_mutexattr_t monitor_attr; //Attributo che serve a definire il mutex buffer_access
    pthread_mutex_t buffer_access; //Mutex che garantisce l'accesso al buffer a solo un processo alla volta
    pthread_cond_t buffer_empty; //Variabile condition per quando il buffer è vuoto
    pthread_cond_t buffer_full; //Variabile condition per quando il buffer è pieno
    pthread_condattr_t cond_attr; //Attributo che serve a definire le varie variabili condition
    pthread_cond_t no_change_happened; //Variabile condition per quando la percentuale di riempimento non ha subito modifiche dall'ultima sua lettura
    
    int increment(int v) {return (v + 1) % N;}; //Incrementa di un'unità gli indici start o end
};

#include "buffer.hpp"

#endif