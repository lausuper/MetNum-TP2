#include <cmath>
#include <iostream>

#include "Matriz.h"

using namespace std;

Matriz::Matriz(const int filas, const int columnas) {
	_filas = filas;
	_columnas = columnas;
	vectorMatriz = new double[filas * columnas];
}

Matriz::Matriz(Matriz& otra) {
	_filas = otra._filas;
	_columnas = otra._columnas;
	vectorMatriz = new double[_filas * _columnas];
	for(int i = 0; i < _filas * _columnas; i++) vectorMatriz[i] = otra.vectorMatriz[i];
}

Matriz::~Matriz() {
	delete vectorMatriz;
}

Matriz* Matriz::identidad(int n) {
	Matriz *m = new Matriz(n, n);
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			m->elem(i, j) = (i == j ? 1 : 0);
		}
	}
	return m;
}

Matriz* Matriz::submatriz(const int desdeFil, const int hastaFil, const int desdeCol, const int hastaCol) {
	Matriz *submatriz = new Matriz((hastaFil - desdeFil + 1), (hastaCol - desdeCol + 1));
	for(int i = desdeFil; i <= hastaFil; i++) {
		for(int j = desdeCol; j <= hastaCol; j++) {
			submatriz->elem(i - desdeFil, j - desdeCol) = elem(i,j);
		}
	}
	return submatriz;
}

void Matriz::cambiarColumna(Matriz& mat, const int columna) {
	for(int i = 0;i < _filas; i++) {
		elem(i,columna) = mat.elem(i,0);
	}
}

int Matriz::filas() const {
	return _filas;
}

int Matriz::columnas() const {
	return _columnas;
}

void Matriz::transponer(){
	int temp = _filas;
	_filas = _columnas;
	_columnas = temp;
	double *tempMat = new double[_filas * _columnas];
	for(int i = 0; i < _filas ; i++) {
		for(int j = 0; j < _columnas; j++) {
			tempMat[j * _columnas + i] = elem(i,j);
		}
	}
	delete vectorMatriz;
	vectorMatriz = tempMat;
}

double &Matriz::elem(const int fila, const int columna) {
	return vectorMatriz[fila * _columnas + columna];
}

double Matriz::max() {
	double max = elem(0, 0);
	for(int i = 0; i < _filas; i++) {
		for(int j = 0; j < _columnas; j++) {
			if(elem(i ,j) > max) {
				max = elem(i,j);
			}
		}
	}
	return max;
}

double Matriz::min() {
	double min = elem(0, 0);
	for(int i = 0; i < _filas; i++) {
		for(int j = 0; j < _columnas; j++) {
			if(elem(i, j) < min) {
				min = elem(i, j);
			}
		}
	}
	return min;
}

double Matriz::rango() {
	return max() - min();
}

Matriz* Matriz::operator+(Matriz &m) {
	Matriz* suma = new Matriz(*this);
	for(int i = 0; i < _filas; i++) {
		for(int j = 0; j < _columnas; j++) {
			suma->elem(i,j) += m.elem(i,j);
		}
	}
	return suma;
}

Matriz* Matriz::operator*(Matriz &m) {
	Matriz* producto = new Matriz(_filas, m._columnas);
	for(int i = 0; i < _filas; i++) {
		for(int j = 0; j < m._columnas; j++) {
			producto->elem(i, j) = 0;
			for(int k = 0; k < _columnas; k++) {
				producto->elem(i, j) += elem(i, k) * m.elem(k, j);
			}
		}
	}
	return producto;
}

Matriz* Matriz::multiplicarPorInversa(Matriz &M) {
	tuple<Matriz*, Matriz*, Matriz*> plu = M.factorizacionPLU();
	//Hago Ly = Px
	Matriz* Px = (*get<0>(plu)) * (*this);
	Matriz* y = get<1>(plu)->forwardSubstitution(*Px);
	//Hago Uj = y
	Matriz* j = get<2>(plu)->backwardSubstitution(*y);
	delete y;
	delete Px;
	delete get<0>(plu);
	delete get<1>(plu);
	delete get<2>(plu);
	return j;
}

Matriz* Matriz::operator*(double k) {
	Matriz* producto = new Matriz(*this);
	for(int i = 0; i < _filas; i++) {
		for(int j = 0; j < _columnas; j++) {
			producto->elem(i, j) = producto->elem(i, j) * k;
		}
	}
	return producto;
}

tuple <Matriz*, Matriz*, Matriz*> Matriz::factorizacionPLU() {
	Matriz *P = identidad(_filas);
	Matriz *L = identidad(_filas);
	Matriz *U = new Matriz(*this);

	for(int j = 0; j < _columnas - 1; j++) {
		// Intercambio la fila con el máximo absoluto por la actual
		// Tener en cuenta que las columnas también determinan la fila a la cual intecambiar el mayor
		// ya que vamos moviendonos diagonalmente, (j,j) va a tener siempre el maximo

		int jp = U->filaConMayorAbsEnCol(j, j);
		P->intercambiarFilas(jp, j);
		L->intercambiarFilas(jp, j, j);
		U->intercambiarFilas(jp, j);

		for(int i = j + 1; i < _filas; i++) {
			L->elem(i, j) = U->elem(i, j) / U->elem(j, j); // Pongo en L Mij
			U->elem(i, j) = 0; // Pongo en 0 el elemento eliminado
			for(int x = j + 1; x < _columnas; x++) {
				U->elem(i, x) = U->elem(i, x) - L->elem(i, j) * U->elem(j, x);
			}
		}
	}
	return make_tuple(P, L, U);
}

Matriz* Matriz::backwardSubstitution(Matriz &b) {
	Matriz *x = new Matriz(_columnas, 1);

	// Voy de la fila de abajo para arriba
	for(int i = _filas - 1; i >= 0; i--) {
		// Me armo un acumulador del nuevo valor Xi, voy construyendo el X de abajo hacia arriba
		// Utilizo Xi = (Bi - sum(Aij, Xj)/Aii j=i+1 hasta n (cols))
		double valorX = b.elem(i, 0);

		// Hago Xi = (Bi - sum(Aij, Xj)
		// Recorro las columnas de la posición + 1 en que tengo mi incognita
		for(int j = i + 1; j < _columnas; j++) {
			valorX -= elem(i,j) * x->elem(j,0);
		}

		// Xi/Aii para terminar
		x->elem(i,0) = valorX / elem(i, i);
	}
	return x;
}

Matriz* Matriz::forwardSubstitution(Matriz &b) {
	Matriz *x = new Matriz(this->_columnas,1);

	for(int i = 0; i < _filas; i++) {
		// Me armo un acumulador del nuevo valor Xi, voy construyendo el X de arriba hacia abajo
		// Utilizo Xi = (Bi - sum(Aij, Xj)/Aii j=i+1 hasta n (cols))
		double valorX = b.elem(i, 0);

		// Hago Xi = (Bi - sum(Aij, Xj)
		for(int j = 0; j < i; j++) {
			valorX -= elem(i, j) * x->elem(j, 0);
		}

		//Xi/Aii para terminar
		x->elem(i, 0) = valorX / elem(i, i);
	}
	return x;
}

void Matriz::print(){
	for(int i = 0; i < _filas; i++) {
		for(int j = 0; j < _columnas; j++) {
			cout << elem(i,j) << '\t';
		}
		cout << endl;
	}
}

void Matriz::intercambiarFilas(const int i, const int j) {
	intercambiarFilas(i, j, _columnas);
}

void Matriz::intercambiarFilas(const int i, const int j, const int hasta) {
	if(i == j) return;
	for(int x = 0; x < hasta; x++) {
		double elemento = elem(i, x);
		elem(i,x) = elem(j,x);
		elem(j,x) = elemento;
	}
}

int Matriz::filaConMayorAbsEnCol(const int col, const int desde) {
	int filaMayor = desde;
	int mayor = elem(desde, col);
	for(int y = desde + 1; y < _filas; y++) {
		if(abs(elem(y, col)) > mayor) {
			mayor = abs(elem(y, col));
			filaMayor = y;
		}
	}
	return filaMayor;
}
