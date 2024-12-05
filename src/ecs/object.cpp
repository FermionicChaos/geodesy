#include <geodesy/engine.h>
#include <geodesy/ecs/object.h>

namespace geodesy::ecs {

	using namespace core;

	// Localized to object.cpp
	namespace {

		// This constitutes the default renderer of the geodesy
		// engine. Since this is localized to object.cpp, for any
		// derived class of object, it can override and define its
		// own renderer.
		class default_renderer : public gfx::renderer {
		public:

		};

	}

	object::object(std::shared_ptr<gcl::context> aContext, stage* aStage, std::string aName) {
		this->Engine 					= aContext->Device->Engine;
		this->Stage 					= aStage;
		this->Name 						= aName;
		this->Mass						= 1.0f;
		this->Time						= 0.0f;
		this->Position					= math::vec<float, 3>(0.0f, 0.0f, 0.0f);//{ 0.0f, 0.0f, 0.0f };
		// this->DirectionX				= { 1.0f, 0.0f, 0.0f };
		// this->DirectionY				= { 0.0f, 1.0f, 0.0f };
		// this->DirectionZ				= { 0.0f, 0.0f, 1.0f };

		// // Is not moving.
		// this->LinearMomentum			= { 0.0f, 0.0f, 0.0f };
		// // Is not rotating.
		// this->AngularMomentum			= { 0.0f, 0.0f, 0.0f };

		this->Motion 					= motion::STATIC;
		this->Gravity 					= false;
		this->Collision 				= false;
		
		this->Context 					= aContext;

		// std::shared_ptr<core::io::file> OpenedFile = this->Engine->FileManager.open("../glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf");
	}

	object::~object() {
		for (auto& FileInstance : Asset) {
			
		}
	}

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

		//return UpdateInfo;
	}

	std::vector<gfx::draw_call> object::draw(subject* aSubject) {
		// NOTE: A single draw call represents a single mesh instance in the model.
		if (this->Renderer.count(aSubject) == 0) {
			// this->Renderer[aSubject] = aSubject->make_default_renderer(this);
		} 

		// Return a vector of the draw calls for the subject, and target draw index.
		return Renderer[aSubject][aSubject->Framechain->DrawIndex];
	}

}