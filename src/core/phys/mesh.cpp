#include <geodesy/core/phys/mesh.h>

namespace geodesy::core::phys {

	mesh::vertex::vertex() {
		this->Position					= math::vec<float, 3>(0.0f, 0.0f, 0.0f);
		this->Normal					= math::vec<float, 3>(0.0f, 0.0f, 0.0f);
		this->Tangent					= math::vec<float, 3>(0.0f, 0.0f, 0.0f);
		this->Bitangent					= math::vec<float, 3>(0.0f, 0.0f, 0.0f);
		// this->BoneID					= math::vec<uint, 4>(UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX);
		// this->BoneWeight				= math::vec<float, 4>(0.0f, 0.0f, 0.0f, 0.0f);
		this->TextureCoordinate			= math::vec<float, 3>(0.0f, 0.0f, 0.0f);
		this->Color						= math::vec<float, 4>(0.0f, 0.0f, 0.0f, 0.0f);
	}

	mesh::mesh() {
		this->Name						= "";
		this->BoundingRadius			= 0.0f;
		this->Mass						= 0.0f;
	}

	mesh::vertex mesh::operator[](size_t aIndex) const {
		return this->Vertex[aIndex];
	}

	mesh::vertex& mesh::operator[](size_t aIndex) {
		return this->Vertex[aIndex];
	}
}
