#pragma once
#ifndef GEODESY_CORE_PHYS_MESH_H
#define GEODESY_CORE_PHYS_MESH_H

#include "../../config.h"
#include "../math.h"

namespace geodesy::core::phys {

    class mesh {
	public:

			// Informs how vertex groups are to be interpreted.
		enum primitive {
			POINT			,
			LINE			,
			LINE_STRIP		,
			TRIANGLE		,
			TRIANGLE_STRIP	,
			TRIANGLE_FAN	,
		};

		struct vertex {
			// enum input_rate {
			// 	VERTEX			= VK_VERTEX_INPUT_RATE_VERTEX,
			// 	INSTANCE		= VK_VERTEX_INPUT_RATE_INSTANCE
			// };
			struct weight {
				math::vec<uint, 4>				BoneID;
				math::vec<float, 4>				BoneWeight;
			};
			math::vec<float, 3> 			Position;
			math::vec<float, 3> 			Normal;
			math::vec<float, 3> 			Tangent;
			math::vec<float, 3> 			Bitangent;
			math::vec<float, 3> 			TextureCoordinate;
			math::vec<float, 4> 			Color;
			vertex();
		};

		struct topology {
			primitive						Primitive;
			std::vector<ushort>				Data16;
			std::vector<uint>				Data32;
		};

		struct bone {
			struct weight {
				uint							ID;
				float							Weight;
			};
			std::string						Name;
			std::vector<weight>				Vertex;
			math::mat<float, 4, 4>			Offset;
		};
		
		// Host Memory Objects
		std::string 					Name;
		float 							Mass;
		math::vec<float, 3> 			CenterOfMass;
		math::vec<float, 3> 			BoundingRadius;
		std::vector<vertex> 			Vertex;
		topology 						Topology;

		mesh();

		vertex operator[](size_t aIndex) const;
		vertex& operator[](size_t aIndex);

		// Calculates various properties of the mesh.
		math::vec<float, 3> center_of_mass() const;
		math::vec<float, 3> bounding_radius() const;

	};

}

#endif // !GEODESY_CORE_PHYS_COLLIDER_H