#include <geodesy/core/gfx/node.h>

// Model Loading
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace geodesy::core::gfx {

	// ASSIMP RANT: This is fucking stupid, what's the point
	// of mesh instancing if the fucking bone animation is 
	// tied to the mesh object itself instead of its instance?
	//
	// For future development, the bone association which
	// is the information needed to animate a mesh should
	// be tied to its instance and not the actual mesh object
	// itself. Instancing only makes sense if you want to
	// reuse an already existing mesh multiple times, and thus
	// you instance it. It then logically follows that the 
	// bone animation data should be tied to its instance
	// rather than the mesh object itself. Tying the bone 
	// structure to the mesh object quite literally defeats
	// the point of mesh instancing, at least for animated
	// meshes. Rant Over

	// This works by not only copying over the mesh instancing index, but
	// also copies over the vertex weight data which informs how to deform
	// the mesh instance. Used for animations.

	node::node() {
		this->Type = node::GRAPHICS; // Default node type is graphics.
	}

	node::node(const aiScene* aScene, const aiNode* aNode, phys::node* aRoot, phys::node* aParent) : gfx::node() {
		// Sets up proper node hierarchy.
		if (aScene->mRootNode != aNode) {
			this->Root = aRoot;
			this->Parent = aParent;
		}

		// Recursively create child nodes.
		this->Child.resize(aNode->mNumChildren);
		for (size_t i = 0; i < this->Child.size(); i++) {
			// Check if aNode's children has any meshes.
			this->Child[i] = new gfx::node(aScene, aNode->mChildren[i], this->Root, this);
		}

		// Copy over non recurisve node data.
		this->Identifier = aNode->mName.C_Str();
		// TODO: Add Pos, Orientation, Scale
		this->Transformation = {
			aNode->mTransformation.a1, aNode->mTransformation.a2, aNode->mTransformation.a3, aNode->mTransformation.a4,
			aNode->mTransformation.b1, aNode->mTransformation.b2, aNode->mTransformation.b3, aNode->mTransformation.b4,
			aNode->mTransformation.c1, aNode->mTransformation.c2, aNode->mTransformation.c3, aNode->mTransformation.c4,
			aNode->mTransformation.d1, aNode->mTransformation.d2, aNode->mTransformation.d3, aNode->mTransformation.d4
		};
		// Copy over mesh instance data from assimp node hierarchy.
		this->MeshInstance.resize(aNode->mNumMeshes);
		for (int i = 0; i < aNode->mNumMeshes; i++) {
			// Get mesh index of mesh instance for this node.
			int MeshIndex 						= aNode->mMeshes[i];
			// Get mesh data from mesh index. Needed to acquire bone data applied to mesh instance.
			const aiMesh* Mesh 					= aScene->mMeshes[MeshIndex];
			// Copy over bone data for mesh instance.
			std::vector<mesh::bone> BoneData(Mesh->mNumBones);
			for (size_t j = 0; j < BoneData.size(); j++) {
				aiBone* Bone = Mesh->mBones[j];
				// Get the name of the bone.
				BoneData[j].Name 	= Bone->mName.C_Str();
				// Copy over vertex affecting weights per bone.
				BoneData[j].Vertex = std::vector<mesh::bone::weight>(Bone->mNumWeights);
				for (size_t k = 0; k < BoneData[j].Vertex.size(); k++) {
					BoneData[j].Vertex[k].ID 		= Bone->mWeights[k].mVertexId;
					BoneData[j].Vertex[k].Weight 	= Bone->mWeights[k].mWeight;
				}
				// Copy over offset matrix. This converts vertices from mesh space to bone space.
				BoneData[j].Offset = math::mat<float, 4, 4>(
					Bone->mOffsetMatrix.a1, Bone->mOffsetMatrix.a2, Bone->mOffsetMatrix.a3, Bone->mOffsetMatrix.a4,
					Bone->mOffsetMatrix.b1, Bone->mOffsetMatrix.b2, Bone->mOffsetMatrix.b3, Bone->mOffsetMatrix.b4,
					Bone->mOffsetMatrix.c1, Bone->mOffsetMatrix.c2, Bone->mOffsetMatrix.c3, Bone->mOffsetMatrix.c4,
					Bone->mOffsetMatrix.d1, Bone->mOffsetMatrix.d2, Bone->mOffsetMatrix.d3, Bone->mOffsetMatrix.d4
				);
			}
			// Load Mesh Instance Data
			this->MeshInstance[i] = mesh::instance(Mesh->mNumVertices, BoneData, MeshIndex, Mesh->mMaterialIndex, this->Root, this);
		}
	}

	node::node(std::shared_ptr<gpu::context> aContext, const node* aNode, phys::node* aRoot, phys::node* aParent) {		
		// Check if root node.
		if (aNode->Parent != nullptr) {
			this->Root = aRoot;
			this->Parent = aParent;
		}

		// Sets up proper node hierarchy.
		this->Child.resize(aNode->Child.size());
		for (size_t i = 0; i < aNode->Child.size(); i++) {
			// Check if aNode's children has any meshes.
			this->Child[i] = new gfx::node(aContext, (gfx::node*)aNode->Child[i], this->Root, this);
		}

		// Set device context.
		this->Context = aContext;
		this->copy_data(aNode);

	}

	node::~node() {
		this->MeshInstance.clear();
	}

	void node::copy_data(const phys::node* aNode) {
		// Copy over the base class node data.
		phys::node::copy_data(aNode);

		// Copy over mesh instance data.
		this->MeshInstance.resize(((gfx::node*)aNode)->MeshInstance.size());
		for (size_t i = 0; i < this->MeshInstance.size(); i++) {
			this->MeshInstance[i] = gfx::mesh::instance(this->Context, ((gfx::node*)aNode)->MeshInstance[i], this->Root, this);
		}
	}

	void node::update(
		double aDeltaTime, 
		double aTime, 
		const std::vector<float>& aAnimationWeight, 
		const std::vector<phys::animation>& aPlaybackAnimation
	) {

		// Call the base class update function to update the node data.
		phys::node::update(aDeltaTime, aTime, aAnimationWeight, aPlaybackAnimation);

		// For each mesh instance, and for each bone, update the 
		// bone transformations according to their respective
		// animation object.
		for (mesh::instance& MI : MeshInstance) {
			// This is only used to tranform mesh instance vertices without bone animation.
			// Update Bone Buffer Date GPU side.
			mesh::instance::uniform_data* UniformData = (mesh::instance::uniform_data*)MI.UniformBuffer->Ptr;
			UniformData->Transform = this->transform(aAnimationWeight, aPlaybackAnimation, aTime);
			for (size_t i = 0; i < MI.Bone.size(); i++) {
				UniformData->BoneTransform[i] = this->Root->find(MI.Bone[i].Name)->transform(aAnimationWeight, aPlaybackAnimation, aTime);
			}
		}
	}

	// Counts the total number of mesh references in the tree.
	size_t node::instance_count() {
		// First linearize the hierarchy.
		std::vector<phys::node*> Nodes = this->linearize();

		// Find all graphics nodes in the hierarchy, and add mesh instances to the count.
		size_t Count = 0;
		for (phys::node* N : Nodes) {
			if (N->Type == phys::node::GRAPHICS) {
				gfx::node* GNode = static_cast<gfx::node*>(N);
				Count += GNode->MeshInstance.size();
			}
		}

		return Count;
	}

	// Gather all mesh instances in the hierarchy.
	std::vector<gfx::mesh::instance*> node::gather_instances() {
		std::vector<gfx::mesh::instance*> Instances;

		// First linearize the hierarchy.
		std::vector<phys::node*> Nodes = this->linearize();

		// Find all graphics nodes in the hierarchy, and add mesh instances to the count.
		for (phys::node* N : Nodes) {
			if (N->Type == phys::node::GRAPHICS) {
				gfx::node* GNode = static_cast<gfx::node*>(N);
				for (gfx::mesh::instance& MI : GNode->MeshInstance) {
					Instances.push_back(&MI);
				}
			}
		}

		return Instances;
	}

}
