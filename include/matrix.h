#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>

template <int N>
/*
    Classe che definisce una matrice quadrata di dimensione N
*/
class matrix {
    public:
    matrix(); //Costruttore di default
    matrix(const double m[N][N]); //Costruisce la matrice a partire da un array bidimensionale
    matrix(const matrix<N>& m); //Costruttore di copia
    matrix<N>& operator= (const matrix<N>& m); //Overriding dell'assegnamento
    double determinant() const; //Calcola il determinante
    matrix<N> invert() const; //Determina la matrice inversa
    double& at(int i, int j); //Restituisce il riferimento all'elemento in posizione (i, j)
    matrix<N> transpose() const; //Determina la matrice trasposta
    
    private:
    double mat[N][N]; //La matrice NxN

    double calculate_determinant(double** m, int l) const; //Utility function per il calcolo del determinante
    void remove_row_and_column(double** m, double** source, int i, int j, int l) const; //Utility function che rimuove la riga i e la colonna j dalla matrice source
};

template <int N>
std::ostream& operator<< (std::ostream& o, matrix<N> m); //Stampa la matrice

#include "matrix.hpp"

#endif