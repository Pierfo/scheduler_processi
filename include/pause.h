#ifndef PAUSE
#define PAUSE
#include <time.h>

namespace pause_h {
    //Sospende l'esecuzione del processo invocante per "seconds" secondi e "nanoseconds" nanosecondi (ad esempio 
    //pause_h::sleep(2, 500000000) sospende l'esecuzione per 2 secondi e 500000000 nanosecondi). "nanoseconds" deve 
    //appartenere all'intervallo [0, 999999999], altrimenti la funzione fallisce silenziosamente
    int sleep(int seconds, int nanoseconds) {        
        //struct timespec è una struttura utile a rappresentare intervalli temporali in Unix. Essa è dotata dei campi
        //tv_sec e tv_nsec
        struct timespec t;
        t.tv_sec = seconds;
        t.tv_nsec = nanoseconds;

        return nanosleep(&t, NULL);
    }
}

#endif