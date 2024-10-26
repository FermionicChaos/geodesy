
#pragma once
#ifndef GEODESY_CORE_MATH_CONFIG_H
#define GEODESY_CORE_MATH_CONFIG_H

#include <array>
#include <type_traits>
#include <ostream>
#include <cmath>

namespace geodesy::core::math {

	// ------------------------- Vector Space Control Logic ------------------------- //

	// This section of the math library allows flow control for the logic of the vector spaces
	// on classes. A lot of objects follows the logic of vector space, and are not conventionally
	// considered vectors in CS terminology. Matrices are also considered a vector space, and
	// closed under addition and scalar multiplication. This list o

	/*
	
	// Forward declarations
	template <typename T, std::size_t N> class vec;
	template <typename T, std::size_t M, std::size_t N> class mat;
	template <typename T> class complex;
	template <typename T> class quaternion;
	template <typename domain_type, std::size_t dimension_count, typename range_type> class field;

	// Type trait to detect vec
	template <typename T>
	struct is_vec : std::false_type {};

	template <typename T, std::size_t N>
	struct is_vec<vec<T, N>> : std::true_type {};

	// Type trait to detect mat
	template <typename T>
	struct is_mat : std::false_type {};

	template <typename T, std::size_t M, std::size_t N>
	struct is_mat<mat<T, M, N>> : std::true_type {};

	// Type trait to detect complex
	template <typename T>
	struct is_complex : std::false_type {};

	template <typename T>
	struct is_complex<complex<T>> : std::true_type {};

	// Type trait to detect quaternion
	template <typename T>
	struct is_quaternion : std::false_type {};

	template <typename T>
	struct is_quaternion<quaternion<T>> : std::true_type {};

	// Type trait to detect field
	template <typename T>
	struct is_field : std::false_type {};

	template <typename domain_type, std::size_t dimension_count, typename range_type>
	struct is_field<field<domain_type, dimension_count, range_type>> : std::true_type {};

	// Type trait to detect any of the allowed classes
	template <typename T>
	struct is_vector_space_type : std::disjunction<is_vec<T>, is_mat<T>, is_complex<T>, is_quaternion<T>, is_field<T>> {};

	// ------------------------- Vector Space Operators ------------------------- //

	template <typename vst, typename = std::enable_if_t<is_vector_space_type<vst>::value>> inline
	vst operator+(const vst& aLhs, const vst& aRhs) {
		vst Out;
		for (std::size_t i = 0; i < aLhs.size(); i++) {
			Out[i] = aLhs[i] + aRhs[i];
		}
		return Out;
	}

	template <typename vst, typename = std::enable_if_t<is_vector_space_type<vst>::value>> inline
	vst operator-(const vst& aLhs, const vst& aRhs) {
		vst Out;
		for (std::size_t i = 0; i < aLhs.size(); i++) {
			Out[i] = aLhs[i] - aRhs[i];
		}
		return Out;
	}

	template <typename vst, typename st, typename = std::enable_if_t<is_vector_space_type<vst>::value>> inline
	vst operator*(const vst& aLhs, const st& aRhs) {
		vst Out;
		for (std::size_t i = 0; i < aLhs.size(); i++) {
			Out[i] = aLhs[i] * aRhs;
		}
		return Out;
	}

	// TODO: Figure out how to prevent collision? (It works now? wtf?)
	template <typename vst, typename st, typename = std::enable_if_t<is_vector_space_type<vst>::value>> inline
	vst operator*(const st& aLhs, const vst& aRhs) {
		return aRhs * aLhs;
	}

	template <typename vst, typename T, typename = std::enable_if_t<is_vector_space_type<vst>::value>> inline
	vst operator/(const vst& aLhs, const T& aRhs) {
		vst Out;
		for (std::size_t i = 0; i < aLhs.size(); i++) {
			Out[i] = aLhs[i] / aRhs;
		}
		return Out;
	}

	template <typename vst, typename = std::enable_if_t<is_vector_space_type<vst>::value>> inline
	vst& operator+=(vst& aLhs, const vst& aRhs) {
		aLhs = aLhs + aRhs;
		return aLhs;
	}

	template <typename vst, typename = std::enable_if_t<is_vector_space_type<vst>::value>> inline
	vst& operator-=(vst& aLhs, const vst& aRhs) {
		aLhs = aLhs - aRhs;
		return aLhs;
	}

	template <typename vst, typename st, typename = std::enable_if_t<is_vector_space_type<vst>::value>> inline
	vst& operator*=(vst& aLhs, st aRhs) {
		aLhs = aLhs * aRhs;
		return aLhs;
	}

	template <typename vst, typename st, typename = std::enable_if_t<is_vector_space_type<vst>::value>> inline
	vst& operator/=(vst& aLhs, st aRhs) {
		aLhs = aLhs / aRhs;
		return aLhs;
	}
	*/

}

#endif // !GEODESY_CORE_MATH_CONFIG_H
