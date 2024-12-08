#include <geodesy/engine.h>
#include <geodesy/ecs/object.h>

namespace geodesy::ecs {

	using namespace core;
	using namespace gcl;

	object::object(std::shared_ptr<core::gcl::context> aContext, stage* aStage, std::string aName, math::vec<float, 3> aPosition, math::vec<float, 2> aDirection) {
		float Theta 					= math::radians(aDirection[0]);
		float Phi 						= math::radians(aDirection[1]);
		this->Engine 					= aContext->Device->Engine;
		this->Stage 					= aStage;
		this->Name 						= aName;
		this->Mass						= 1.0f;
		this->Time						= 0.0f;
		this->Position					= aPosition;
		this->DirectionRight			= {  std::sin(Phi), 					-std::cos(Phi), 					0.0f 			};
		this->DirectionUp				= { -std::cos(Theta) * std::cos(Phi), 	-std::cos(Theta) * std::sin(Phi), 	std::sin(Theta) };
		this->DirectionFront			= {  std::sin(Theta) * std::cos(Phi), 	 std::sin(Theta) * std::sin(Phi), 	std::cos(Theta) };

		// // Is not moving.
		// this->LinearMomentum			= { 0.0f, 0.0f, 0.0f };
		// // Is not rotating.
		// this->AngularMomentum			= { 0.0f, 0.0f, 0.0f };

		this->Motion 					= motion::STATIC;
		this->Gravity 					= false;
		this->Collision 				= false;
		
		this->Context 					= aContext;

		buffer::create_info UBCI;
		UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
		UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;

		uniform_data UniformData;
		UniformData.Position = this->Position;
		UniformData.Orientation = {
			DirectionRight[0], 		DirectionFront[0], 		DirectionUp[0], 	0.0f,
			DirectionRight[1], 		DirectionFront[1], 		DirectionUp[1], 	0.0f,
			DirectionRight[2], 		DirectionFront[2], 		DirectionUp[2], 	0.0f,
			0.0f, 					0.0f, 					0.0f, 				1.0f
		};

		this->UniformBuffer = aContext->create_buffer(UBCI, sizeof(uniform_data), &UniformData);
		this->UniformBuffer->map_memory(0, sizeof(uniform_data));
	}

	object::~object() {}

	bool object::is_subject() {
		return false;
	}

	void object::update(double aDeltaTime, math::vec<float, 3> aAppliedForce, math::vec<float, 3> aAppliedTorque) {
		//update_info UpdateInfo;
		// Newtons First Law: An object in motion tends to stay in motion.
		// Newtons Second Law: The change in momentum of an object is equal to the forces applied to it.
		// Newtons Third Law: For every action, there is an equal and opposite reaction.
		math::mat<float, 3, 3> InvertedInertiaTensor;// = math::inverse(PhysicsMesh->InertiaTensor);

		// How the momentum of the object will change when a force is applied to it.
		this->LinearMomentum += aAppliedForce * aDeltaTime;
		this->AngularMomentum += aAppliedTorque * aDeltaTime;

		// How the object will move according to its current momentum.
		this->Position += (this->LinearMomentum / this->Mass) * aDeltaTime;

		// TODO: Add update using angular momentum to change orientation of object over time.

		this->Time += aDeltaTime;

		this->Model->update(aDeltaTime);

		//return UpdateInfo;
	}

	std::vector<gfx::draw_call> object::draw(subject* aSubject) {
		// NOTE: A single draw call represents a single mesh instance in the model.
		if (this->Renderer.count(aSubject) == 0) {
			this->Renderer[aSubject] = aSubject->default_renderer(this);
		} 

		// Return a vector of the draw calls for the subject, and target draw index.
		return Renderer[aSubject][aSubject->Framechain->DrawIndex];
	}

}