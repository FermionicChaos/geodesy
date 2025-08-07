#pragma once
#ifndef GEODESY_CORE_PHYS_ANIMATION_H
#define GEODESY_CORE_PHYS_ANIMATION_H

#include <string>
#include <vector>
#include <map>

#include "../../config.h"
#include "../math.h"

// #include "mesh.h"
// #include "model.h"

struct aiAnimation;

namespace geodesy::core::phys {

	class animation {
	public:

		// Template Code for Keyframe Animation
		template <typename T>
		struct key {
			double		Time; 		// Time in Ticks
			T			Value;
			key() : Time(0.0), Value() {}
		};

		// Determines transform override based on time, for a singular node.
		struct node {
			std::vector<key<math::vec<float, 3>>> 		PositionKey;
			std::vector<key<math::quaternion<float>>> 	RotationKey;
			std::vector<key<math::vec<float, 3>>> 		ScalingKey;
			math::mat<float, 4, 4> operator[](double aTime) const; // Expects Time in Ticks
			bool exists() const;
		};

		// Determiens and overrides mesh vertices over time.
		struct mesh {};

		std::string 						Name;
		double 								Start;
		double 								Stop;
		double 								TicksPerSecond;			// Conversion Factor for Ticks to Seconds
		std::map<std::string, node> 		NodeAnimMap;
		std::map<std::string, mesh> 		MeshAnimMap;

		animation();
		animation(const aiAnimation* aAnimation);

		const node& operator[](std::string aNodeName) const;

	};

	// Calculates the full transformation matrix from position, rotation, and scale states.
	template <typename T> inline
	math::mat<T, 4, 4> calculate_transform(math::vec<T, 3> aPosition, math::quaternion<T> aOrientation, math::vec<T, 3> aScale) {
		math::mat<T, 4, 4> Translation = {
			1.0f, 		0.0f, 		0.0f, 			aPosition[0],
			0.0f, 		1.0f, 		0.0f, 			aPosition[1],
			0.0f, 		0.0f, 		1.0f, 			aPosition[2],
			0.0f, 		0.0f, 		0.0f, 			1.0f
		};
		math::mat<T, 4, 4> Orientation = math::mat<T, 4, 4>(aOrientation);
		math::mat<T, 4, 4> Scale = {
			aScale[0], 	0.0f, 		0.0f, 			0.0f,
			0.0f, 		aScale[1], 	0.0f, 			0.0f,
			0.0f, 		0.0f, 		aScale[2], 		0.0f,
			0.0f, 		0.0f, 		0.0f, 			1.0f
		};
		return Translation * Orientation * Scale;
	}

}

#endif // !GEODESY_CORE_PHYS_ANIMATION_H
