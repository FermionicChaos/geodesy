#include <geodesy/core/gfx/mesh.h>

#include <vector>
#include <algorithm>

// Model Loading
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define MAX_BONE_COUNT 256

namespace geodesy::core::gfx {

	using namespace gcl;

	namespace {

		struct mesh_instance_ubo_data {
			alignas(16) math::mat<float, 4, 4> Transform;
			alignas(16) math::mat<float, 4, 4> BoneTransform[MAX_BONE_COUNT];
			alignas(16) math::mat<float, 4, 4> BoneOffset[MAX_BONE_COUNT];
			mesh_instance_ubo_data();
			mesh_instance_ubo_data(const mesh::instance* aInstance);
		};

		mesh_instance_ubo_data::mesh_instance_ubo_data() {
			Transform = {
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			};
			for (size_t i = 0; i < MAX_BONE_COUNT; i++) {
				BoneTransform[i] = {
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				};
				BoneOffset[i] = {
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				};
			}
		}

		mesh_instance_ubo_data::mesh_instance_ubo_data(const mesh::instance* aInstance) {
			this->Transform = aInstance->Transform;
			for (size_t i = 0; i < aInstance->Bone.size(); i++) {
				this->BoneTransform[i] = aInstance->Bone[i].Transform;
				this->BoneOffset[i] = aInstance->Bone[i].Offset;
			}
		}

	}

	mesh::instance::instance() {
		this->Index 	= -1;
		this->Context 	= nullptr;
	}

	mesh::instance::instance(int aMeshIndex, math::mat<float, 4, 4> aTransform, uint aVertexCount, const std::vector<bone>& aBoneData, uint aMaterialIndex) : instance() {
		this->Index 		= aMeshIndex;
		this->Transform 	= aTransform;
		this->Vertex 		= std::vector<vertex::weight>(aVertexCount);
		this->Bone 			= aBoneData;
		this->MaterialIndex = aMaterialIndex;
		// Generate the corresponding vertex buffer which will supply the mesh
		// instance the needed bone animation data.
		for (size_t i = 0; i < Vertex.size(); i++) {
			Vertex[i].BoneID 		= math::vec<uint, 4>(UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX);
			Vertex[i].BoneWeight 	= math::vec<float, 4>(0.0f, 0.0f, 0.0f, 0.0f);

			// Group Bone Weights By Vertex Index.
			std::vector<bone::weight> VertexBoneWeight;
			for (size_t j = 0; j < Bone.size(); j++) {
				for (size_t k = 0; k < Bone[j].Vertex.size(); k++) {
					// If the vertex index matches the bone vertex index, then then copy over.
					if (i == Bone[j].Vertex[k].ID) {
						// Store Bone Index j and Weight.
						bone::weight NewVertexWeight = { (uint)j, Bone[j].Vertex[k].Weight };
						if (VertexBoneWeight.size() > 0) {
							// Uses insert sort to keep the weights sorted from largest to smallest.
							// {  0,  1   2,  3, 4 }
							// { 69, 25, 21, 10, 4 } <- 15
							// Insert at index 3
							// { 69, 25, 21, 15, 10, 4 }
							for (size_t a = 0; a < VertexBoneWeight.size(); a++) {
								if (VertexBoneWeight[a].Weight < NewVertexWeight.Weight) {
									VertexBoneWeight.insert(VertexBoneWeight.begin() + a, NewVertexWeight);
									break;
								}
							}
						}
						else {
							// Add First Element.
							VertexBoneWeight.push_back(NewVertexWeight);
						}
					}
				}
			}

			// Will take the first and largest elements. Disable this section if you
			// want to make sure bind pose bones are equal to default mesh transform.
			float TotalVertexWeight = 0.0f;
			for (size_t j = 0; j < std::min((size_t)4, VertexBoneWeight.size()); j++) {
				if (VertexBoneWeight[j].Weight == 0.0f) {
					break;
				}
				Vertex[i].BoneID[j] 		= VertexBoneWeight[j].ID;
				Vertex[i].BoneWeight[j] 	= VertexBoneWeight[j].Weight;
				TotalVertexWeight 			+= VertexBoneWeight[j].Weight;
			}
			Vertex[i].BoneWeight /= TotalVertexWeight;
		}
	}

	mesh::instance::instance(std::shared_ptr<gcl::context> aContext, const instance& aInstance) {
		this->Index 		= aInstance.Index;
		this->Transform 	= aInstance.Transform;
		this->Vertex 		= aInstance.Vertex;
		this->Bone 			= aInstance.Bone;
		this->MaterialIndex = aInstance.MaterialIndex;
		this->Context 		= aContext;

		// Create Vertex Weight Buffer
        buffer::create_info VBCI;
        VBCI.Memory = device::memory::DEVICE_LOCAL;
        VBCI.Usage = buffer::usage::VERTEX | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;
        this->VertexWeightBuffer = Context->create_buffer(VBCI, Vertex.size() * sizeof(vertex::weight), Vertex.data());

        // Create Mesh Instance Uniform Buffer
        buffer::create_info UBCI;
        UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
        UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;

		mesh_instance_ubo_data MeshInstanceUBOData = mesh_instance_ubo_data(this);
        this->UniformBuffer = Context->create_buffer(UBCI, sizeof(mesh_instance_ubo_data), &MeshInstanceUBOData);
		this->UniformBuffer->map_memory(0, sizeof(mesh_instance_ubo_data));
	}

	void mesh::instance::update(double DeltaTime) {
		// Convert uniform buffer pointer into data structure for writing.
		mesh_instance_ubo_data* MeshInstanceUBOData = (mesh_instance_ubo_data*)this->UniformBuffer->Ptr;
		// Carry over mesh instance node transform.
		MeshInstanceUBOData->Transform = this->Transform;
		// Transfer all bone transformations to the uniform buffer.
		for (size_t i = 0; i < Bone.size(); i++) {
			MeshInstanceUBOData->BoneTransform[i] = Bone[i].Transform;
		}
	}

	mesh::mesh(const aiMesh* aMesh) {
		// Size Vertex Buffer to hold all vertices.
		Vertex = std::vector<vertex>(aMesh->mNumVertices);
		// Load Vertex Data
		for (size_t i = 0; i < Vertex.size(); i++) {
			// Check if mesh has positions.
			if (aMesh->HasPositions()) {
				Vertex[i].Position = {
					aMesh->mVertices[i].x,
					aMesh->mVertices[i].y,
					aMesh->mVertices[i].z
				};
			}
			// Check if mesh has normals.
			if (aMesh->HasNormals()) {
				Vertex[i].Normal = {
					aMesh->mNormals[i].x,
					aMesh->mNormals[i].y,
					aMesh->mNormals[i].z
				};
			}
			// Check if mesh has tangents and bitangents.
			if (aMesh->HasTangentsAndBitangents()) {
				Vertex[i].Tangent = {
					aMesh->mTangents[i].x,
					aMesh->mTangents[i].y,
					aMesh->mTangents[i].z
				};
				Vertex[i].Bitangent = {
					aMesh->mBitangents[i].x,
					aMesh->mBitangents[i].y,
					aMesh->mBitangents[i].z
				};
			}

			// ^Save for later, not useful now. Just commit it.
			// // Filter normal components and insure that tangent is perpendicular to normal.
			// Vertex[i].Tangent -= (Vertex[i].Normal * Vertex[i].Tangent) * Vertex[i].Normal;
			// Vertex[i].Tangent = math::normalize(Vertex[i].Tangent);
			// // Generate bitangent from tangent and normal.
			// Vertex[i].Bitangent = Vertex[i].Normal ^ Vertex[i].Tangent;

			// -------------------- Texturing & Coloring -------------------- //

			// TODO: Support multiple textures
			// Take only the first element of the Texture Coordinate Array.
			for (int k = 0; k < 1 /* AI_MAX_NUMBER_OF_TEXTURECOORDS */; k++) {
				if (aMesh->HasTextureCoords(k)) {
					Vertex[i].TextureCoordinate = {
						aMesh->mTextureCoords[k][i].x,
						aMesh->mTextureCoords[k][i].y,
						aMesh->mTextureCoords[k][i].z
					};
				}
			}
			// Take an average of all the Colors associated with Vertex.
			for (int k = 0; k < 1 /*AI_MAX_NUMBER_OF_COLOR_SETS*/; k++) {
				if (aMesh->HasVertexColors(k)) {
					Vertex[i].Color += {
						aMesh->mColors[k][i].r,
						aMesh->mColors[k][i].g,
						aMesh->mColors[k][i].b,
						aMesh->mColors[k][i].a
					};
				}
			}
			// VertexData[j].Color /= AI_MAX_NUMBER_OF_COLOR_SETS;
		}

		// Load Index Data
		if (aMesh->mNumVertices <= (1 << 16)) {
			// Implies that the mesh has less than 2^16 vertices, only 16 bit indices are needed.
			Topology.Data16 = std::vector<ushort>(aMesh->mNumFaces * 3);
			for (size_t i = 0; i < aMesh->mNumFaces; i++) {
				// Skip non triangle indices.
				if (aMesh->mFaces[i].mNumIndices != 3) {
					continue;
				}
				Topology.Data16[3*i + 0] = (ushort)aMesh->mFaces[i].mIndices[0];
				Topology.Data16[3*i + 1] = (ushort)aMesh->mFaces[i].mIndices[1];
				Topology.Data16[3*i + 2] = (ushort)aMesh->mFaces[i].mIndices[2];
			} 
		}
		else {
			// Implies that the mesh has more than 2^16 vertices, 32 bit indices are needed.
			Topology.Data32 = std::vector<uint>(aMesh->mNumFaces * 3);
			for (size_t i = 0; i < aMesh->mNumFaces; i++) {
				// Skip non triangle indices.
				if (aMesh->mFaces[i].mNumIndices != 3) {
					continue;
				}
				Topology.Data32[3*i + 0] = (uint)aMesh->mFaces[i].mIndices[0];
				Topology.Data32[3*i + 1] = (uint)aMesh->mFaces[i].mIndices[1];
				Topology.Data32[3*i + 2] = (uint)aMesh->mFaces[i].mIndices[2];
			}
		}
	}

	mesh::mesh(std::shared_ptr<gcl::context> aContext, const std::vector<vertex>& aVertexData, const topology& aTopologyData) : phys::mesh() {
		this->Vertex 	= aVertexData;
		this->Topology	= aTopologyData;
		if (aContext != nullptr) {
			this->Context = aContext;
			// Vertex Buffer Creation Info
			gcl::buffer::create_info VBCI;
			VBCI.Memory = device::memory::DEVICE_LOCAL;
			VBCI.Usage = buffer::usage::VERTEX | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;
			VBCI.ElementCount = Vertex.size();
			// Index buffer Create Info
			gcl::buffer::create_info IBCI;
			IBCI.Memory = device::memory::DEVICE_LOCAL;
			IBCI.Usage = buffer::usage::INDEX | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;
			IBCI.ElementCount = Topology.Data16.size() > 0 ? Topology.Data16.size() : Topology.Data32.size();
			// Index buffer Create Info
			// gcl::buffer::create_info BBCI(
			// 	device::memory::HOST_VISIBLE | device::memory::DEVICE_LOCAL,
			// 	buffer::usage::INDEX | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST
			// );
			//VertexBuffer = std::make_shared<buffer>(Context, VBCI, Vertex.size() * sizeof(vertex), (void*)Vertex.data());
			VertexBuffer = Context->create_buffer(VBCI, Vertex.size() * sizeof(vertex), Vertex.data());
			// wtf this is legal?
			// IndexBuffer = std::vector<gcl::buffer>(1, buffer(Context, IBCI, aIndexData.size() * sizeof(ushort), (void*)aIndexData.data()));
			if (this->Vertex.size() <= (1 << 16)) {
				//IndexBuffer = std::make_shared<buffer>(Context, IBCI, Topology.Data16.size() * sizeof(ushort), (void*)Topology.Data16.data());
				IndexBuffer = Context->create_buffer(IBCI, Topology.Data16.size() * sizeof(ushort), Topology.Data16.data());
			}
			else {
				// IndexBuffer = std::make_shared<buffer>(Context, IBCI, Topology.Data32.size() * sizeof(uint), (void*)Topology.Data32.data());
				IndexBuffer = Context->create_buffer(IBCI, Topology.Data32.size() * sizeof(uint), Topology.Data32.data());
			}
		}
	}

	mesh::mesh(std::shared_ptr<gcl::context> aContext, std::shared_ptr<mesh> aMesh) : mesh(aContext, aMesh->Vertex, aMesh->Topology) {}

	// void mesh::draw(VkCommandBuffer aCommandBuffer, std::shared_ptr<gcl::pipeline> aPipeline, std::shared_ptr<gcl::framebuffer> aFramebuffer, std::shared_ptr<gcl::descriptor::array> aDescriptorArray) {
	// 	aPipeline->draw(aCommandBuffer, aFramebuffer, { VertexBuffer }, IndexBuffer, aDescriptorArray);
	// }

}
