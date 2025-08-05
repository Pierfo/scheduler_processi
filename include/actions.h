#ifndef ACTIONS_H
#define ACTIONS_H

#include "matrix.h"
#include <iostream>

template<int N>
class matrix_action_before_insertion {
    public:
    matrix_action_before_insertion() {};
    matrix<N> operator() () {
        matrix<N> mat;

        for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
                mat.at(i, j) = rand() % 1001;
            }
        }

        return mat.invert();
    };
};

template<int N>
class matrix_action_after_extraction {
    public:
    matrix_action_after_extraction() {};
    void operator() (matrix<N> mat) {mat.invert().determinant();};
};

#endif