#include <geodesy/engine.h>
#include <geodesy/runtime/object.h>

namespace geodesy::runtime {

	using namespace core;
	using namespace gpu;

	object::uniform_data::uniform_data(
		math::vec<float, 3> aPosition, 
		math::vec<float, 3> aDirRight, 
		math::vec<float, 3> aDirUp, 
		math::vec<float, 3> aDirForward,
		math::vec<float, 3> aScale
	) {
		this->Position = aPosition;
		this->Orientation = {
			aDirRight[0], 	aDirForward[0], 	aDirUp[0], 		0.0f,
			aDirRight[1], 	aDirForward[1], 	aDirUp[1], 		0.0f,
			aDirRight[2], 	aDirForward[2], 	aDirUp[2], 		0.0f,
			0.0f, 			0.0f, 				0.0f, 			1.0f
		};
		this->Scale = aScale;
	}

	object::creator::creator() {
		this->Name 					= "";
		this->ModelPath 			= "";
		this->Position 				= { 0.0f, 0.0f, 0.0f };
		this->Direction 			= { -90.0f, 0.0f };
		this->Scale 				= { 1.0f, 1.0f, 1.0f };
		this->AnimationWeights 		= std::vector<float>(1, 1.0f);
		this->MotionType 			= motion::STATIC;
		this->GravityEnabled 		= false;
		this->CollisionEnabled 		= false;
	}

	object::draw_call::draw_call() {
		TransparencyMode 		= gfx::material::transparency::OPAQUE;
		RenderingPriority 		= 0.0f;
		DrawCommand 			= VK_NULL_HANDLE;
	}

	void object::draw_call::update(
		subject* aSubject, 
		size_t aFrameIndex,
		object* aObject, 
		size_t aMeshInstanceIndex
	) {}

	object::renderer::renderer() {}

	object::renderer::renderer(object* aObject, subject* aSubject) {
		this->Object = aObject;
		this->Subject = aSubject;
	}

	object::renderer::~renderer() {
		// Clear out all command buffers.
		for (size_t i = 0; i < this->DrawCallList.size(); i++) {
			for (size_t j = 0; j < this->DrawCallList[i].size(); j++) {
				this->Subject->CommandPool->release(this->DrawCallList[i][j]->DrawCommand);
			}
		}
		// Clear out all GPU interface resources (framebuffers, descriptor arrays, etc).
		this->DrawCallList.clear();
	}

	std::vector<std::shared_ptr<object::draw_call>> object::renderer::operator[](size_t aIndex) const {
		return this->DrawCallList[aIndex];
	}

	void object::renderer::update(
		double aDeltaTime, 
		double aTime
	) {
		// Update the renderer with the current time and delta time.
		// This is where you would update the draw calls based on the object's state.
		for (size_t i = 0; i < this->DrawCallList.size(); i++) {
			for (size_t j = 0; j < this->DrawCallList[i].size(); j++) {
				// Update each draw call based on updated scene data.
				this->DrawCallList[i][j]->update(this->Subject, i, this->Object, j);
			}
		}
	}

	object::object(std::shared_ptr<core::gpu::context> aContext, stage* aStage, creator* aCreator) : core::gfx::node() {
		// this->Type 				= core::phys::node::type::OBJECT;
		this->Name 				= aCreator->Name;
		this->Stage 			= aStage;
		this->Engine 			= aContext->Device->Engine;
		this->Position 			= aCreator->Position;
		this->Theta 			= math::radians(aCreator->Direction[0] + 90.0f);
		this->Phi 				= math::radians(aCreator->Direction[1] + 90.0f);
		this->DirectionRight 	= {  std::sin(Phi), 					-std::cos(Phi), 					0.0f 			};
		this->DirectionUp 		= { -std::cos(Theta) * std::cos(Phi), 	-std::cos(Theta) * std::sin(Phi), 	std::sin(Theta) };
		this->DirectionFront 	= {  std::sin(Theta) * std::cos(Phi), 	 std::sin(Theta) * std::sin(Phi), 	std::cos(Theta) };
		this->Scale 			= aCreator->Scale;

		this->Context 			= aContext;

		// Create Object Model from GPU Device Context.
		if (aCreator->ModelPath != "") {
			// Load Host Model into memory.
			std::shared_ptr<io::file> ModelFile = Engine->FileManager.open(aCreator->ModelPath);

			// Check if Model File is valid.
			if (ModelFile.get() != nullptr) {
				// Save Model File Instance to Object to persist lifetime of resource.
				this->Asset.push_back(ModelFile);

				// Pointer cast into model type.
				std::shared_ptr<gfx::model> HostModel = std::dynamic_pointer_cast<gfx::model>(ModelFile);

				// Create Device Model from Host Model.
				image::create_info MaterialTextureInfo;
				MaterialTextureInfo.Layout 		= image::layout::SHADER_READ_ONLY_OPTIMAL;
				MaterialTextureInfo.Memory 		= device::memory::DEVICE_LOCAL;
				MaterialTextureInfo.Usage	 	= image::usage::SAMPLED | image::usage::COLOR_ATTACHMENT | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

				// TODO: Do a Device Context Registry to check which host models have been loaded
				// into device memory, and recycle them if they have been loaded already.
				this->Model = aContext->create_model(HostModel, MaterialTextureInfo);

				// Give the transform hierarchy to object, and make it root.
				this->swap(this->Model->Hierarchy.get());

				// If model has animations, then create animation data.
				if (this->Model->Animation.size() > 0) {
					// Include Bind Pose as the first Weight.
					this->AnimationWeights = std::vector<float>(this->Model->Animation.size() + 1, 0.0f);
					this->AnimationWeights[0] = 1.0f;
				}

				this->AnimationWeights = aCreator->AnimationWeights;
			}
		}

		// Linearize node tree for faster processing.
		this->LinearizedNodeTree = this->linearize();

		// Gather mesh instances.
		this->TotalMeshInstance = this->gather_instances();

		// Object Uniform Buffer Creation from GPU Device Context.
		buffer::create_info UBCI;
		UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
		UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;

		uniform_data UniformData = uniform_data(
			this->Position, 
			this->DirectionRight, 
			this->DirectionUp, 
			this->DirectionFront,
			this->Scale
		);
		this->UniformBuffer = aContext->create_buffer(UBCI, sizeof(uniform_data), &UniformData);
		this->UniformBuffer->map_memory(0, sizeof(uniform_data));
	}

	object::~object() {}

	void object::copy_data(const core::phys::node* aNode) {
		// This is ideally for root nodes, ignore base transform data.
		this->Identifier = aNode->Identifier;
		this->Transformation = aNode->Transformation;

		// Copy over physics mesh.
		this->CollisionMesh = aNode->CollisionMesh; // Copy the collision mesh if it exists.

		// Copy over mesh instance data.
		this->MeshInstance.resize(((gfx::node*)aNode)->MeshInstance.size());
		for (size_t i = 0; i < this->MeshInstance.size(); i++) {
			this->MeshInstance[i] = gfx::mesh::instance(this->Context, ((gfx::node*)aNode)->MeshInstance[i], this->Root, this);
		}
	}

	bool object::is_subject() {
		return false;
	}

	void object::input(const core::hid::input& aInput) {

	}

	void object::update(
		double 									aDeltaTime, 
		double 									aTime, 
		const std::vector<float>& 				aAnimationWeight, 
		const std::vector<phys::animation>& 	aPlaybackAnimation,
		const std::vector<phys::force>& 		aAppliedForces
	) {

		core::gfx::node::update(aDeltaTime, aTime, this->AnimationWeights, this->Model->Animation);

		this->DirectionRight			= {  std::sin(Phi), 					-std::cos(Phi), 					0.0f 			};
		this->DirectionUp				= { -std::cos(Theta) * std::cos(Phi), 	-std::cos(Theta) * std::sin(Phi), 	std::sin(Theta) };
		this->DirectionFront			= {  std::sin(Theta) * std::cos(Phi), 	 std::sin(Theta) * std::sin(Phi), 	std::cos(Theta) };

		// Update renderers.
		for (auto& R : this->Renderer) {
			R.second->update(aDeltaTime, aTime);
		}

		*(uniform_data*)this->UniformBuffer->Ptr = uniform_data(
			this->Position, 
			this->DirectionRight, 
			this->DirectionUp, 
			this->DirectionFront,
			this->Scale
		);
	}

	std::vector<std::shared_ptr<object::draw_call>> object::draw(subject* aSubject) {
		std::vector<std::shared_ptr<object::draw_call>> DrawCallList;
		
		// If the subject is the same as the object, return the draw call list empty.
		// A subject cannot draw itself.
		if (this == aSubject) return DrawCallList;

		// NOTE: A single draw call represents a single mesh instance in the model.
		if (this->Renderer.count(aSubject) == 0) {
			this->Renderer[aSubject] = aSubject->default_renderer(this);
		}

		// Get draw calls from this object for the subject.
		DrawCallList = (*this->Renderer[aSubject])[aSubject->Framechain->DrawIndex];

		// Return a vector of the draw calls for the subject, and target draw index.
		return DrawCallList;
	}

	std::vector<VkCommandBuffer> convert(std::vector<std::shared_ptr<object::draw_call>> aDrawCallList) {
		std::vector<VkCommandBuffer> CommandBufferList(aDrawCallList.size());
		for (size_t i = 0; i < aDrawCallList.size(); i++) {
			CommandBufferList[i] = aDrawCallList[i]->DrawCommand;
		}
		return CommandBufferList;
	}

}