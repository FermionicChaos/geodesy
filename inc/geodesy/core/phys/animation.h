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

}

#endif // !GEODESY_CORE_PHYS_ANIMATION_H
