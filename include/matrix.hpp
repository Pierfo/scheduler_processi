#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <iostream>
#include <unistd.h>
#include <string>
#include <sstream>

//Costruttore di default, costruisce una matrice con tutti i termini nulli
template<int N>
matrix<N>::matrix() {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            mat[i][j] = 0;
        }
    }
}

//Costruisce la matrice a partire da un array bidimensionale
template<int N>
matrix<N>::matrix(const double m[N][N]) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            mat[i][j] = m[i][j];
        }
    }
}

//Costruttore di copia
template<int N>
matrix<N>::matrix(const matrix<N>& m) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            mat[i][j] = m.mat[i][j];
        }
    }
}

//Overriding dell'assegnamento di copia
template<int N>
matrix<N>& matrix<N>::operator=(const matrix<N>& m) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            mat[i][j] = m.mat[i][j];
        }
    }

    return *this;
}

//Calcola il determinante
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


//Determina la matrice inversa, lancia errore se la matrice non è invertibile
template<int N>
matrix<N> matrix<N>::invert() const {
    double det = determinant();
    
    if(det == 0) {
        throw std::logic_error{"Matrice non invertibile"};
    }

    matrix<N> inverse {};

    double** m_tr = (double**)malloc((N-1)*sizeof(double*));
    
    for(int i = 0; i < N-1; i++) {
        m_tr[i] = (double*)malloc((N-1)*sizeof(double));
    }

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            remove_row_and_column(m_tr, NULL, i, j, N);

            inverse.at(i, j) = ((i + j) % 2 == 0 ? 1 : -1) * calculate_determinant(m_tr, N-1) / det;
        }
    }

    for(int k = 0; k < N-1; k++) {
        free(m_tr[k]);
    }

    free(m_tr);

    return inverse.transpose();
}

//Restituisce il riferimento all'elemento in posizione (i, j), lancia errore se i o j sono fuori dai confini della matrice
template<int N>
double& matrix<N>::at(int i, int j) {
    if(i < 0 || i >= N || j < 0 || j >= N) {
        throw std::logic_error{"Out of bounds"};
    }

    return mat[i][j];
}

//Determina la matrice trasposta
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

//Utility function per il calcolo del determinante. Calcola ricorsivamente il determinante utilizzando la formula di Laplace
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
        remove_row_and_column(m_tr, m, i, 0, l);
        
        det += (i % 2 == 0 ? 1 : -1) * m[i][0] * calculate_determinant(m_tr, l-1);
    }
    
    for(int i = 0; i < l-1; i++) {
        free(m_tr[i]);
    }

    free(m_tr);

    return det;
}

//Utility function che rimuove la riga i e la colonna j dalla matrice source. m punta all'area di memoria dove si salverà
//la matrice risultante. l è la dimensione di source
template<int N>
void matrix<N>::remove_row_and_column(double** m, double** source, int i, int j, int l) const {
    int row_index = 0;
    
    for(int a = 0; a < l; a++) {
        int column_index = 0;
        
        if(a == i) continue;
                    
        for(int b = 0; b < l; b++) {
            if(b == j) continue;

            m[row_index][column_index] = (source != NULL ? source[a][b] : mat[a][b]);
            
            column_index++;
        }
        
        row_index++;
    }
}

//Stampa la matrice
template<int N>
std::ostream& operator<< (std::ostream& o, matrix<N> m) {
    std::string elements[N*N];

    //Converte ciascun termine della matrice in stringa
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            std::stringstream s {};
            s << m.at(i, j) << " ";
            elements[i*N + j] = s.str();
        }
    }

    //Trova il valore massimo di lunghezza e fa sì che tutte le stringhe abbiano tale lunghezza 
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
        o << "| ";
        for(int j = 0; j < N; j++) {
            o << " " << elements[i*N + j];
        }

        o << " |" << std::endl;
    }

    return o;
}

#endif