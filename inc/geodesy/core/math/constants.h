#pragma once
#ifndef GEODESY_CORE_MATH_CONSTANTS_H
#define GEODESY_CORE_MATH_CONSTANTS_H

#include "config.h"
#include "type.h"

namespace geodesy::core::math {
	namespace constant {
		constexpr double pi = 3.14159265358979323846;
		constexpr double e = 2.71828182845904523536;
	}

	template <typename T>
	T radians(T aDegrees) {
		return aDegrees * constant::pi / 180.0;
	}

	template <typename T>
	T degrees(T aRadians) {
		return aRadians * 180.0 / constant::pi;
	}
	
}

#endif // !GEODESY_CORE_MATH_CONSTANTS_H
