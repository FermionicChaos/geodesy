#include <geodesy/core/gfx/mesh.h>

#include <vector>
#include <algorithm>

#define MAX_BONE_COUNT 256

namespace geodesy::core::gfx {

	using namespace gcl;

	namespace {

		struct mesh_instance_ubo_data {
			alignas(16) math::mat<float, 4, 4> Transform;
			alignas(16) math::mat<float, 4, 4> BoneTransform[MAX_BONE_COUNT];
			alignas(16) math::mat<float, 4, 4> BoneOffset[MAX_BONE_COUNT];
			mesh_instance_ubo_data();
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
						// Uses insert sort to keep the weights sorted from largest to smallest.
						// {  0,  1   2,  3, 4 }
						// { 69, 25, 21, 10, 4 } <- 15
						// Insert at index 3
						// { 69, 25, 21, 15, 10, 4 }
						for (size_t a = 0; a < VertexBoneWeight.size(); a++) {
							if (VertexBoneWeight[a].Weight < Bone[j].Vertex[k].Weight) {
								VertexBoneWeight.insert(VertexBoneWeight.begin() + a, Bone[j].Vertex[k]);
								break;
							}
						}
					}
				}
			}

			// Will take the first and largest elements.
			for (size_t j = 0; j < std::min((size_t)4, VertexBoneWeight.size()); j++) {
				Vertex[i].BoneID[j] 		= VertexBoneWeight[j].ID;
				Vertex[i].BoneWeight[j] 	= VertexBoneWeight[j].Weight;
			}
		}
	}

	mesh::instance::instance(std::shared_ptr<gcl::context> aContext, const instance& aInstance) {
		this->Index 		= aInstance.Index;
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
		mesh_instance_ubo_data MeshInstanceUBOData;
		MeshInstanceUBOData.Transform = aInstance.Transform;
		for (size_t i = 0; i < aInstance.Bone.size(); i++) {
			MeshInstanceUBOData.BoneTransform[i] 	= aInstance.Bone[i].Transform;
			MeshInstanceUBOData.BoneOffset[i] 		= aInstance.Bone[i].Offset;
		}
        this->UniformBuffer = Context->create_buffer(UBCI, sizeof(mesh_instance_ubo_data), &MeshInstanceUBOData);
		this->UniformBuffer->map_memory(0, sizeof(mesh_instance_ubo_data));
	}

	void mesh::instance::update(double DeltaTime) {
		// The goal here is to update the Bone Buffer in the vertex shader.
		std::vector<math::mat<float, 4, 4>> TransformData(1 + 2*Bone.size());

		// This is the Mesh Instance Transform. This transform is applied to mesh space vertices
		// directly is no bone structure is altering the vertices. It takes the mesh space vertices
		// and transforms them to root model space. This is directly applied to the vertices.
		TransformData[0] = this->Transform;

		// This is the current transform data for each bone modified by the current animations structure.
		for (size_t i = 0; i < Bone.size(); i++) {
			TransformData[i + 1] = Bone[i].Transform;
		}

		// This is the offset matrix data that transforms vertices from mesh space to bone space.
		// Needed to transform vertices from bone space to mesh space for animated bones to animate
		// the mesh.
		for (size_t i = 0; i < Bone.size(); i++) {
			TransformData[i + Bone.size() + 1] = Bone[i].Offset;
		}

		// Send data to the GPU Uniform Buffer.
		this->UniformBuffer->write(0, TransformData.data(), 0, TransformData.size() * sizeof(math::mat<float, 4, 4>));
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
