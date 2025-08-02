#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <string>

template <int N>
class matrix {
    public:
    matrix();
    matrix(const double m[N][N]);
    matrix(const matrix<N>& m);
    matrix(const matrix<N>&& m);
    matrix<N>& operator= (const matrix<N>& m);
    matrix<N> invert() const;
    double determinant() const;
    double& at(int i, int j);
    matrix<N> transpose() const;
    
    private:
    double mat[N][N];

    double calculate_determinant(double** m, int l) const; 
};

template <int N>
std::ostream& operator<< (std::ostream& o, matrix<N> m);

#include "matrix.hpp"

#endif