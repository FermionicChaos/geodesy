#include <geodesy/engine.h>
#include <geodesy/ecs/object.h>

namespace geodesy::ecs {

	using namespace core;
	using namespace gcl;

	object::uniform_data::uniform_data(
		core::math::vec<float, 3> aPosition, 
		core::math::vec<float, 3> aDirRight, 
		core::math::vec<float, 3> aDirUp, 
		core::math::vec<float, 3> aDirForward
	) {
		this->Position = aPosition;
		this->Orientation = {
			aDirRight[0], 	aDirForward[0], 	aDirUp[0], 		0.0f,
			aDirRight[1], 	aDirForward[1], 	aDirUp[1], 		0.0f,
			aDirRight[2], 	aDirForward[2], 	aDirUp[2], 		0.0f,
			0.0f, 			0.0f, 				0.0f, 			1.0f
		};
	}

	object::object(
		std::shared_ptr<core::gcl::context> 	aContext, 
		stage* 									aStage, 
		std::string 							aName, 
		std::string 							aModelPath,
		math::vec<float, 3> 					aPosition, 
		math::vec<float, 2> 					aDirection
	) {
		this->Name 						= aName;
		this->Stage 					= aStage;
		this->Engine 					= aContext->Device->Engine;
		this->Time						= 0.0f;
		this->DeltaTime					= 0.0f;
		this->Mass						= 1.0f;
		this->Scale						= { 1.0f, 1.0f, 1.0f };
		this->Position					= aPosition;
		this->Theta 					= math::radians(aDirection[0] + 90.0f);
		this->Phi 						= math::radians(aDirection[1] + 90.0f);
		this->DirectionRight			= {  std::sin(Phi), 					-std::cos(Phi), 					0.0f 			};
		this->DirectionUp				= { -std::cos(Theta) * std::cos(Phi), 	-std::cos(Theta) * std::sin(Phi), 	std::sin(Theta) };
		this->DirectionFront			= {  std::sin(Theta) * std::cos(Phi), 	 std::sin(Theta) * std::sin(Phi), 	std::cos(Theta) };
		this->LinearMomentum			= { 0.0f, 0.0f, 0.0f };
		this->AngularMomentum			= { 0.0f, 0.0f, 0.0f };

		this->Motion 					= motion::STATIC;
		this->Gravity 					= false;
		this->Collision 				= false;

		// TODO: Load main object assets.
		if (aModelPath != "") {
			// Cast into vector.
			std::vector<std::string> AssetPath = { aModelPath };
			// Load Model into Host Memory.
			std::vector<std::shared_ptr<core::io::file>> NewAsset = this->Engine->FileManager.open(AssetPath);
			// Append to already loaded assets (none).
			this->Asset.insert(this->Asset.end(), NewAsset.begin(), NewAsset.end());
		}

		// Initialize GPU stuff.
		this->Context 					= aContext;
		// Create Object Model from GPU Device Context.
		if (aModelPath != "") {
			// Get Host Model
			std::shared_ptr<gfx::model> HostModel = std::dynamic_pointer_cast<gfx::model>(this->Asset[0]);

			// Create Material Texture Info
			image::create_info MaterialTextureInfo;
			MaterialTextureInfo.Layout 		= image::layout::SHADER_READ_ONLY_OPTIMAL;
			MaterialTextureInfo.Memory 		= device::memory::DEVICE_LOCAL;
			MaterialTextureInfo.Usage	 	= image::usage::SAMPLED | image::usage::COLOR_ATTACHMENT | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

			// Convert Host model into device model, and bind to object.
			this->Model = std::make_shared<gfx::model>(aContext, HostModel, MaterialTextureInfo);
		}

		// Object Uniform Buffer Creation from GPU Device Context.
		buffer::create_info UBCI;
		UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
		UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;

		uniform_data UniformData = uniform_data(
			this->Position, 
			this->DirectionRight, 
			this->DirectionUp, 
			this->DirectionFront
		);
		this->UniformBuffer = aContext->create_buffer(UBCI, sizeof(uniform_data), &UniformData);
		this->UniformBuffer->map_memory(0, sizeof(uniform_data));
	}

	object::~object() {}

	bool object::is_subject() {
		return false;
	}

	void object::input(const core::hid::input& aInput) {

	}

	void object::update(double aDeltaTime, math::vec<float, 3> aAppliedForce, math::vec<float, 3> aAppliedTorque) {
		this->Time += aDeltaTime;
		this->DeltaTime = aDeltaTime;
		//update_info UpdateInfo;
		// Newtons First Law: An object in motion tends to stay in motion.
		// Newtons Second Law: The change in momentum of an object is equal to the forces applied to it.
		// Newtons Third Law: For every action, there is an equal and opposite reaction.
		math::mat<float, 3, 3> InvertedInertiaTensor;// = math::inverse(PhysicsMesh->InertiaTensor);

		// How the momentum of the object will change when a force is applied to it.
		this->AngularMomentum += aAppliedTorque * aDeltaTime;

		// How the object will move according to its current momentum.
		this->LinearMomentum += (aAppliedForce + this->InputForce) * aDeltaTime;
		this->Position += (this->LinearMomentum / this->Mass + this->InputVelocity) * aDeltaTime;

		// TODO: Add update using angular momentum to change orientation of object over time.


		if (this->Model.get() != nullptr) {
			// TODO: Update Model animation later.
			// this->Model->update(aDeltaTime);
		}

		this->DirectionRight			= {  std::sin(Phi), 					-std::cos(Phi), 					0.0f 			};
		this->DirectionUp				= { -std::cos(Theta) * std::cos(Phi), 	-std::cos(Theta) * std::sin(Phi), 	std::sin(Theta) };
		this->DirectionFront			= {  std::sin(Theta) * std::cos(Phi), 	 std::sin(Theta) * std::sin(Phi), 	std::cos(Theta) };

		uniform_data UniformData = uniform_data(
			this->Position, 
			this->DirectionRight, 
			this->DirectionUp, 
			this->DirectionFront
		);
		memcpy(this->UniformBuffer->Ptr, &UniformData, sizeof(uniform_data));
	}

	std::vector<gfx::draw_call> object::draw(subject* aSubject) {
		std::vector<gfx::draw_call> DrawCallList;
		
		// If the subject is the same as the object, return the draw call list empty.
		// A subject cannot draw itself.
		if (this == aSubject) return DrawCallList;

		// NOTE: A single draw call represents a single mesh instance in the model.
		if (this->Renderer.count(aSubject) == 0) {
			this->Renderer[aSubject] = aSubject->default_renderer(this);
		}

		// Get draw calls from this object for the subject.
		DrawCallList = this->Renderer[aSubject][aSubject->Framechain->DrawIndex];

		// Return a vector of the draw calls for the subject, and target draw index.
		return DrawCallList;
	}

}