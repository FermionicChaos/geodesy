#pragma once
#ifndef GEODESY_CORE_MATH_QUATERNION_H
#define GEODESY_CORE_MATH_QUATERNION_H

#include "config.h"
#include "type.h"
#include "constants.h"
#include "vec.h"
#include "complex.h"


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

		// Variadic template constructor to fill the vector
    	template<typename... args, typename = std::enable_if_t<sizeof...(args) == 4>>
    	quaternion(args... aArgs) : std::array<T, 4>{static_cast<T>(aArgs)...} {}

		// Calculate the conjugate of the quaternion
		quaternion<T> operator~() const {
			return quaternion<T>((*this)[0], -(*this)[1], -(*this)[2], -(*this)[3]);
		}

		// Calculates the multiplication of two quaternions, using the Hamilton product
		// and the array accessor.
		quaternion<T> operator*(const quaternion<T>& aRhs) const {
			T lqa = (*this)[0], lqb = (*this)[1], lqc = (*this)[2], lqd = (*this)[3];
			T rqa = aRhs[0], rqb = aRhs[1], rqc = aRhs[2], rqd = aRhs[3];
			T oqa = lqa * rqa - lqb * rqb - lqc * rqc - lqd * rqd;
			T oqb = lqa * rqb + lqb * rqa + lqc * rqd - lqd * rqc;
			T oqc = lqa * rqc - lqb * rqd + lqc * rqa + lqd * rqb;
			T oqd = lqa * rqd + lqb * rqc - lqc * rqb + lqd * rqa;
			return quaternion<T>(oqa, oqb, oqc, oqd);
		}

		quaternion<T> operator/(const quaternion<T>& aRhs) const {
			return ((*this) * (~aRhs)) / abs_sqrd(aRhs);
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
	T abs_sqrd(const quaternion<T>& aArg) {
		return (aArg[0]*aArg[0] + aArg[1]*aArg[1] + aArg[2]*aArg[2] + aArg[3]*aArg[3]);
	}

	template <typename T> inline 
	T abs(const quaternion<T>& aArg) {
		return std::sqrt(abs_sqrd(aArg));
	}

	template <typename T> inline 
	quaternion<T> exp(const quaternion<T>& aArg) {
		quaternion<T> u = quaternion<T>(0.0, aArg.b, aArg.c, aArg.d);
		T uMag = abs(u);
		return (std::exp(aArg.a) * (std::cos(uMag) + u * (std::sin(uMag) / uMag)));
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

	template <typename T>
    inline mat<T, 4, 4> perspective(T FOV, T AspectRatio, T Near, T Far) {
        //tex:
        // Aspect Ratio: $$a$$
        // Field of View (Radians): $$\theta$$
        // Near Point: $$n$$
        // Far Point: $$f$$
        // $$ x_{n} = \frac{1}{\tan{\frac{\theta}{2}}} \frac{x_{e}}{z_{e}}$$
        // $$ y_{n} = \frac{a}{\tan{\frac{\theta}{2}}} \frac{y_{e}}{z_{e}}$$
        // $$ z_{n} = \frac{1}{z_{e}} \bigg(-\frac{f+n}{f-n} z_{e} + \frac{2fn}{f-n} \bigg)$$ 
        // The $z$ term is why the perspective matrix must be a mat4<float> type 
        // and not just a float3x3. The set of equations above describe
        // the transform from what the perspective of the camera
        // to the screen space of the context.
        // 
        // The matrix then takes the form of 
        // $$ P =
        // \begin{bmatrix}
        // \frac{1}{\tan{\frac{\theta}{2}}} & 0 & 0 & 0 \\
    	// 0 & \frac{a}{\tan{\frac{\theta}{2}}} & 0 & 0 \\
    	// 0 & 0 & - \frac{f + n}{f - n} & \frac{2fn}{f - n} \\
    	// 0 & 0 & 1 & 0 \\
    	// \end{bmatrix}
        // $$

        T tn = std::tan(FOV / 2.0);
        return mat<T, 4, 4>(
            (1.0 / tn),     0.0,                    0.0,                                    0.0,
            0.0,            (AspectRatio / tn),     0.0,                                    0.0,
            0.0,            0.0,                    (-((Far + Near) / (Far - Near))),       ((2.0 * Far * Near) / ((double)Far - (double)Near)),
            0.0,            0.0,                    1.0,                                    0.0
        );
    }

	// Uses a quaternion intended for rotation and casts it into a rotation matrix.
	template <typename T>
	inline mat<T, 4, 4> rotation(quaternion<T> aQuaternion) {
		//tex:
		// In quaternion notation, a rotation is of the form
		// $$ \vec{r}^{'} = q\vec{r}q^{-1} $$
		// Where 
		// $ q = e^{\phi} $
		// and $\phi$ is
		// $$ \phi = \frac{\theta}{2} \hat{u} $$
		// $\theta$ is the angle of rotation, and $\hat{u}$ is the vector
		// which the object is rotated around.
		// $$ s = \frac{1}{|q|^{2}} $$
		// The matrix below is to be used in the following way $\vec{r}^{'} = R \vec{r}$
		// and is equivalent to $ \vec{r}^{'} = q \vec{r} q^{-1} $.
		// $$ R = 
		// \begin{bmatrix}
		// 1 - s(c^{2} + d^{2}) & 2s(bc - da) & 2s(bd + ca) \\ 
		// 2s(bc + da) & 1 - 2s(b^{2} + d^{2}) & 2s(cd - ba) \\
    	// 2s(bd - ca) & 2s(cd + ba) & 1 - 2s(b^{2} + c^{2})
		// \end{bmatrix}    
		// $$
		// Citation: http://www.faqs.org/faqs/gfx/algorithms-faq/

		quaternion<T> q = aQuaternion;
		T s = 1.0 / abs_sqrd(q);
		T qa = q[0], qb = q[1], qc = q[2], qd = q[3];
		return mat<T, 4, 4>(
			1.0 - 2.0 * s * (qc * qc + qd * qd), 2.0 * s * (qb * qc - qd * qa), 2.0 * s * (qb * qd + qc * qa), 0.0,
			2.0 * s * (qb * qc + qd * qa), 1.0 - 2.0 * s * (qb * qb + qd * qd), 2.0 * s * (qc * qd - qb * qa), 0.0,
			2.0 * s * (qb * qd - qc * qa), 2.0 * s * (qc * qd + qb * qa), 1.0 - 2.0 * s * (qb * qb + qc * qc), 0.0,
			0.0, 0.0, 0.0, 1.0
		);
		// return mat<T, 4, 4>(
		// 	1.0 - 2.0 * s * (q.c * q.c + q.d * q.d), 2.0 * s * (q.b * q.c - q.d * q.a), 2.0 * s * (q.b * q.d + q.c * q.a), 0.0,
		// 	2.0 * s * (q.b * q.c + q.d * q.a), 1.0 - 2.0 * s * (q.b * q.b + q.d * q.d), 2.0 * s * (q.c * q.d - q.b * q.a), 0.0,
		// 	2.0 * s * (q.b * q.d - q.c * q.a), 2.0 * s * (q.c * q.d + q.b * q.a), 1.0 - 2.0 * s * (q.b * q.b + q.c * q.c), 0.0,
		// 	0.0, 0.0, 0.0, 1.0
		// );
	}

    template <typename T>
	inline mat<T, 4, 4> rotation(T aAngle, vec<T, 3> aAxis) {
        vec<T, 3> V 	    = normalize(aAxis);
		T Phi 		        = std::exp(aAngle / 2.0);
		quaternion<T> q     = Phi * quaternion<T>(0.0, V.x, V.y, V.z);
		return rotation(q);
	}

}

#endif // !GEODESY_CORE_MATH_QUATERNION_H
