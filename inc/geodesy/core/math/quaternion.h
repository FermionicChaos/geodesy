#pragma once
#ifndef GEODESY_CORE_MATH_QUATERNION_H
#define GEODESY_CORE_MATH_QUATERNION_H

#include "config.h"
#include "type.h"
#include "constants.h"

//tex:
// A quaternion<T> can be writtin in the mathematical form.
// $$ q = a + b\hat{i} + c \hat{j} + d \hat{k} = a + \vec{v} $$

//tex:
// ----- Exponentiation of Quaternions -----
// $$ e^{q} = e^{a} ( \cos{v} + \hat{v} \sin{v} ) $$
// Where:
// $$ v = \text{Magnitude of} \: \; \vec{v} \quad $$
// and,
// $$ \hat{v} = \frac{\vec{v}}{v} $$
// ----- Exponentiation of Quaternions -----
//

//tex:
// ----- Rotation of Quaternions -----
// $$ r^{\prime} = q r q^{-1} $$
// Where $ r $ is defined to be some arbitrary vector to be
// rotated in 3D space. The quaternion $ q $ contains the
// information of what vector $ r $ is to be rotated around.
// The quaternion $ q $ containing the direction vector info
// can be rewritten in its exponential form:
// $$ q = e^{a + \vec{v}} $$
// Its inverse is just simply
// $$ q^{-1} = e^{- a - \vec{v}} $$
// When substituting that in the rotation equation,
// $$ r^{\prime} = (e^{a + \vec{v}}) \cdot r \cdot (e^{- a - \vec{v}}) $$
// since $ e^{a} $ is simply just a scalar value, this commute with our
// quaternions and thus,
// $$ r^{\prime} = (e^{\vec{v}}) \cdot r \cdot (e^{-\vec{v}}) $$
// But since our element $ e^{\vec{v}} $ is simply just,
// $$ e^{\vec{v}} = ( \cos{v} + \hat{v} \sin{v} ) $$
// This tells us that the magnitude of $ \vec{v} $ ($v$), is the degree which
// the vector $ r $ is rotated while the unit vector $ \hat{v} = \frac{\vec{v}}{v} $
// is just simply the base unit vector in which $ r $ is rotated around.
// However, when we are saying that $ v $ designates by "how much" we want
// rotate the vector $ r $ around $ \vec{v} $, we want to know how many radians.
// We know that when the magnitude is zero, there is no rotation at all. For
// reasons I do not currently know, the magnitude corresponds to a rotation
// by the following ammount.
// $$ v = \frac{\theta \; [\text{radians}]}{2} $$
// Therefore the only information needed to calculate an arbitrary rotation, is 
// a vector $ \hat{v} $ to rotate around, and how many degrees $d$ one wishes to 
// rotate by. 
// $$ \vec{r}^{\prime} = e^{\frac{\theta}{2}\hat{v}} \; \vec{r} \; e^{-\frac{\theta}{2}\hat{v}} $$
// ----- Rotation of Quaternions -----

namespace geodesy::core::math {

	template <typename T>
	class quaternion : public std::array<T, 4> {
	public:

		quaternion<T> operator+(const quaternion<T>& aRhs) const {
			quaternion<T> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] + aRhs[i];
			}
			return Out;
		}

		quaternion<T> operator-(const quaternion<T>& aRhs) const {
			quaternion<T> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] - aRhs[i];
			}
			return Out;
		}

		quaternion<T> operator*(const T& aRhs) const {
			quaternion<T> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] * aRhs;
			}
			return Out;
		}

		quaternion<T> operator/(const T& aRhs) const {
			quaternion<T> Out;
			for (std::size_t i = 0; i < this->size(); i++) {
				Out[i] = (*this)[i] / aRhs;
			}
			return Out;
		}

		quaternion<T>& operator+=(const quaternion<T>& aRhs) {
			(*this) = (*this) + aRhs;
			return *this;
		}

		quaternion<T>& operator-=(const quaternion<T>& aRhs) {
			(*this) = (*this) - aRhs;
			return *this;
		}

		quaternion<T>& operator*=(const T& aRhs) {
			(*this) = (*this) * aRhs;
			return *this;
		}

		quaternion<T>& operator/=(const T& aRhs) {
			(*this) = (*this) / aRhs;
			return *this;
		}

		using std::array<T, 4>::array;

		quaternion() {
			for (std::size_t i = 0; i < this->size(); i++) {
				(*this)[i] = T();
			}
		}

		// Generates an orientation quaternion from spherical coordinates.
		quaternion(T aTheta, T aPhi) {
			// Your coordinate system: Z-up, XY ground plane
			// Theta: polar angle from +Z axis (0 = pointing up, π/2 = horizontal, π = pointing down)
			// Phi: azimuthal angle in XY plane (0 = +X direction, π/2 = +Y direction)
		
			// Convert spherical coordinates to Cartesian direction vector
			T SinTheta = std::sin(aTheta);
			T CosTheta = std::cos(aTheta);
			T SinPhi = std::sin(aPhi);
			T CosPhi = std::cos(aPhi);
		
			// Target direction in your coordinate system (Z-up)
			std::array<T, 3> TargetDirection = {
				SinTheta * CosPhi,   // X component
				SinTheta * SinPhi,   // Y component  
				CosTheta            // Z component
			};
		
			// Base direction is +Z axis
			std::array<T, 3> BaseDirection = {T(0), T(0), T(1)};
		
			// Calculate cross product: BaseDirection × TargetDirection
			std::array<T, 3> RotationAxis = {
				BaseDirection[1] * TargetDirection[2] - BaseDirection[2] * TargetDirection[1],  // X
				BaseDirection[2] * TargetDirection[0] - BaseDirection[0] * TargetDirection[2],  // Y
				BaseDirection[0] * TargetDirection[1] - BaseDirection[1] * TargetDirection[0]   // Z
			};
		
			// Calculate dot product: BaseDirection · TargetDirection
			T DotProduct = BaseDirection[0] * TargetDirection[0] + 
						   BaseDirection[1] * TargetDirection[1] + 
						   BaseDirection[2] * TargetDirection[2];
		
			// Calculate length of rotation axis
			T AxisLength = std::sqrt(RotationAxis[0] * RotationAxis[0] + 
									RotationAxis[1] * RotationAxis[1] + 
									RotationAxis[2] * RotationAxis[2]);
			
			if (AxisLength < std::numeric_limits<T>::epsilon()) {
				// Vectors are parallel or anti-parallel
				if (DotProduct > T(0.999)) {
					// Same direction - identity quaternion
					(*this)[0] = T(1);  // w
					(*this)[1] = T(0);  // x
					(*this)[2] = T(0);  // y
					(*this)[3] = T(0);  // z
				} else {
					// Opposite directions - 180° rotation
					// Use X-axis for 180° rotation (since BaseDirection is {0,0,1})
					(*this)[0] = T(0);  // w
					(*this)[1] = T(1);  // x (180° around X-axis)
					(*this)[2] = T(0);  // y
					(*this)[3] = T(0);  // z
				}
			} else {
				// Calculate rotation angle
				T Angle = std::acos(std::clamp(DotProduct, T(-1), T(1)));
			
				// Normalize rotation axis
				T InvAxisLength = T(1) / AxisLength;
				RotationAxis[0] *= InvAxisLength;
				RotationAxis[1] *= InvAxisLength;
				RotationAxis[2] *= InvAxisLength;
			
				// Create quaternion from axis-angle
				T HalfAngle = Angle / T(2);
				T SinHalfAngle = std::sin(HalfAngle);
				T CosHalfAngle = std::cos(HalfAngle);
			
				(*this)[0] = CosHalfAngle;                    // w
				(*this)[1] = RotationAxis[0] * SinHalfAngle; // x
				(*this)[2] = RotationAxis[1] * SinHalfAngle; // y
				(*this)[3] = RotationAxis[2] * SinHalfAngle; // z
			}
		}

		// Variadic template constructor to fill the vector
		template<typename... args, typename = std::enable_if_t<sizeof...(args) == 4>>
		quaternion(args... aArgs) : std::array<T, 4>{static_cast<T>(aArgs)...} {}

		// Calculate the conjugate of the quaternion
		quaternion<T> operator~() const {
			return quaternion<T>((*this)[0], -(*this)[1], -(*this)[2], -(*this)[3]);
		}

		// Calculate the conjugate of the quaternion
		quaternion<T> operator-() const {
			return quaternion<T>(-(*this)[0], -(*this)[1], -(*this)[2], -(*this)[3]);
		}

		quaternion<T> operator+(const T& aRhs) const {
			quaternion<T> Out = *this;
			Out[0] += aRhs;
			return Out;
		}

		quaternion<T> operator-(const T& aRhs) const {
			quaternion<T> Out = *this;
			Out[0] -= aRhs;
			return Out;
		}

		// Calculates the multiplication of two quaternions, using the Hamilton product
		// and the array accessor.
		quaternion<T> operator*(const quaternion<T>& aRhs) const {
			T lqa = (*this)[0], 	lqb = (*this)[1], 	lqc = (*this)[2], 	lqd = (*this)[3];
			T rqa = aRhs[0], 		rqb = aRhs[1], 		rqc = aRhs[2], 		rqd = aRhs[3];
			T oqa = lqa * rqa - lqb * rqb - lqc * rqc - lqd * rqd;
			T oqb = lqa * rqb + lqb * rqa + lqc * rqd - lqd * rqc;
			T oqc = lqa * rqc - lqb * rqd + lqc * rqa + lqd * rqb;
			T oqd = lqa * rqd + lqb * rqc - lqc * rqb + lqd * rqa;
			return quaternion<T>(oqa, oqb, oqc, oqd);
		}

		quaternion<T> operator/(const quaternion<T>& aRhs) const {
			return ((*this) * (~aRhs)) / abs2(aRhs);
		}

		quaternion<T>& operator*=(const quaternion<T>& aRhs) {
			*this = *this * aRhs;
			return *this;
		}

		quaternion<T>& operator/=(const quaternion<T>& aRhs) {
			*this = *this / aRhs;
			return *this;
		}

	};

	// -------------------- External Functions --------------------

	template <typename T> inline 
	quaternion<T> operator*(const T& aLhs, const quaternion<T>& aRhs) {
		return aRhs * aLhs;
	}

	template <typename T> inline 
	quaternion<T> operator+(const T& aLhs, const quaternion<T>& aRhs) {
		return (aRhs + aLhs);
	}

	template <typename T> inline 
	T abs2(const quaternion<T>& aArg) {
		return (aArg[0]*aArg[0] + aArg[1]*aArg[1] + aArg[2]*aArg[2] + aArg[3]*aArg[3]);
	}

	template <typename T> inline 
	T abs(const quaternion<T>& aArg) {
		return std::sqrt(abs2(aArg));
	}

	template <typename T> inline
	quaternion<T> normalize(const quaternion<T>& aArg) {
		return aArg / abs(aArg);
	}

	template <typename T> inline 
	quaternion<T> exp(const quaternion<T>& aArg) {
		quaternion<T> u = quaternion<T>(0.0, aArg[1], aArg[2], aArg[3]);
		T uMag = abs(u);
		return (std::exp(aArg[0]) * (std::cos(uMag) + u * (std::sin(uMag) / uMag)));
	}

	template <typename T> inline 
	quaternion<T> ln(const quaternion<T>& aArg) {
		quaternion<T> u = quaternion<T>(0.0, aArg.b, aArg.c, aArg.d);
		T Q = abs(aArg);
		T U = abs(u);
		return (ln(Q) + u * (std::acos(aArg.a / Q) / U));
	}

	template <typename T> inline 
	quaternion<T> pow(const quaternion<T>& aBase, const quaternion<T>& aExponent) {
		return exp(ln(aBase) * aExponent);
	}

}

#endif // !GEODESY_CORE_MATH_QUATERNION_H
