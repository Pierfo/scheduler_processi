#ifndef ACTIONS_H
#define ACTIONS_H

#include "matrix.h"

/*
    Function object che definisce la funzione che dev'essere eseguita prima di inserire un elemento nel buffer
*/
template<int N>
class matrix_action_before_insertion {
    public:
    matrix_action_before_insertion() {}; //Costruttore di default
    matrix<N> operator() () { //Overriding dell'operatore ()
        matrix<N> mat;

        for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
                mat.at(i, j) = rand() % 1001;
            }
        }

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
    void operator() (matrix<N> mat) {mat.invert().determinant();}; //Overriding del'operatore ()
};

#endif