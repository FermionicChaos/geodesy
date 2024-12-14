#pragma once
#ifndef GEODESY_CORE_MATH_VEC_H
#define GEODESY_CORE_MATH_VEC_H

#include "config.h"
#include "complex.h"
#include "quaternion.h"

namespace geodesy::core::math {

	template <typename T, std::size_t N>
	class vec : public std::array<T, N>{
	public:

		// -------------------- Vector Space Operations -------------------- //

		vec<T, N> operator-() const {
			vec<T, N> Out;
			for (std::size_t i = 0; i < N; i++) {
				Out[i] = -(*this)[i];
			}
			return Out;
		}

		vec<T, N> operator+(const vec<T, N>& aRhs) const {
			vec<T, N> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] + aRhs[i];
			}
			return Out;
		}

		vec<T, N> operator-(const vec<T, N>& aRhs) const {
			vec<T, N> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] - aRhs[i];
			}
			return Out;
		}

		vec<T, N> operator*(const T& aRhs) const {
			vec<T, N> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] * aRhs;
			}
			return Out;
		}

		vec<T, N> operator/(const T& aRhs) const {
			vec<T, N> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] / aRhs;
			}
			return Out;
		}

		vec<T, N>& operator+=(const vec<T, N>& aRhs) {
			(*this) = (*this) + aRhs;
			return *this;
		}

		vec<T, N>& operator-=(const vec<T, N>& aRhs) {
			(*this) = (*this) - aRhs;
			return *this;
		}

		vec<T, N>& operator*=(const T& aRhs) {
			(*this) = (*this) * aRhs;
			return *this;
		}

		vec<T, N>& operator/=(const T& aRhs) {
			(*this) = (*this) / aRhs;
			return *this;
		}

		bool operator==(const vec<T, N>& aRhs) const {
			bool Out = true;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out &= ((*this)[i] == aRhs[i]);
			}
			return Out;
		}

		bool operator>(const vec<T, N>& aRhs) const {
			bool Out = true;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out &= ((*this)[i] > aRhs[i]);
			}
			return Out;
		}

		bool operator<(const vec<T, N>& aRhs) const {
			bool Out = true;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out &= ((*this)[i] < aRhs[i]);
			}
			return Out;
		}

		bool operator>=(const vec<T, N>& aRhs) const {
			bool Out = true;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out &= ((*this)[i] >= aRhs[i]);
			}
			return Out;
		}

		bool operator<=(const vec<T, N>& aRhs) const {
			bool Out = true;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out &= ((*this)[i] <= aRhs[i]);
			}
			return Out;
		}

		// -------------------- Specific Operations -------------------- //		

		using std::array<T, N>::array;

		vec() {
			for (std::size_t i = 0; i < this->size(); i++) {
				(*this)[i] = T();
			}
		}

		// Variadic template constructor to fill the vector
    	template<typename... args, typename = std::enable_if_t<sizeof...(args) == N>>
    	vec(args... aArgs) : std::array<T, N>{static_cast<T>(aArgs)...} {}

		vec(std::initializer_list<T> aList) {
			if (aList.size() != N) {
				throw std::invalid_argument("Initializer list size does not match vector size.");
			}
			std::copy(aList.begin(), aList.end(), this->begin());
		}

		// Dot product
		T operator*(const vec& aRhs) const {
			T Out = T();
			for (std::size_t i = 0; i < this->size(); i++) {
				Out += (*this)[i] * aRhs[i];
			}
			return Out;
		}

	};

	template<typename T, std::size_t N> inline
	vec<T, N> operator*(const T& aLhs, const vec<T, N>& aRhs) {
		return aRhs * aLhs;
	}

	template<typename T> inline
	T operator^(const vec<T, 2>& aLhs, const vec<T, 2>& aRhs) {
		return ((aLhs[0] * aRhs[1]) - (aLhs[1] * aRhs[0]));
	}

	template<typename T, std::size_t N> inline
	T length(const vec<T, N>& aVector) {
		return std::sqrt(aVector * aVector);
	}

	template<typename T, std::size_t N> inline
	vec<T, N> normalize(const vec<T, N>& aVector) {
		return aVector / length(aVector);
	}

	template<typename T> inline 
	vec<T, 3> operator^(const vec<T, 3>& aLhs, const vec<T, 3>& aRhs) {
		return vec<T, 3>(
			aLhs[1] * aRhs[2] - aLhs[2] * aRhs[1],
			aLhs[2] * aRhs[0] - aLhs[0] * aRhs[2],
			aLhs[0] * aRhs[1] - aLhs[1] * aRhs[0]
		);
	}

	// Arbitrary rotation of a vector around an axis by an angle.
	template <typename T> inline
	vec<T, 3> rotate(vec<T, 3> aInput, T aAngle, vec<T, 3> aAxis) {
		quaternion<T> r 	= quaternion<T>(0.0, aInput[0], aInput[1], aInput[2]);
		quaternion<T> q 	= exp((aAngle / 2.0) * quaternion<T>(0.0, aAxis[0], aAxis[1], aAxis[2]));
		r = q * r * (~q);
		return vec<T, 3>(r[1], r[2], r[3]);
	}

	template<typename T, std::size_t N> inline
	std::ostream& operator<<(std::ostream& aOutStream, const vec<T, N>& aArg) {
		aOutStream << "{ ";
		for (std::size_t i = 0; i < aArg.size(); i++) {
			aOutStream << aArg[i];
			if (i < aArg.size() - 1) {
				aOutStream << ", ";
			}
		}
		aOutStream << " }";
		return aOutStream;
	}

}

#endif // !GEODESY_CORE_MATH_VEC_H
