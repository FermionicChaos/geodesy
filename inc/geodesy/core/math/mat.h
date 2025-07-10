
#pragma once
#ifndef GEODESY_CORE_MATH_MAT_H
#define GEODESY_CORE_MATH_MAT_H

#include "config.h"
#include "complex.h"
#include "quaternion.h"
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

		// Variadic template constructor that converts row-major input to column-major storage
		template<typename... Args, typename = std::enable_if_t<sizeof...(Args) == M*N>>
		mat(Args... aArgs) {
			// First store the arguments in a temporary array
			std::array<T, M*N> TempArgs{static_cast<T>(aArgs)...};

			// Convert from row-major input to column-major storage
			for (std::size_t Row = 0; Row < M; Row++) {
				for (std::size_t Col = 0; Col < N; Col++) {
					// Input index assumes row-major format: Row * N + Col
					// Output index in column-major format: Row + Col * M
					(*this)(Row, Col) = TempArgs[Row * N + Col];
				}
			}
		}

		// Initializer list constructor with row-to-column major conversion
		mat(std::initializer_list<T> aList) {
			if (aList.size() != M*N) {
				throw std::invalid_argument("Initializer list size does not match matrix dimensions");
			}

			// Convert from row-major input to column-major storage
			auto It = aList.begin();
			for (std::size_t Row = 0; Row < M; Row++) {
				for (std::size_t Col = 0; Col < N; Col++) {
					(*this)(Row, Col) = *It++;
				}
			}
		}

		//--------------------------------------------------------------------
		//  In mat.h  — inside template<class T,std::size_t M,std::size_t N> struct mat
		//  Works for any square size ≥ 3×3.
		//  Access pattern:  q[0] = real  ‖  q[1] q[2] q[3] = x y z
		//--------------------------------------------------------------------
		mat(const quaternion<T>& q) {
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
			
			// Initialize the matrix to the identity matrix
			for (std::size_t r = 0; r < M; ++r)
				for (std::size_t c = 0; c < N; ++c)
					(*this)(r, c) = (r == c ? T{1} : T{0});
		
			// Load quaternion components.
			const T w = q[0]; // Real Component
			const T x = q[1];
			const T y = q[2];
			const T z = q[3];
		
			const T xx = x * x,  yy = y * y,  zz = z * z;
			const T xy = x * y,  xz = x * z,  yz = y * z;
			const T wx = w * x,  wy = w * y,  wz = w * z;
		
			// Load the upper 3x3 of the matrix.
			(*this)(0,0) = 1.0 - 2.0 * (yy + zz);
			(*this)(0,1) =       2.0 * (xy - wz);
			(*this)(0,2) =       2.0 * (xz + wy);
			(*this)(1,0) =       2.0 * (xy + wz);
			(*this)(1,1) = 1.0 - 2.0 * (xx + zz);
			(*this)(1,2) =       2.0 * (yz - wx);
			(*this)(2,0) =       2.0 * (xz - wy);
			(*this)(2,1) =       2.0 * (yz + wx);
			(*this)(2,2) = 1.0 - 2.0 * (xx + yy);
		}

		// Accessor functions, default memory interpretation is column-major
		const T operator()(std::size_t aRow, std::size_t aColumn) const {
			return (*this)[aRow + aColumn * M];
		}

		T& operator()(std::size_t aRow, std::size_t aColumn) {
			return (*this)[aRow + aColumn * M];
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

	template<typename T, std::size_t M, std::size_t N> inline
	mat<T, M, N> transpose(const mat<T, M, N>& aMatrix) {
		mat<T, N, M> Result;
		for (std::size_t i = 0; i < M; i++) {
			for (std::size_t j = 0; j < N; j++) {
				Result(j, i) = aMatrix(i, j);
			}
		}
		return Result;
	}

	template <typename T, std::size_t M, std::size_t N> inline
	T trace(const mat<T, M, N>& aMatrix) {
		T Result = T();
		for (std::size_t i = 0; i < M; ++i) {
			Result += aMatrix(i, i);
		}
		return Result;
	}

	/*======================================================================
	 |  Determinant via LU  (constexpr, O(N³))
	 *====================================================================*/
	template<typename T, std::size_t N> inline
	constexpr T determinant(mat<T,N,N> A) {
		T   detSign = T{1};
		for (std::size_t k = 0; k < N; ++k) {
			/* --- partial pivot ------------------------------------------------ */
			std::size_t piv = k;
			T maxAbs = std::abs(A(k,k));
			for (std::size_t r = k+1; r < N; ++r) {
				T v = std::abs(A(r,k));
				if (v > maxAbs) { maxAbs = v; piv = r; }
			}
			if (maxAbs < std::numeric_limits<T>::epsilon())
				return T{0};                                         // singular

			if (piv != k) {                                          // row swap
				for (std::size_t c = 0; c < N; ++c)
					std::swap(A(k,c), A(piv,c));
				detSign = -detSign;
			}

			/* --- elimination below pivot ------------------------------------- */
			for (std::size_t r = k+1; r < N; ++r) {
				T factor = A(r,k) / A(k,k);
				A(r,k) = T{0};                                       // exact zero
				for (std::size_t c = k+1; c < N; ++c)
					A(r,c) -= factor * A(k,c);
			}
		}

		T det = detSign;
		for (std::size_t i = 0; i < N; ++i) det *= A(i,i);           // product diag
		return det;
	}

	/*======================================================================
	 |  Inverse via Gauss-Jordan  (constexpr, O(N³))
	 *====================================================================*/
	template<typename T, std::size_t N> inline
	constexpr mat<T,N,N> inverse(mat<T,N,N> A) {
		mat<T,N,N> I{};                                              // identity
		for (std::size_t i = 0; i < N; ++i) I(i,i) = T{1};

		for (std::size_t k = 0; k < N; ++k) {
			/* --- pivot selection --------------------------------------------- */
			std::size_t piv = k;
			T maxAbs = std::abs(A(k,k));
			for (std::size_t r = k+1; r < N; ++r) {
				T v = std::abs(A(r,k));
				if (v > maxAbs) { maxAbs = v; piv = r; }
			}
			if (maxAbs < std::numeric_limits<T>::epsilon())
				throw std::domain_error("inverse(): singular matrix");

			/* --- swap pivot row into place (both A and I) --------------------- */
			if (piv != k) {
				for (std::size_t c = 0; c < N; ++c) {
					std::swap(A(k,c), A(piv,c));
					std::swap(I(k,c), I(piv,c));
				}
			}

			/* --- scale pivot row to make pivot = 1 ---------------------------- */
			const T invPivot = T{1} / A(k,k);
			for (std::size_t c = 0; c < N; ++c) {
				A(k,c) *= invPivot;
				I(k,c) *= invPivot;
			}

			/* --- eliminate other rows ---------------------------------------- */
			for (std::size_t r = 0; r < N; ++r) if (r != k) {
				const T f = A(r,k);
				if (f == T{}) continue;
				for (std::size_t c = 0; c < N; ++c) {
					A(r,c) -= f * A(k,c);
					I(r,c) -= f * I(k,c);
				}
			}
		}
		return I;                                                    // now A⁻¹
	}

	// Generates orthographic projection matrix for Geodesy → Vulkan NDC
	template <typename T> inline
	mat<T, 4, 4> orthographic(T aDeltaX, T aDeltaY, T aNear, T aFar) {
		return mat<T, 4, 4>(
			2 / aDeltaX,		0,					0,							0,
			0,					-2 / aDeltaY,		0,							0, 							// Flipped Y for Vulkan
			0,					0,					-1 / (aFar - aNear),		aFar / (aFar - aNear), 		// Z: [Near,Far] → [1,0] (reversed)
			0,					0,					0,							1 							// Translation for reverse depth
		);
	}

	// Generates a projection matrix
	template <typename T> inline 
	mat<T, 4, 4> perspective(T FOV, T AspectRatio, T Near, T Far) {
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
			0.0,            0.0,                    (-Near / (Far - Near)),       			(Far * Near / (Far - Near)),
			0.0,            0.0,                    1.0,                                    0.0
		);
	}

	//--------------------------------------------------------------------
	//  Matrix → Quaternion   (upper-left 3×3 block)
	//--------------------------------------------------------------------
	template<typename T, std::size_t M, std::size_t N> inline
	quaternion<T> quat(const mat<T,M,N>& R) {
		const T r00 = R(0,0), r01 = R(0,1), r02 = R(0,2);
		const T r10 = R(1,0), r11 = R(1,1), r12 = R(1,2);
		const T r20 = R(2,0), r21 = R(2,1), r22 = R(2,2);

		quaternion<T> q;
		const T trace = r00 + r11 + r22;

		if (trace > T(0)) {
			T s = std::sqrt(trace + T(1));
			q[0] = s * T(0.5);                     // w
			s    = T(0.5) / s;
			q[1] = (r21 - r12) * s;
			q[2] = (r02 - r20) * s;
			q[3] = (r10 - r01) * s;
		}
		else {
			if (r00 > r11 && r00 > r22) {
				T s = std::sqrt(T(1) + r00 - r11 - r22);
				q[1] = s * T(0.5);
				s    = T(0.5) / s;
				q[0] = (r21 - r12) * s;
				q[2] = (r01 + r10) * s;
				q[3] = (r02 + r20) * s;
			}
			else if (r11 > r22) {
				T s = std::sqrt(T(1) + r11 - r00 - r22);
				q[2] = s * T(0.5);
				s    = T(0.5) / s;
				q[0] = (r02 - r20) * s;
				q[1] = (r01 + r10) * s;
				q[3] = (r12 + r21) * s;
			}
			else {
				T s = std::sqrt(T(1) + r22 - r00 - r11);
				q[3] = s * T(0.5);
				s    = T(0.5) / s;
				q[0] = (r10 - r01) * s;
				q[1] = (r02 + r20) * s;
				q[2] = (r12 + r21) * s;
			}
		}
		return normalize(q);   // call your normalize() if available
	}

	template<typename T> inline
	mat<T, 4, 4> orientation(T aTheta, T aPhi) {
		const T CosTheta = std::cos(aTheta);
		const T SinTheta = std::sin(aTheta);
		const T CosPhi = std::cos(aPhi);
		const T SinPhi = std::sin(aPhi);
		mat<T, 4, 4> OrientationMatrix = {
			SinPhi,                     SinTheta * CosPhi,      -CosTheta * CosPhi,     0.0f,
			-CosPhi,                    SinTheta * SinPhi,      -CosTheta * SinPhi,     0.0f,
			0.0f,                       CosTheta,               SinTheta,               0.0f,
			0.0f,                       0.0f,                   0.0f,                   1.0f
		};
		return OrientationMatrix;
	}

	template<typename T> inline
	mat<T, 4, 4> rotation(T aTheta, T aPhi) {
		const T CosTheta = std::cos(aTheta);
		const T SinTheta = std::sin(aTheta);
		const T CosPhi = std::cos(aPhi);
		const T SinPhi = std::sin(aPhi);
		mat<T, 4, 4> RotationMatrix = {
			SinPhi,                     -CosPhi,                0.0f,                   0.0f,
			CosTheta * CosPhi,          CosTheta * SinPhi,      -SinTheta,              0.0f,
			SinTheta * CosPhi,          SinTheta * SinPhi,      CosTheta,               0.0f,
			0.0f,                       0.0f,                   0.0f,                   1.0f
		};
		return RotationMatrix;
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
