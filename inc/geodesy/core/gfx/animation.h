#pragma once
#ifndef GEODESY_CORE_GFX_ANIMATION_H
#define GEODESY_CORE_GFX_ANIMATION_H

#include <string>
#include <vector>
#include <map>

#include "../math.h"

// #include "mesh.h"
// #include "model.h"

namespace geodesy::core::gfx {

	class animation {
	public:

		template <typename T>
		struct key {
			double		Time;
			T			Value;
		};

		struct node_anim {
			std::vector<key<math::vec<float, 3>>> 			PositionKey;
			std::vector<key<math::quaternion<float>>> 		RotationKey;
			std::vector<key<math::vec<float, 3>>> 			ScalingKey;
			node_anim();
			math::mat<float, 4, 4> operator[](double aTime) const;
		};

		struct mesh_anim {

		};

		std::string 						Name;
		float 								Weight;
		float 								Duration;
		float 								TicksPerSecond;
		std::map<std::string, node_anim> 	NodeAnimMap;
		std::map<std::string, mesh_anim> 	MeshAnimMap;

		animation();

		node_anim operator[](std::string aNodeName);

	private:

	};

}

#endif // !GEODESY_CORE_GFX_ANIMATION_H
