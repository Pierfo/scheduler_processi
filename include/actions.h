#ifndef ACTIONS_H
#define ACTIONS_H

#include "matrix.h"
#include "pause.h"
#include <fcntl.h>
#include <errno.h>

/*
    Function object che definisce la funzione che dev'essere eseguita per generare un nuovo elemento da inserire nel buffer
*/
template<int N>
class matrix_action_before_insertion {
    public:
    matrix_action_before_insertion() {}; //Costruttore di default
    matrix<N> operator() () /*Overriding dell'operatore (): genera una matrice e, se è invertibile, costruisce la sua inversa*/ { 
        matrix<N> mat;
        double det = 0;

        while(det == 0) {
            for(int i = 0; i < N; i++) {
                for(int j = 0; j < N; j++) {
                    mat.at(i, j) = rand() % 1001;
                }
            }

            det = mat.determinant();
        }        

        if(errno) perror("");
        return mat.invert();
    };
};

/*
    Function object che definisce la funzione che dev'essere eseguita dopo aver estratto un elemento dal buffer
*/
template<int N>
class matrix_action_after_extraction {
    public:
    matrix_action_after_extraction() {}; //Costruttore di default
    void operator() (matrix<N> mat) {
        mat.invert();
    }; //Overriding del'operatore (): costruisce la matrice inversa
};

#endif