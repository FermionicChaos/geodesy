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
		this->Mass						= 1.0f;
		this->CenterOfMass 				= { 0.0f, 0.0f, 0.0f };
		this->BoundingRadius			= 0.0f;
	}

	mesh::vertex mesh::operator[](size_t aIndex) const {
		return this->Vertex[aIndex];
	}

	mesh::vertex& mesh::operator[](size_t aIndex) {
		return this->Vertex[aIndex];
	}

	// Calculates the center of mass of the mesh.
	math::vec<float, 3> mesh::center_of_mass() const {
		math::vec<float, 3> COM = math::vec<float, 3>(0.0f, 0.0f, 0.0f);
		for (const auto& Vertex : this->Vertex) {
			COM += Vertex.Position;
		}
		COM /= static_cast<float>(this->Vertex.size());
		return COM;
	}

	// Determines the bounding radius of the mesh.
	float mesh::bounding_radius() const {
		float Radius = 0.0f;
		math::vec<float, 3> COM = this->center_of_mass();
		for (const auto& Vertex : this->Vertex) {
			float Distance = math::length(Vertex.Position - COM);
			if (Distance > Radius) {
				Radius = Distance;
			}
		}
		return Radius;
	}

}
