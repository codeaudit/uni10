/****************************************************************************
*  @file Matrix.h
*  @license
*    Universal Tensor Network Library
*    Copyright (c) 2013-2014
*    Yun-Da Hsieh, Pochung Chen and Ying-Jer Kao 
*
*    This file is part of Uni10, the Universal Tensor Network Library.
*
*    Uni10 is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    Uni10 is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with Uni10.  If not, see <http://www.gnu.org/licenses/>.
*  @endlicense
*  @brief Header file for Matrix class
*  @author Yun-Da Hsieh
*  @date 2014-05-06
*  @since 0.1.0
*
*****************************************************************************/
#ifndef MATRIX_H
#define MATRIX_H
#include <iostream>
#include <iomanip>
#include <vector>
#include <assert.h>
#include <string.h>
#include <math.h>
//Type of Matrix
//#include <uni10/tensor-network/UniTensor.h>
namespace uni10{

class Matrix {
	public:
		Matrix(int _Rnum, int _Cnum, bool _diag=false);
		Matrix(int _Rnum, int _Cnum, double* _elem, bool _diag=false);
		Matrix(const Matrix& _m);
		~Matrix();
		int row()const;
		int col()const;
		bool isDiag()const{return diag;};
		size_t elemNum()const;
		Matrix& operator=(const Matrix& _m);
		friend Matrix operator* (const Matrix& Ma, const Matrix& Mb);
		Matrix& operator*= (const Matrix& Mb);
		friend std::ostream& operator<< (std::ostream& os, const Matrix& b);
		std::vector<Matrix> diagonalize();
		std::vector<Matrix> svd();
		void addElem(double* elem);
		void randomize();
		void orthoRand();
		void set_zero();
		void transpose();
		double trace();
		void save(const std::string& fname);
		void load(const std::string& fname);
		friend Matrix operator*(const Matrix& Ma, double a);
		friend Matrix operator*(double a, const Matrix& Ma){return Ma * a;};
		friend bool operator== (const Matrix& m1, const Matrix& m2);
		Matrix& operator*= (double a);
		friend Matrix operator+(const Matrix& Ma, const Matrix& Mb);
		Matrix& operator+= (const Matrix& Mb);
		double& operator[](size_t idx);
		double* elem()const;
		double& at(int i, int j);
	private:
		int Rnum;		//number of rows of the block
		int Cnum;		//number of columns of the block
		double* m_elem;
		size_t m_elemNum;
		bool diag;
};

};	/* namespace uni10 */	
#endif /* MATRIX_H */