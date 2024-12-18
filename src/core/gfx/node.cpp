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

	// This function recreates the node hierarchy from the assimp node hierarchy.
	static void traverse_and_fill(node& aModelNode, const aiNode* aAssimpNode) {
		// Copy over traversal data
		aModelNode.Child.resize(aAssimpNode->mNumChildren);
		for (size_t i = 0; i < aModelNode.Child.size(); i++) {
			aModelNode[i].Root 		= aModelNode.Root;
			aModelNode[i].Parent 	= &aModelNode;
			traverse_and_fill(aModelNode[i], aAssimpNode->mChildren[i]);
		}

		// Copy over non recurisve node data.
		aModelNode.Name 		= aAssimpNode->mName.C_Str();
		aModelNode.Transformation = math::mat<float, 4, 4>(
			aAssimpNode->mTransformation.a1, aAssimpNode->mTransformation.a2, aAssimpNode->mTransformation.a3, aAssimpNode->mTransformation.a4,
			aAssimpNode->mTransformation.b1, aAssimpNode->mTransformation.b2, aAssimpNode->mTransformation.b3, aAssimpNode->mTransformation.b4,
			aAssimpNode->mTransformation.c1, aAssimpNode->mTransformation.c2, aAssimpNode->mTransformation.c3, aAssimpNode->mTransformation.c4,
			aAssimpNode->mTransformation.d1, aAssimpNode->mTransformation.d2, aAssimpNode->mTransformation.d3, aAssimpNode->mTransformation.d4
		);
	}

	// This function loads the mesh instance data from the assimp node hierarchy. This is done separately
	// to insure that the node hierarchy is fully created before the mesh instance data is loaded.
	static void load_mesh_instance_data(const aiScene* aScene, node& aModelNode, const aiNode* aAssimpNode) {

		// Iterate through hierarchy and load mesh instance data.
		for (size_t i = 0; i < aModelNode.Child.size(); i++) {
			load_mesh_instance_data(aScene, aModelNode[i], aAssimpNode->mChildren[i]);
		}

		aModelNode.MeshInstance.resize(aAssimpNode->mNumMeshes);
		for (int i = 0; i < aAssimpNode->mNumMeshes; i++) {
			// Get mesh index of mesh instance for this node.
			int MeshIndex 						= aAssimpNode->mMeshes[i];
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
				// Copy over bone transformation matrix. This converts vertices from bone space to model space.
				BoneData[j].Transform = aModelNode.Root->find(BoneData[j].Name)->transform();
				// Copy over offset matrix. This converts vertices from mesh space to bone space.
				BoneData[j].Offset = math::mat<float, 4, 4>(
					Bone->mOffsetMatrix.a1, Bone->mOffsetMatrix.a2, Bone->mOffsetMatrix.a3, Bone->mOffsetMatrix.a4,
					Bone->mOffsetMatrix.b1, Bone->mOffsetMatrix.b2, Bone->mOffsetMatrix.b3, Bone->mOffsetMatrix.b4,
					Bone->mOffsetMatrix.c1, Bone->mOffsetMatrix.c2, Bone->mOffsetMatrix.c3, Bone->mOffsetMatrix.c4,
					Bone->mOffsetMatrix.d1, Bone->mOffsetMatrix.d2, Bone->mOffsetMatrix.d3, Bone->mOffsetMatrix.d4
				);
			}
			// Load Mesh Instance Data
			aModelNode.MeshInstance[i] = mesh::instance(MeshIndex, aModelNode.transform(), Mesh->mNumVertices, BoneData, Mesh->mMaterialIndex);
		}
	}

	node::node() {
		this->Root				= this;
		this->Parent			= nullptr;
		this->Name				= "";
		this->Transformation 	= {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	}

	node::node(const aiScene* aScene, const aiNode* aNode) : node() {
		// Setup Node Hierarchy first.
		this->Root = this;
		this->Parent = nullptr;
		traverse_and_fill(*this, aNode);
		// Load Mesh instance data after.
		load_mesh_instance_data(aScene, *this, aNode);
	}

	node::node(std::shared_ptr<gcl::context> aContext, const node& aNode) : node() {
		// TODO: Fix recursive node hierarchy copy.
		this->Name 				= aNode.Name;
		this->Transformation 	= aNode.Transformation;
		this->MeshInstance 		= std::vector<mesh::instance>(aNode.MeshInstance.size());
		for (size_t i = 0; i < aNode.MeshInstance.size(); i++) {
			// Create GPU Mesh Instances
			this->MeshInstance[i] = mesh::instance(aContext, aNode.MeshInstance[i]);
		}
		this->Child 	= std::vector<node>(aNode.Child.size());
		for (size_t i = 0; i < aNode.Child.size(); i++) {
			this->Child[i].Root 	= this->Root;
			this->Child[i].Parent 	= this;
			this->Child[i] 			= node(aContext, aNode.Child[i]);
		}
	}

	node::node(const node& aInput) : node() {
		*this = aInput;
	}

	node::node(node&& aInput) noexcept : node() {
		*this = aInput;
	}

	node::~node() {
		this->Child.clear();
		this->MeshInstance.clear();
	}

	node& node::operator=(const node& aRhs) {
		if (this == &aRhs) return *this;
		this->Name					= aRhs.Name;
		this->Transformation		= aRhs.Transformation;
		this->MeshInstance 			= aRhs.MeshInstance;
		this->Child 				= std::vector<node>(aRhs.Child.size());
		for (size_t i = 0; i < aRhs.Child.size(); i++) {
			this->Child[i].Root 		= this->Root;
			this->Child[i].Parent 		= this;
			this->Child[i] 				= aRhs.Child[i];
		}
		return *this;
	}

	node& node::operator=(node&& aRhs) noexcept {
		*this = aRhs;
		return *this;
	}

	node& node::operator[](int aIndex) {
		return this->Child[aIndex];
	}

	node* node::find(std::string aName) {
		node* Node = nullptr;
		if (this->Name == aName) {
			Node = this;
		}
		else {
			for (node& Chd : Child) {
				Node = Chd.find(aName);
				if (Node != nullptr) {
					break;
				}
			}
		}
		return Node;
	}

	void node::update(const std::vector<float>& aAnimationWeight, const std::vector<animation>& aPlaybackAnimation, double aTime) {
		// Work done here will start at the root
		// of the node hierarchy.

		// For each mesh instance, and for each bone, update the 
		// bone transformations according to their respective
		// animation object.
		for (mesh::instance& MI : MeshInstance) {
			// This is only used to tranform mesh instance vertices without bone animation.
			MI.Transform = this->transform(aAnimationWeight, aPlaybackAnimation, aTime);
			for (mesh::bone& B : MI.Bone) {
				// Update Bone Transformations from Bone Hierarchies
				B.Transform = this->Root->find(B.Name)->transform(aAnimationWeight, aPlaybackAnimation, aTime);
			}
			// Update Bone Buffer Date GPU side.
			MI.update(aTime);
		}

		// Go to child nodes and update children nodes.
		for (node& Chd : Child) {
			Chd.update(aAnimationWeight, aPlaybackAnimation, aTime);
		}
	}

	size_t node::node_count() const {
		size_t TotalCount = 1;
		for (const node& Chd : Child) {
			TotalCount += Chd.node_count();
		}
		return TotalCount;
	}

	size_t node::instance_count() const {
		size_t MeshCount = 0;
		for (const node& Chd : Child) {
			MeshCount += Chd.MeshInstance.size();
			MeshCount += Chd.instance_count();
		}
		return MeshCount;
	}

	math::mat<float, 4, 4> node::transform(const std::vector<float>& aAnimationWeight, const std::vector<animation>& aPlaybackAnimation, double aTime) {
		// This calculates the global transform of the node which it is
		// being called from. If there are no animations associated with 
		// the node hierarchy, the bind pose transformations will be used
		// instead. If there are animations associated with the node hierarchy,
		// Then the animation transformations will be used in a weighted average.

		//tex:
		// It is the responsibility of the model class to insure that the sum of the contribution
		// factors (weights) is equal to 1.
		// $$ 1 = w^{b} + \sum_{\forall A \in Anim} w_{i} $$
		// $$ T = T^{base} \cdot w^{base} + \sum_{\forall i \in A} T_{i}^{A} \cdot w_{i}^{A} $$ 
		//

		// Bind Pose Transform
		math::mat<float, 4, 4> NodeTransform = (this->Transformation * aAnimationWeight[0]);

		// TODO: Figure out how to load animations per node. Also incredibly slow right now. Optimize Later.
		// Overrides/Averages Animation Transformations with Bind Pose Transform based on weights.
		for (size_t i = 0; i < aPlaybackAnimation.size(); i++) {
			// Check if Animation Data exists for this node, if not, use bind pose.
			if (aPlaybackAnimation[i][this->Name].exists()) {
				// Animation Data Exists
				double TickerTime = std::fmod(aTime * aPlaybackAnimation[i].TicksPerSecond, aPlaybackAnimation[i].Duration);
				if (this->Root == this) {
					NodeTransform += this->Transformation * aPlaybackAnimation[i][this->Name][TickerTime] * aAnimationWeight[i + 1];
				}
				else {
					NodeTransform += aPlaybackAnimation[i][this->Name][TickerTime] * aAnimationWeight[i + 1];
				}
			}
			else {
				// Animation Data Does Not Exist, use bind pose animation.
				NodeTransform += this->Transformation * aAnimationWeight[i + 1];
			}
		}

		// Recursively apply parent transformations.
		if (this->Root != this) {
			return this->Parent->transform(aAnimationWeight, aPlaybackAnimation, aTime) * NodeTransform;
		}
		else {
			return NodeTransform;
		}
	}

	std::vector<mesh::instance*> node::gather_mesh_instances() {
		std::vector<mesh::instance*> MIL(this->MeshInstance.size());
		for (size_t i = 0; i < MIL.size(); i++) {
			MIL[i] = &this->MeshInstance[i];
		}
		for (node& Chd : this->Child) {
			std::vector<mesh::instance*> CMIL = Chd.gather_mesh_instances();
			MIL.insert(MIL.end(), CMIL.begin(), CMIL.end());
		}
		return MIL;
	}

	std::vector<node*> node::linearize() {
		std::vector<node*> Nodes;
		Nodes.push_back(this);
		for (node& Chd : this->Child) {
			std::vector<node*> CNodes = Chd.linearize();
			Nodes.insert(Nodes.end(), CNodes.begin(), CNodes.end());
		}
		return Nodes;
	}

}
