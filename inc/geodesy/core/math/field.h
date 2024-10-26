#pragma once
#ifndef GEODESY_CORE_MATH_FIELD_H
#define GEODESY_CORE_MATH_FIELD_H

// ------------------------------ field.h ------------------------------ //
/*
* field.h is a class which exists over a selected space with
*/

#include <cmath>
#include <vector>
#include "vec.h"
#include <omp.h>

namespace geodesy::core::math {

	// Compile-time calculation of power of two using constexpr.
	constexpr std::size_t power_of_two(std::size_t n) {
    	return (n == 0) ? 1 : 2 * power_of_two(n - 1);
	}

	template <typename X, std::size_t N, typename Y>
	class field : public std::vector<Y> {
	public:

		// Domain boundaries, must have middle values to interpolate.
		vec<std::size_t, N> ElementCount;
		vec<X, N> LowerBound, UpperBound;

		field() : LowerBound(vec<X, N>{}), UpperBound(vec<X, N>{}), ElementCount(vec<std::size_t, N>{}) {}

		field(vec<std::size_t, N> aElementCount) : LowerBound(vec<X, N>{}), UpperBound(vec<X, N>{}), ElementCount(aElementCount) {
			std::size_t ElementCountTotal = 1;
			for (std::size_t i = 0; i < N; i++) {
				ElementCountTotal *= ElementCount[i];
			}
			this->resize(ElementCountTotal, Y());
		}

		// Constructor for single dimension field.
		field(X aLowerBound, X aUpperBound, std::size_t aElementCount) : field(vec<X, 1>{aLowerBound}, vec<X, 1>{aUpperBound}, vec<std::size_t, 1>{aElementCount}) {}

		field(vec<X, N> aLowerBound, vec<X, N> aUpperBound, vec<std::size_t, N> aElementCount) : LowerBound(aLowerBound), UpperBound(aUpperBound), ElementCount(aElementCount) {
			std::size_t ElementCountTotal = 1;
			for (std::size_t i = 0; i < N; i++) {
				ElementCountTotal *= ElementCount[i];
			}
			this->resize(ElementCountTotal, Y());
		}

		// Multidimensional constructor.
		field(vec<X, N> aLowerBound, vec<X, N> aUpperBound, vec<std::size_t, N> aElementCount, std::size_t aDomainType) : field(aLowerBound, aUpperBound, aElementCount) {
			// ds is the step size for each dimension.
			vec<X, N> ds = (UpperBound - LowerBound);
			for (std::size_t i = 0; i < N; i++) {
				ds[i] /= ElementCount[i] - 1;
			}

			for (std::ptrdiff_t i = 0; i < this->size(); i++) {
				// This is to generate numerical dimensions along each axis, which can be used for numerical functions.
				// ds is the step which will be used to generate the value at each sample point in the domain.
				vec<std::size_t, N> Index = this->convert_to_dimensional_index(i);
				(*this)(Index) = ds[aDomainType - 1] * Index[aDomainType - 1] + LowerBound[aDomainType - 1];
			}
		}

		Y& operator()(const vec<std::size_t, N>& aI) {
			std::size_t GlobalIndex = this->convert_to_global_index(aI);
			return (*this)[GlobalIndex];
		}

		Y operator()(const vec<std::size_t, N>& aI) const {
			std::size_t GlobalIndex = this->convert_to_global_index(aI);
			return (*this)[GlobalIndex];
		}

		// Y& operator()(const vec<X, N>& aX) {
		// 	vec<std::size_t, N> Index;
		// 	// ds is the step size for each dimension.
		// 	vec<X, N> ds = (UpperBound - LowerBound);
		// 	for (std::size_t i = 0; i < N; i++) {
		// 		ds[i] /= ElementCount[i] - 1;
		// 	}
		// 	// Determine Nearest Neighbour.
		// 	for (std::size_t i = 0; i < N; i++) {
		// 		// ds = (X2 - X1) / (N - 1)
		// 		// i = ((X - X1) / ds) + 1
		// 		Index[i] = std::round((aX[i] - LowerBound[i]) / ds[i]);
		// 	}
		// 	return (*this)(Index);
		// }

		// Access f(x) = y. Uses interpolation.
		Y operator()(const vec<X, N>& aX) const {
			Y ReturnValue = Y();
			bool InsideBounds = true;
			for (std::size_t i = 0; i < N; i++) {
				InsideBounds &= (aX[i] >= LowerBound[i]) && (aX[i] <= UpperBound[i]);
			}
			if (!InsideBounds) return ReturnValue;
			std::array<vec<std::size_t, N>, power_of_two(N)> NeighbourIndices;
			std::array<vec<X, N>, power_of_two(N)> SamplePositions;
			std::array<Y, power_of_two(N)> SamplePoints;

			// ds is the step size for each dimension.
			vec<X, N> ds = (UpperBound - LowerBound);
			for (std::size_t i = 0; i < N; i++) {
				ds[i] /= ElementCount[i] - 1;
			}

			// Determine Lower Nearest Neighbour.
			for (std::size_t i = 0; i < N; i++) {
				// ds = (X2 - X1) / (N - 1)
				// i = ((X - X1) / ds) + 1
				NeighbourIndices[0][i] = std::floor((aX[i] - LowerBound[i]) / ds[i]);
			}

			// Generate list of neighboring points with contain aX.
			for (std::size_t i = 1; i < power_of_two(N); i++) {
				NeighbourIndices[i] = NeighbourIndices[0];
				for (std::size_t j = 0; j < N; j++) {
					if (i & (1 << j)) {
						NeighbourIndices[i][j]++;
					}
				}
			}

			// Load Sample Positions & Sample Points to stack memory.
			for (std::size_t i = 0; i < power_of_two(N); i++) {
				SamplePositions[i] = convert_index_to_position(NeighbourIndices[i]);
				if ((NeighbourIndices[i] >= vec<std::size_t, N>()) && (NeighbourIndices[i] < ElementCount)) {
					SamplePoints[i] = (*this)(NeighbourIndices[i]);
				} else {
					SamplePoints[i] = Y();
				}
			}

			// Iteratively reduce the problem dimensionality.
        	std::size_t CurrentPoints = power_of_two(N);
        	for (std::size_t Dim = 0; Dim < N; ++Dim) {
        	    std::size_t Step = CurrentPoints / 2;
        	    for (std::size_t i = 0; i < Step; ++i) {
        	        X Denominator = SamplePositions[2 * i + 1][Dim] - SamplePositions[2 * i][Dim];

        	        // Check for zero denominator to avoid division by zero.
        	        if (std::fabs(Denominator) < std::numeric_limits<X>::epsilon()) {
        	            SamplePoints[i] = SamplePoints[2 * i]; // Use the first point if they are effectively the same.
        	        } else {
        	            X T = (aX[Dim] - SamplePositions[2 * i][Dim]) / Denominator;
        	            SamplePoints[i] = SamplePoints[2 * i] * (1 - T) + SamplePoints[2 * i + 1] * T;
        	        }

        	        // Update sample positions for the next round.
        	        SamplePositions[i] = SamplePositions[2 * i]; // Keep track of updated positions.
        	    }

        	    // Halve the number of points for the next dimension.
        	    CurrentPoints /= 2;
        	}

			return SamplePoints[0];
		}

		// Determines the union bounds of the two fields.
		field<X, N, Y> operator|(const field<X, N, Y>& aRhs) const {
			field<X, N, Y> Out;
			for (std::size_t i = 0; i < N; i++) {
				Out.LowerBound[i] 	= std::min(this->LowerBound[i], aRhs.LowerBound[i]);
				Out.UpperBound[i] 	= std::max(this->UpperBound[i], aRhs.UpperBound[i]);
				Out.ElementCount[i] = std::max(this->ElementCount[i], aRhs.ElementCount[i]);
			}
			std::size_t ElementCountTotal = 1;
			for (std::size_t i = 0; i < N; i++) {
				ElementCountTotal *= Out.ElementCount[i];
			}
			Out.resize(ElementCountTotal, Y());
			return Out;
		}

		// Determines the intersection bounds of the two fields.
		field<X, N, Y> operator&(const field<X, N, Y>& aRhs) const {
			field<X, N, Y> Out;
			for (std::size_t i = 0; i < N; i++) {
				Out.LowerBound[i] 	= std::max(this->LowerBound[i], aRhs.LowerBound[i]);
				Out.UpperBound[i] 	= std::min(this->UpperBound[i], aRhs.UpperBound[i]);
				Out.ElementCount[i] = std::max(this->ElementCount[i], aRhs.ElementCount[i]);
			}
			std::size_t ElementCountTotal = 1;
			for (std::size_t i = 0; i < N; i++) {
				ElementCountTotal *= Out.ElementCount[i];
			}
			Out.resize(ElementCountTotal, Y());
			return Out;
		}

		field<X, N, Y> operator-() const {
			field<X, N, Y> Out;
			for (std::size_t i = 0; i < N; i++) {
				Out[i] = -(*this)[i];
			}
			return Out;
		}

		field<X, N, Y> operator+(const field<X, N, Y>& aRhs) const {
			field<X, N, Y> Out = *this | aRhs;
			#pragma omp parallel for
			for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
				vec<std::size_t, N> Index = Out.convert_to_dimensional_index(i);
				vec<X, N> Position = Out.convert_index_to_position(Index);
				Out[i] = (*this)(Position) + aRhs(Position);
			}
			return Out;
		}

		field<X, N, Y> operator-(const field<X, N, Y>& aRhs) const {
			field<X, N, Y> Out = *this | aRhs;
			#pragma omp parallel for
			for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
				vec<std::size_t, N> Index = Out.convert_to_dimensional_index(i);
				vec<X, N> Position = Out.convert_index_to_position(Index);
				Out[i] = (*this)(Position) - aRhs(Position);
			}
			return Out;
		}

		field<X, N, Y> operator*(const field<X, N, Y>& aRhs) const {
			field<X, N, Y> Out = *this | aRhs;
			#pragma omp parallel for
			for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
				vec<std::size_t, N> Index = Out.convert_to_dimensional_index(i);
				vec<X, N> Position = Out.convert_index_to_position(Index);
				Out[i] = (*this)(Position) * aRhs(Position);
			}
			return Out;
		}

		field<X, N, Y> operator/(const field<X, N, Y>& aRhs) const {
			field<X, N, Y> Out = *this | aRhs;
			#pragma omp parallel for
			for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
				vec<std::size_t, N> Index = Out.convert_to_dimensional_index(i);
				vec<X, N> Position = Out.convert_index_to_position(Index);
				Out[i] = (*this)(Position) / aRhs(Position);
			}
			return Out;
		}

		field<X, N, Y> operator+(const Y& aRhs) const {
			field<X, N, Y> Out = *this;
			#pragma omp parallel for
			for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
				vec<std::size_t, N> Index = Out.convert_to_dimensional_index(i);
				vec<X, N> Position = Out.convert_index_to_position(Index);
				Out[i] = (*this)(Position) + aRhs;
			}
			return Out;
		}

		field<X, N, Y> operator-(const Y& aRhs) const {
			field<X, N, Y> Out = *this;
			#pragma omp parallel for
			for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
				vec<std::size_t, N> Index = Out.convert_to_dimensional_index(i);
				vec<X, N> Position = Out.convert_index_to_position(Index);
				Out[i] = (*this)(Position) - aRhs;
			}
			return Out;
		}

		field<X, N, Y> operator*(const Y& aRhs) const {
			field<X, N, Y> Out = *this;
			#pragma omp parallel for
			for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
				vec<std::size_t, N> Index = Out.convert_to_dimensional_index(i);
				vec<X, N> Position = Out.convert_index_to_position(Index);
				Out[i] = (*this)(Position) * aRhs;
			}
			return Out;
		}

		field<X, N, Y> operator/(const Y& aRhs) const {
			field<X, N, Y> Out = *this;
			#pragma omp parallel for
			for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
				vec<std::size_t, N> Index = Out.convert_to_dimensional_index(i);
				vec<X, N> Position = Out.convert_index_to_position(Index);
				Out[i] = (*this)(Position) / aRhs;
			}
			return Out;
		}

		field<X, N, Y>& operator+=(const field<X, N, Y>& aRhs) {
			(*this) = (*this) + aRhs;
			return *this;
		}

		field<X, N, Y>& operator-=(const field<X, N, Y>& aRhs) {
			(*this) = (*this) - aRhs;
			return *this;
		}

		field<X, N, Y>& operator*=(const Y& aRhs) {
			(*this) = (*this) * aRhs;
			return *this;
		}

		field<X, N, Y>& operator/=(const Y& aRhs) {
			(*this) = (*this) / aRhs;
			return *this;
		}

		bool increment(vec<std::size_t, N>& aIndex, const vec<std::size_t, N>& aElementCount) const {
			bool BreakLoop = false;
			for (std::size_t i = 0; i < N; i++) {
				aIndex[i]++;
				if (aIndex[i] >= aElementCount[i]) {
					if (i == N - 1) {
						aIndex = vec<std::size_t, N>{};
						BreakLoop = true;
						break;
					}
					aIndex[i] = 0;
				} else {
					break;
				}
			}
			return !BreakLoop;
		}

		vec<std::size_t, N> convert_to_dimensional_index(std::size_t aIndex) const {
			std::size_t GlobalIndex = aIndex;
			vec<std::size_t, N> Out;
			// GlobalIndex = i1 + i2*N1 + i3*N1*N2 ... 

    		// Iterate through each dimension, from the first to the last.
    		for (std::size_t i = 0; i < N; ++i) {

    		    // For each dimension, we compute the index as the remainder of division (modulo).
    		    Out[i] = GlobalIndex % ElementCount[i];

				// Remove term from sum.
				GlobalIndex -= Out[i];

    		    // Update the global index for the next dimension by dividing by the size of the current dimension.
    		    GlobalIndex /= ElementCount[i];
    		}

			return Out;
		}

		std::size_t convert_to_global_index(const vec<std::size_t, N>& aIndex) const {
			// GlobalIndex = i1 + i2*N1 + i3*N1*N2 ... 
			std::size_t GlobalIndex = 0;
			for (std::size_t i = 0; i < N; i++) {
				std::size_t Product = 1;
				for (std::size_t j = 0; j < i; j++) {
					Product *= ElementCount[j];
				}
				GlobalIndex += aIndex[i] * Product;
			}
			return GlobalIndex;
		}

		vec<X, N> convert_index_to_position(const vec<std::size_t, N>& aIndex) const {
			vec<X, N> Out;
			for (std::size_t i = 0; i < N; i++) {
				Out[i] = LowerBound[i] + aIndex[i] * (UpperBound[i] - LowerBound[i]) / ((X)(ElementCount[i] - 1));
			}
			return Out;
		}

	};

	template <typename X, std::size_t N, typename Y>
	field<X, N, Y> operator+(const Y& aLhs, field<X, N, Y>& aRhs) {
		return aRhs + aLhs;
	}

	template <typename X, std::size_t N, typename Y>
	field<X, N, Y> operator-(const Y& aLhs, field<X, N, Y>& aRhs) {
		return (-aRhs) + aLhs;
	}

	template <typename X, std::size_t N, typename Y>
	field<X, N, Y> operator*(const Y& aLhs, field<X, N, Y>& aRhs) {
		return aRhs * aLhs;
	}

	template <typename X, std::size_t N, typename Y>
	field<X, N, Y> operator/(const Y& aLhs, field<X, N, Y>& aRhs) {
		field<X, N, Y> Out = aRhs;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			vec<std::size_t, N> Index = Out.convert_to_dimensional_index(i);
			vec<X, N> Position = Out.convert_index_to_position(Index);
			Out[i] = aLhs / aRhs(Position);
		}
		return Out;
	}

	// ------------------------- Mathematical functions -------------------------

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> sin(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::sin(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> cos(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::cos(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> tan(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::tan(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> asin(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::asin(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> acos(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::acos(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> atan(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::atan(Out[i]);
		}
		return Out;
	}

	// Hyperbolic functions
	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> sinh(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::sinh(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> cosh(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::cosh(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> tanh(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::tanh(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> asinh(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::asinh(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> acosh(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::acosh(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> atanh(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::atanh(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> exp(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::exp(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> log(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			if (Out[i] > 0) {
				Out[i] = std::log(Out[i]);
			} else {
				Out[i] = Y();
			}
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> pow(const field<X, N, Y>& aBase, const field<X, N, Y>& aExponent) {
		field<X, N, Y> Out = aBase | aExponent;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			vec<std::size_t, N> Index = Out.convert_to_dimensional_index(i);
			vec<X, N> Position = Out.convert_index_to_position(Index);
			Out[i] = std::pow(aBase(Position), aExponent(Position));
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> log(const field<X, N, Y>& aBase, const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aBase | aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			vec<std::size_t, N> Index = Out.convert_to_dimensional_index(i);
			vec<X, N> Position = Out.convert_index_to_position(Index);
			if (aInput(Position) > 0) {
				Out[i] = std::log(aInput(Position)) / std::log(aBase(Position));
			} else {
				Out[i] = Y();
			}
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> sqrt(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			if (Out[i] >= 0) {
				Out[i] = std::sqrt(Out[i]);
			} else {
				Out[i] = Y();
			}
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> erf(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::erf(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> gamma(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::tgamma(Out[i]);
		}
		return Out;
	}

	// ------------------------- Utility functions -------------------------

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> abs(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			Out[i] = std::abs(Out[i]);
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> normalize(const field<X, N, Y>& aInput) {
		field<X, N, Y> Out = aInput;
		Y MaxValue = Y();
		for (std::size_t i = 0; i < aInput.size(); i++) {
			MaxValue = std::max(MaxValue, aInput[i]);
		}
		return Out / MaxValue;
	}

	// ------------------------- Differential operators -------------------------

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, vec<Y, N>> grad(const field<X, N, Y>& aF) {
		field<X, N, vec<Y, N>> Out(aF.LowerBound, aF.UpperBound, aF.ElementCount);
		vec<X, N> ds = (aF.UpperBound - aF.LowerBound);
		for (std::size_t i = 0; i < N; i++) {
			ds[i] /= (aF.ElementCount[i] - 1);
		}
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			vec<std::size_t, N> k = Out.convert_to_dimensional_index(i);
			for (std::size_t j = 0; j < N; j++) {
				Y df = Y();
				X dxj = 2.0 * ds[j];
				if ((k[j] > 0) && (k[j] < (aF.ElementCount[j] - 1))) {
					vec<std::size_t, N> k1 = k;
					vec<std::size_t, N> k2 = k;
					k1[j] -= 1;
					k2[j] += 1;
					df = aF(k2) - aF(k1);
				}
				Out[i][j] = df / dxj;
			}
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> div(const field<X, N, vec<Y, N>>& aF) {
		field<X, N, Y> Out(aF.LowerBound, aF.UpperBound, aF.ElementCount);
		vec<X, N> ds = (aF.UpperBound - aF.LowerBound);
		for (std::size_t i = 0; i < N; i++) {
			ds[i] /= (aF.ElementCount[i] - 1);
		}
		#pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			vec<std::size_t, N> k = Out.convert_to_dimensional_index(i);
			vec<Y, N> df = vec<Y, N>();
			for (std::size_t j = 0; j < N; j++) {
				X dxj = 2.0 * ds[j];
				if ((k[j] > 0) && (k[j] < (aF.ElementCount[j] - 1))) {
					vec<std::size_t, N> k1 = k;
					vec<std::size_t, N> k2 = k;
					k1[j] -= 1;
					k2[j] += 1;
					df = aF(k2)[j] - aF(k1)[j];
				}
				Out[i][j] += df / dxj;
			}
		}
		return Out;
	}

	template <typename X, std::size_t N, typename Y> inline
	field<X, N, Y> laplacian(const field<X, N, Y>& aF) {
		field<X, N, Y> Out(aF.LowerBound, aF.UpperBound, aF.ElementCount);
		vec<X, N> ds = (aF.UpperBound - aF.LowerBound);
		for (std::size_t i = 0; i < N; i++) {
			ds[i] /= (aF.ElementCount[i] - 1);
		}
		// #pragma omp parallel for
		for (std::ptrdiff_t i = 0; i < Out.size(); i++) {
			vec<std::size_t, N> k = Out.convert_to_dimensional_index(i);
			for (std::size_t j = 0; j < N; j++) {
				Y df = Y();
				if ((k[j] > 0) && (k[j] < (aF.ElementCount[j] - 1))) {
					vec<std::size_t, N> k1 = k;
					vec<std::size_t, N> k2 = k;
					k1[j] -= 1;
					k2[j] += 1;
					df = aF(k2) - aF(k) + aF(k1) - aF(k);
					// Y fback = aF(k1);
					// Y fmid = aF(k);
					// fmid = 2.0f * fmid;
					// Y ffront = aF(k2);
					// df = ffront - fmid + fback;
				}
				Out[i] += df / (ds[j] * ds[j]);
			}
		}
		return Out;
	}

	// Define integration over boundaries.
	template <typename X, std::size_t N, typename Y> inline
	Y integrate(const vec<X, N>& aLowerBound, const vec<X, N>& aUpperBound, const field<X, N, Y>& aF) {
		Y Out = Y();
		vec<X, N> ds = (aF.UpperBound - aF.LowerBound);
		for (std::size_t i = 0; i < N; i++) {
			ds[i] /= (aF.ElementCount[i] - 1);
		}
		// Perform integration over region here.
		return Out;
	}

}

#endif // !GEODESY_CORE_MATH_FIELD_H
