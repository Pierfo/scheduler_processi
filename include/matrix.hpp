#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <iostream>
#include <unistd.h>
#include <string>
#include <sstream>
#include <vector>

template<int N>
matrix<N>::matrix() {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            mat[i][j] = 0;
        }
    }
}

template<int N>
matrix<N>::matrix(const double m[N][N]) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            mat[i][j] = m[i][j];
        }
    }
}

template<int N>
matrix<N>::matrix(const matrix<N>& m) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            mat[i][j] = m.mat[i][j];
        }
    }
}

template<int N>
matrix<N>& matrix<N>::operator=(const matrix<N>& m) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            mat[i][j] = m.mat[i][j];
        }
    }

    return *this;
}

template<int N>
double matrix<N>::determinant() const {
    double** m = (double**)malloc(N*sizeof(double*));

    for(int i = 0; i < N; i++) {
        m[i] = (double*)malloc(N*sizeof(double));
    
        for(int j = 0; j < N; j++) {
            m[i][j] = mat[i][j];
        }
    }
    
    double det = calculate_determinant(m, N);

    for(int i = 0; i < N; i++) {
        free(m[i]);
    }

    free(m);

    return det;
}

template<int N>
matrix<N> matrix<N>::invert() const {
    matrix<N> inverse {};

    double det = determinant();

    if(det == 0) {
        throw std::logic_error{"Matrice non invertibile"};
    }

    double** m_tr = (double**)malloc((N-1)*sizeof(double*));
    
    for(int i = 0; i < N-1; i++) {
        m_tr[i] = (double*)malloc((N-1)*sizeof(double));
    }

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            int row_index = 0;
            
            for(int a = 0; a < N; a++) {
                int column_index = 0;
                
                if(a == i) continue;
                    
                for(int b = 0; b < N; b++) {
                    if(b == j) continue;
                    
                    m_tr[row_index][column_index] = mat[a][b];

                    column_index++;
                }

                    
                row_index++;
            }

            inverse.at(i, j) = ((i + j) % 2 == 0 ? 1 : -1) * calculate_determinant(m_tr, N-1) / det;
        }
    }

    for(int k = 0; k < N-1; k++) {
        free(m_tr[k]);
    }

    free(m_tr);

    return inverse.transpose();
}

template<int N>
double matrix<N>::calculate_determinant(double** m, int l) const {    
    if(l == 1) {
        return m[0][0];
    }

    double det = 0;
    
    double** m_tr = (double**)malloc((l-1)*sizeof(double*));
    
    for(int i = 0; i < l-1; i++) {
        m_tr[i] = (double*)malloc((l-1)*sizeof(double));
    }
    
    for(int i = 0; i < l; i++) {
        int row_index = 0;
        
        for(int a = 0; a < l; a++) {
            if(a == i) continue;
            
            for(int j = 1; j < l; j++) {
                m_tr[row_index][j-1] = m[a][j];
            }
            
            row_index++;
        }
        
        det += (i % 2 == 0 ? 1 : -1) * m[i][0] * calculate_determinant(m_tr, l-1);
    }
    
    for(int i = 0; i < l-1; i++) {
        free(m_tr[i]);
    }

    free(m_tr);

    return det;
}

template<int N>
double& matrix<N>::at(int i, int j) {
    if(i < 0 || i >= N || j < 0 || j >= N) {
        throw std::logic_error{"Out of bounds"};
    }

    return mat[i][j];
}

template<int N>
matrix<N> matrix<N>::transpose() const {
    double tr[N][N];

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            tr[i][j] = mat[j][i];
        }
    }

    matrix<N> trm {tr};

    return trm;
}

template<int N>
std::ostream& operator<< (std::ostream& o, matrix<N> m) {
    std::vector<std::string> elements {};

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            std::stringstream s {};
            s << m.at(i, j) << " ";
            elements.push_back(s.str());
        }
    }

    int max = 0;

    for(std::string s : elements) {
        if(s.size() > max) {
            max = s.size();
        }
    }

    for(std::string& s : elements) {
        while(s.size() < max) {
            s += std::string{" "};
        }
    }
    
    o << std::endl;
    for(int i = 0; i < N; i++) {
        o << "|";
        for(int j = 0; j < N - 1; j++) {
            o << elements[i*N + j];
        }

        o << elements[(i+1)*N - 1] << "|" << std::endl;
    }

    return o;
}

#endif