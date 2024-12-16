
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

	// !BUG: Still doesn't work, fix later. Study LU decomposition.
	template <typename T, std::size_t N> inline
	T determinant(const mat<T, N, N>& aMatrix) {
	    // Create working copy
	    mat<T, N, N> LU = aMatrix;
	    T Det = T(1);
	
	    // Perform LU decomposition without pivoting
	    // The determinant will be the product of diagonal elements
	    for(std::size_t i = 0; i < N; ++i) {
	        if(std::abs(LU(i,i)) < std::numeric_limits<T>::epsilon()) {
	            return T(0);  // Matrix is singular
	        }
	
	        Det *= LU(i,i);
	
	        for(std::size_t j = i + 1; j < N; ++j) {
	            T Factor = LU(j,i) / LU(i,i);
	            for(std::size_t k = i + 1; k < N; ++k) {
	                LU(j,k) -= Factor * LU(i,k);
	            }
	        }
	    }
	
	    return Det;
	}

	// Generates a rotation matrix from a quaternion for an arbitrary vector.
	template <typename T> inline 
	mat<T, 4, 4> rotation(quaternion<T> aQuaternion) {
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
		T qs = abs2(q);
		if (qs < std::numeric_limits<T>::epsilon()) {
			return mat<T, 4, 4>(
				1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, 1.0
			);
		}
		T s = 1.0 / qs;
		T qa = q[0], qb = q[1], qc = q[2], qd = q[3];
		return mat<T, 4, 4>(
			1.0 - 2.0 * s * (qc * qc + qd * qd), 	2.0 * s * (qb * qc - qd * qa), 			2.0 * s * (qb * qd + qc * qa), 			0.0,
			2.0 * s * (qb * qc + qd * qa), 			1.0 - 2.0 * s * (qb * qb + qd * qd), 	2.0 * s * (qc * qd - qb * qa), 			0.0,
			2.0 * s * (qb * qd - qc * qa), 			2.0 * s * (qc * qd + qb * qa), 			1.0 - 2.0 * s * (qb * qb + qc * qc), 	0.0,
			0.0, 									0.0, 									0.0, 									1.0
		);
	}

	// Generates a rotation matrix from an angle amount to rotate and arbitrary vector around aAxis vector.
    template <typename T> inline 
	mat<T, 4, 4> rotation(T aAngle, vec<T, 3> aAxis) {
		vec<T, 3> UnitAxis = normalize(aAxis);
		quaternion<T> q = exp((aAngle / 2.0) * quaternion<T>(0.0, UnitAxis[0], UnitAxis[1], UnitAxis[2]));
		return rotation(q);
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
            0.0,            0.0,                    (Far / (Far - Near)),       (-(Far * Near) / (Far - Near)),
            0.0,            0.0,                    1.0,                                    0.0
        );
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
