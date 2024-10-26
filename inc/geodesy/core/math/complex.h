#pragma once
#ifndef GEODESY_CORE_MATH_COMPLEX_H
#define GEODESY_CORE_MATH_COMPLEX_H

#include "config.h"
#include "type.h"
#include "constants.h"
#include "vec.h"

namespace geodesy::core::math {

	template <typename T>
	class complex : public std::array<T, 2> {
	public:

		complex<T> operator+(const complex<T>& aRhs) const {
			complex<T> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] + aRhs[i];
			}
			return Out;
		}

		complex<T> operator-(const complex<T>& aRhs) const {
			complex<T> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] - aRhs[i];
			}
			return Out;
		}

		complex<T> operator*(const T& aRhs) const {
			complex<T> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] * aRhs;
			}
			return Out;
		}

		complex<T> operator/(const T& aRhs) const {
			complex<T> Out;
			for (std::size_t i = 0; i < 2; i++) {
				Out[i] = (*this)[i] / aRhs;
			}
			return Out;
		}

		complex<T>& operator+=(const complex<T>& aRhs) {
			(*this) = (*this) + aRhs;
			return *this;
		}

		complex<T>& operator-=(const complex<T>& aRhs) {
			(*this) = (*this) - aRhs;
			return *this;
		}

		complex<T>& operator*=(const T& aRhs) {
			(*this) = (*this) * aRhs;
			return *this;
		}

		complex<T>& operator/=(const T& aRhs) {
			(*this) = (*this) / aRhs;
			return *this;
		}	

		using std::array<T, 2>::array;

		complex() {
			for (std::size_t i = 0; i < 2; i++) {
				(*this)[i] = T();
			}
		}

		// Variadic template constructor to fill the vector
    	template<typename... args, typename = std::enable_if_t<sizeof...(args) == 2>>
    	complex(args... aArgs) : std::array<T, 2>{static_cast<T>(aArgs)...} {}

		// Calculates the conjugate of the complex number
		complex<T> operator~() const {
			return complex<T>((*this)[0], -(*this)[1]);
		}

		complex<T> operator*(const complex<T>& aRhs) const {
			return complex<T>((*this)[0] * aRhs[0] - (*this)[1] * aRhs[1], (*this)[0] * aRhs[1] + (*this)[1] * aRhs[0]);
		}

		complex<T> operator/(const complex<T>& aRhs) const {
			complex<T> Result;
			T NewR = abs(*this) / abs(aRhs);
			T DeltaPhase = phase(*this) - phase(aRhs);
			Result = NewR * complex<T>(std::cos(DeltaPhase), std::sin(DeltaPhase));
			return Result;
		}

		complex<T>& operator*=(const complex<T>& aRhs) {
			*this = *this * aRhs;
			return *this;
		}

		complex<T>& operator/=(const complex<T>& aRhs) {
			*this = *this / aRhs;
			return *this;
		}

		// complex<T> operator+(T aRhs) const {
		// 	return complex<T>(this->a + aRhs, this->b);
		// }

		// complex<T> operator-(T aRhs) const {
		// 	return complex<T>(this->a - aRhs, this->b);
		// }

		// complex<T>& operator+=(T aRhs) {
		// 	*this = *this + aRhs;
		// 	return *this;
		// }

		// complex<T>& operator-=(T aRhs) {
		// 	*this = *this - aRhs;
		// 	return *this;
		// }

	};

	// template <typename T> inline 
	// complex<T> operator+(T aLhs, const complex<T>& aRhs) {
	// 	return complex<T>(aLhs + aRhs.a, aRhs.b);
	// }

	// template <typename T> inline 
	// complex<T> operator-(T aLhs, const complex<T>& aRhs) {
	// 	return complex<T>(aLhs - aRhs.a, -aRhs.b);
	// }

	// template <typename T> inline 
	// complex<T> operator/(T aLhs, const complex<T>& aRhs) {
	// 	return (aLhs / abs_sqrd(aRhs)) * conj(aRhs);
	// }

	// -------------------- External Functions --------------------

	template <typename T> inline 
	complex<T> operator*(const T& aLhs, const complex<T>& aRhs) {
		return aRhs * aLhs;
	}

	template <typename T> inline 
	T abs_sqrd(const complex<T>& Arg) {
		return (Arg[0]*Arg[0] + Arg[1]*Arg[1]);
	}

	template <typename T> inline 
	T abs(const complex<T>& Arg) {
		return std::sqrt(abs_sqrd(Arg));
	}

	template <typename T> inline 
	T phase(const complex<T>& Arg) {
		return std::atan2(Arg[1], Arg[0]);
	}

	template <typename T> inline 
	complex<T> conj(const complex<T>& Arg) {
		return ~Arg;
	}

	template <typename T> inline 
	complex<T> sqrt(const complex<T>& Arg) {
		T Mag = std::sqrt(abs(Arg));
		T Angle = 0.5 * phase(Arg);
		return Mag * complex<T>(std::cos(Angle), std::sin(Angle));
	}

	template <typename T> inline 
	complex<T> exp(const complex<T>& Arg) {
		return std::exp(Arg.a) * complex<T>(std::cos(Arg.b), std::sin(Arg.b));
	}

	template <typename T> inline 
	complex<T> ln(const complex<T>& Arg) {
		return complex<T>(ln(abs(Arg)), phase(Arg));
	}

	template <typename T> inline 
	complex<T> pow(const complex<T>& Base, const complex<T>& Exponent) {
		return exp(Exponent * (ln(abs(Base)) + complex<T>::i * phase(Base)));
	}

	template <typename T> inline 
	complex<T> sin(const complex<T>& Arg) {
		return complex<T>(std::sin(Arg.a) * std::cosh(Arg.b), std::cos(Arg.a) * std::sinh(Arg.b));
	}

	template <typename T> inline 
	complex<T> cos(const complex<T>& Arg) {
		return complex<T>(std::cos(Arg.a) * std::cosh(Arg.b), -std::sin(Arg.a) * std::sinh(Arg.b));
	}

	template <typename T> inline 
	complex<T> tan(const complex<T>& Arg) {
		return sin(Arg) / cos(Arg);
	}

	template <typename T> inline 
	complex<T> sinh(const complex<T>& Arg) {
		return complex<T>(std::sinh(Arg.a) * std::cos(Arg.b), std::cosh(Arg.a) * std::sin(Arg.b));
	}

	template <typename T> inline 
	complex<T> cosh(const complex<T>& Arg) {
		return complex<T>(std::cosh(Arg.a) * std::cos(Arg.b), std::sinh(Arg.a) * std::sin(Arg.b));
	}

	template <typename T> inline 
	complex<T> tanh(const complex<T>& Arg) {
		return (sinh(Arg) / cosh(Arg));
	}

	template <typename T> inline 
	complex<T> asin(const complex<T>& Arg) {
		return (-complex<T>::i * ln(complex<T>::i * Arg + sqrt(1.0 - pow(Arg, 2.0))));
	}

	template <typename T> inline 
	complex<T> acos(const complex<T>& Arg) {
		return (0.5 * constant::pi + complex<T>::i * ln(complex<T>::i * Arg + sqrt(1.0 - pow(Arg, 2.0))));
	}

	template <typename T> inline 
	complex<T> atan(const complex<T>& Arg) {
		return (0.5 * complex<T>::i * (ln(1.0 - complex<T>::i * Arg) - ln(1.0 + complex<T>::i * Arg)));
	}

	template <typename T> inline 
	complex<T> asinh(const complex<T>& Arg) {
		return ln(Arg + sqrt(pow(Arg, 2.0) + 1.0));
	}

	template <typename T> inline 
	complex<T> acosh(const complex<T>& Arg) {
		return ln(Arg + sqrt(Arg - 1.0) * sqrt(Arg + 1.0));
	}

	template <typename T> inline 
	complex<T> atanh(const complex<T>& Arg) {
		return (0.5 * (ln(1.0 + Arg) - ln(1.0 - Arg)));
	}

}

#endif // !GEODESY_CORE_MATH_COMPLEX_H
