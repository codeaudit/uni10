/****************************************************************************
*  @file uni10_tools.h
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
*  @brief Header file for useful string functions and wrapper functions
*  @author Yun-Da Hsieh
*  @date 2014-05-06
*  @since 0.1.0
*
*****************************************************************************/
#ifndef UNI10_TOOLS_H
#define UNI10_TOOLS_H
#include <stdint.h>
#include <limits.h>
#include <string>
#include <assert.h>
#include <vector>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <uni10/data-structure/uni10_struct.h>
namespace uni10{
void* myMalloc(void* ptr, size_t memsize, int& status);
void* myMemcpy(void* des, const void* src, size_t memsize, int des_st, int src_st);
void myFree(void* ptr, size_t memsize, int status);
void membzero(void* ptr, size_t memsize, int status);
void randomNums(double* elem, int N, int status);
std::vector<_Swap> recSwap(int* ord, int n, int* ordF);
std::vector<_Swap> recSwap(int* _ord, int n);	//Given the reshape order out to in. 

// trim from start
static inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}
};	/* namespace uni10 */	

#endif /* UNI10_TOOLS_H */