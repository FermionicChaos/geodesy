#include <geodesy/engine.h>
#include <geodesy/runtime/object.h>

namespace geodesy::runtime {

	using namespace core;
	using namespace gpu;

	object::uniform_data::uniform_data(
		math::vec<float, 3> aPosition, 
		math::vec<float, 2> aDirection,
		math::vec<float, 3> aScale
	) {
		this->Position = aPosition;
		this->Orientation = math::orientation(aDirection[0], aDirection[1]);
		this->Scale = aScale;
	}

	object::creator::creator() {
		this->Name 					= "";
		this->ModelPath 			= "";
		this->Position 				= { 0.0f, 0.0f, 0.0f };
		this->Direction 			= { 0.0f, 0.0f };
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
		// Default direction in stage space is +y
		this->Type 				= core::phys::node::type::OBJECT;
		this->Theta 			= math::radians(aCreator->Direction[0] + 90.0f);
		this->Phi 				= math::radians(aCreator->Direction[1] + 90.0f);
		this->Name 				= aCreator->Name;
		this->Stage 			= aStage;
		this->Engine 			= aContext->Device->Engine;
		this->Position 			= aCreator->Position;
		this->Orientation 		= math::orientation(this->Theta, this->Phi);
		this->Scale 			= aCreator->Scale;
		this->DefaultTransform  = phys::calculate_transform(this->Position, this->Orientation, this->Scale);

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
			{ this->Theta, this->Phi }, // Direction is now a vec2 for Theta and Phi angles.
			this->Scale
		);
		this->UniformBuffer = aContext->create_buffer(UBCI, sizeof(uniform_data), &UniformData);
		this->UniformBuffer->map_memory(0, sizeof(uniform_data));
	}

	object::~object() {}

	void object::copy_data(const core::phys::node* aNode) {
		// This is ideally for root nodes, ignore base transform data.
		this->Identifier = aNode->Identifier;

		// Copy over physics mesh.
		this->CollisionMesh = aNode->CollisionMesh; // Copy the collision mesh if it exists.

		// Copy over mesh instance data.
		this->Context = ((gfx::node*)aNode)->Context; // Copy the context from the node.
		this->MeshInstance.resize(((gfx::node*)aNode)->MeshInstance.size());
		for (size_t i = 0; i < this->MeshInstance.size(); i++) {
			this->MeshInstance[i] = gfx::mesh::instance(((gfx::node*)aNode)->Context, ((gfx::node*)aNode)->MeshInstance[i], this->Root, this);
		}
	}

	bool object::is_subject() {
		return false;
	}

	void object::input(const core::hid::input& aInput) {

	}

	void object::host_update(
		double 										aDeltaTime, 
		double 										aTime, 
		const std::vector<float>& 					aAnimationWeight, 
		const std::vector<phys::animation>& 		aPlaybackAnimation,
		const std::vector<phys::force>& 			aAppliedForces
	) {

		// // Perform host update on the base class.
		// gfx::node::host_update(
		// 	aDeltaTime, 
		// 	aTime, 
		// 	aAnimationWeight, 
		// 	aPlaybackAnimation, 
		// 	aAppliedForces
		// );

		this->CurrentTransform = phys::calculate_transform(
			this->Position, 
			this->Orientation,
			this->Scale
		);

	}

	void object::device_update(
		double 										aDeltaTime, 
		double 										aTime, 
		const std::vector<float>& 					aAnimationWeight, 
		const std::vector<core::phys::animation>& 	aPlaybackAnimation,
		const std::vector<core::phys::force>& 		aAppliedForces
	) {

		// Update any mesh instances that exist at root.
		gfx::node::device_update(
			aDeltaTime, 
			aTime, 
			aAnimationWeight, 
			aPlaybackAnimation, 
			aAppliedForces
		);

		// Update renderers.
		for (auto& R : this->Renderer) {
			R.second->update(aDeltaTime, aTime);
		}

		// Update object uniform buffer (this is being phased out)
		*(uniform_data*)this->UniformBuffer->Ptr = uniform_data(
			this->Position, 
			{ this->Theta, this->Phi }, // Direction is now a vec2 for Theta and Phi angles.
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