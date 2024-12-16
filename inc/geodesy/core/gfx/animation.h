#pragma once
#ifndef GEODESY_CORE_GFX_ANIMATION_H
#define GEODESY_CORE_GFX_ANIMATION_H

#include <string>
#include <vector>
#include <map>

#include "../math.h"

// #include "mesh.h"
// #include "model.h"

struct aiAnimation;

namespace geodesy::core::gfx {

	class animation {
	public:

		// Template Code for Keyframe Animation
		template <typename T>
		struct key {
			double		Time; 		// Time in Ticks
			T			Value;
			key() : Time(0.0), Value() {}
		};

		struct node_anim {
			std::vector<key<math::vec<float, 3>>> 		PositionKey;
			std::vector<key<math::quaternion<float>>> 	RotationKey;
			std::vector<key<math::vec<float, 3>>> 		ScalingKey;
			math::mat<float, 4, 4> operator[](double aTime) const; // Expects Time in Ticks
		};

		// Not Implemented Yet
		struct mesh_anim {};

		std::string 						Name;
		float 								Weight;
		double 								Duration;				// Duration is in Ticks
		double 								TicksPerSecond;			// Conversion Factor for Ticks to Seconds
		std::map<std::string, node_anim> 	NodeAnimMap;
		std::map<std::string, mesh_anim> 	MeshAnimMap;

		animation();
		animation(const aiAnimation* aAnimation);

		const node_anim& operator[](std::string aNodeName) const;

	};

}

#endif // !GEODESY_CORE_GFX_ANIMATION_H
