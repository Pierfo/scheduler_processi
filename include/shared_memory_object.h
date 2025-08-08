#ifndef SHARED_MEMORY_OBJECT
#define SHARED_MEMORY_OBJECT
#include "buffer.h"
#include "matrix.h"
#include "actions.h"

#define MATRIX_SIZE 6
#define BUFFER_SIZE 1001

/*
    struct contenente gli elementi che si vuole condividere con i vari processi. Esso è dotato dei seguenti campi:

        -shared_buffer: il buffer condiviso dai processi
        -action_before_insertion: il function object che definisce l'azione da eseguire per generare l'elemento da inserire nel buffer
        -action_after_extraction: il function object che definisce l'azione da eseguire dopo aver estratto un elemento dal buffer

    Questo struct serve a fornire all'utente la possibilità di personalizzare alcuni aspetti del funzionamento del programma:
    in particolare l'utente può modificare

        -la dimensione del buffer e il tipo di oggetti che può contenere
        -il tipo del campo action_before_insertion, così da poter definire una propria implementazione dell'azione che si
            vuole eseguire per generare un elemento
        -il tipo del campo action_after_extraction, così da poter definire una propria implementazione dell'azione che si 
            vuole eseguire dopo aver estratto un elemento

    Per quanto riguarda gli ultimi due punti, le modifiche devono soddisfare i seguenti vincoli:

        -il tipo della variabile action_before_insertion deve essere un function object in cui l'overriding dell'operatore ()
            è una funzione priva di argomenti e che restituisce un oggetto che può essere inserito nel buffer
        -il tipo della variabile action_after_extraction deve essere un function object in cui l'overriding dell'operatore ()
            è una funzione void che accetta come argomento un oggetto compatibile con gli oggetti contenuti nel buffer

    L'header "actions.h" fornisce un'implementazione di questi due function object
*/
struct shared_memory_object {
    buffer<matrix<MATRIX_SIZE>, BUFFER_SIZE> shared_buffer = buffer<matrix<MATRIX_SIZE>, BUFFER_SIZE>{};
    matrix_action_before_insertion<MATRIX_SIZE> action_before_insertion = matrix_action_before_insertion<MATRIX_SIZE>{};
    matrix_action_after_extraction<MATRIX_SIZE> action_after_extraction = matrix_action_after_extraction<MATRIX_SIZE>{};
};

#endif