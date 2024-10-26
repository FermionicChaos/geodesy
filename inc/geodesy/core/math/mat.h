
#pragma once
#ifndef GEODESY_CORE_MATH_MAT_H
#define GEODESY_CORE_MATH_MAT_H

#include "config.h"
#include "vec.h"

namespace geodesy::core::math {

	template <typename T, std::size_t M, std::size_t N>
	class mat : public std::array<T, M*N> {
	public:

		mat<T, M, N> operator+(const mat<T, M, N>& aRhs) const {
			mat<T, M, N> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] + aRhs[i];
			}
			return Out;
		}

		mat<T, M, N> operator-(const mat<T, M, N>& aRhs) const {
			mat<T, M, N> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] - aRhs[i];
			}
			return Out;
		}

		mat<T, M, N> operator*(const T& aRhs) const {
			mat<T, M, N> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] * aRhs;
			}
			return Out;
		}

		mat<T, M, N> operator/(const T& aRhs) const {
			mat<T, M, N> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] / aRhs;
			}
			return Out;
		}

		mat<T, M, N>& operator+=(const mat<T, M, N>& aRhs) {
			(*this) = (*this) + aRhs;
			return *this;
		}

		mat<T, M, N>& operator-=(const mat<T, M, N>& aRhs) {
			(*this) = (*this) - aRhs;
			return *this;
		}

		mat<T, M, N>& operator*=(const T& aRhs) {
			(*this) = (*this) * aRhs;
			return *this;
		}

		mat<T, M, N>& operator/=(const T& aRhs) {
			(*this) = (*this) / aRhs;
			return *this;
		}

		using std::array<T, M*N>::array;

		mat() {
			for (std::size_t i = 0; i < M*N; i++) {
				(*this)[i] = T();
			}
		}

		// Variadic template constructor to fill the vector
    	template<typename... args, typename = std::enable_if_t<sizeof...(args) == M*N>>
    	mat(args... aArgs) : std::array<T, M*N>{static_cast<T>(aArgs)...} {}

		// Accessor functions, default memory interpretation is row-major
		const T operator()(std::size_t aRow, std::size_t aColumn) const {
			return (*this)[aRow * N + aColumn];
		}

		T& operator()(std::size_t aRow, std::size_t aColumn) {
			return (*this)[aRow * N + aColumn];
		}

		// Matrix-vector multiplication
		vec<T, M> operator*(const vec<T, N>& aRhs) const {
			vec<T, M> Out;
			for (std::size_t i = 0; i < M; i++) {
				Out[i] = T();
				for (std::size_t j = 0; j < N; j++) {
					Out[i] += (*this)(i, j) * aRhs[j];
				}
			}
			return Out;
		}

    	// Matrix multiplication function
    	template <std::size_t P>
    	mat<T, M, P> operator*(const mat<T, N, P>& aRhs) const {
    	    mat<T, M, P> Result;
    	    for (std::size_t i = 0; i < M; ++i) {
    	        for (std::size_t j = 0; j < P; ++j) {
    	            for (std::size_t k = 0; k < N; ++k) {
    	                Result(i, j) += (*this)(i, k) * aRhs(k, j);
    	            }
    	        }
    	    }
    	    return Result;
    	}

		mat<T, M-1, N-1> minor(std::size_t aI, std::size_t aJ) const {
			mat<T, M-1, N-1> Result;
			for (std::size_t i = 0; i < M - 1; ++i) {
				for (std::size_t j = 0; j < N - 1; ++j) {
					Result(i, j) = (*this)(i < aI ? i : i + 1, j < aJ ? j : j + 1);
				}
			}
			return Result;
		}

	};

	template<typename T, std::size_t M, std::size_t N> inline
	mat<T, M, N> operator*(const T& aLhs, const mat<T, M, N>& aRhs) {
		return aRhs * aLhs;
	}

	template <typename T, std::size_t M, std::size_t N> inline
	T trace(const mat<T, M, N>& aMatrix) {
		T Result = T();
		for (std::size_t i = 0; i < M; ++i) {
			Result += aMatrix(i, i);
		}
		return Result;
	}

	// BUG: Recursion too complex to generate for compiler, solve another way.
	template <typename T, std::size_t M, std::size_t N> inline
	T determinant(const mat<T, M, N>& aMatrix) {
		T Result = T();
		// Throw runtime error if M != N
		if (M != N) {
			throw std::runtime_error("Matrix must be square to calculate determinant.");
		}
		// Calculate determinant recursively
		if (M == 1) {
			// Base case
			Result = aMatrix(0, 0);
		}
		else {
			// TODO: Optimize by searching for Rows/Columns with most zeros, and expanding along those.
			for (std::size_t i = 0; i < M; ++i) {
				T Value = aMatrix(i, 0);
				if (Value != T()) {
					// Only Calculate determinant if the value is not zero.
					Result += ((i + 0) % 2 == 0 ? 1 : -1) * Value * determinant(aMatrix.minor(i, 0));
				}
			}
		}
		return Result;
	}

	template <typename T, std::size_t M, std::size_t N> inline
	std::ostream& operator<<(std::ostream& aOutStream, const mat<T, M, N>& aMatrix) {
		aOutStream << std::endl;
		for (std::size_t i = 0; i < M; i++) {
			aOutStream << "{ ";
			for (std::size_t j = 0; j < N; j++) {
				aOutStream << aMatrix(i, j);
				if (j < N - 1) {
					aOutStream << ", ";
				}
			}
			aOutStream << " }" << std::endl;
		}
		return aOutStream;
	}

}

#endif // !GEODESY_CORE_MATH_MAT_H
