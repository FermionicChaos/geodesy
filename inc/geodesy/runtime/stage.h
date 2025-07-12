#ifndef GEODESY_CORE_STAGE_H
#define GEODESY_CORE_STAGE_H

#include <memory>

#include "../config.h"
#include "object.h"
#include "subject.h"

/*
Originally it seemed like a good idea to allow object sharing between stages, and use stage pointers to indicate
ownership, but upon reflecting on this idea, it was not a good one. I have been for the better half of a year
been trying to figure out an extendable object management system allowing for multi render target rendering. 

Object sharing seemed like good idea at first, because the rendered results of a scene3d could then be shared
with a canvas stage so gui elements could be added in in post. The reason for allowing the sharing of objects 
in general was a bad idea, because certain attributes such as orientation and position only make sense in a 
single space (stage).

Therefore to then allow for complex rendering systems, a new solution has been reached. Every render target (subject)
and its results are just a series of images that can be read at any other point an any stage. It is effectively a 
streaming window.

For example, camera3d has all the properties of a camera, a model and such, but to share it in a compositor (canvas),
its content has to be referenced in another window. subject_window, which makes any subject a 2d window either in 
a 2d space or 3d space.

subject_window will allow the contents of a subject to be directly shared in another stage, in the form of a window.
This can be used for mirrors, camera systems in game, and so on. Or it can be put into a canvas stage to be used as
video source for post processing. It doesn't really matter.

virtual_window is a type of render target that can be used to share inputs and outputs between canvas instances.
*/

// Geodesy uses a right-handed coordinate system.
// Right-handed coordinate system
//        +Z
//         |
//         |
//         |
//         O-------> +Y
//        /
//       /
//      +X

namespace geodesy::runtime {
	
	class stage {
	public:

		struct workload {
			size_t Start;
			size_t Count;
		};

		std::string											Name;
		double												Time;
		std::vector<core::phys::node*>						NodeCache; // This is a list of all nodes in the stage, used for updating.
		std::shared_ptr<core::gpu::context> 				Context;
		std::vector<std::shared_ptr<object>>				Object;
		std::map<std::string, std::shared_ptr<object>> 		ObjectLookup;

		std::shared_ptr<core::gpu::acceleration_structure> 	TLAS;
		// std::map<subject*, 

		stage(std::shared_ptr<core::gpu::context> aContext, std::string aName);
		~stage();

		// This is a helper utility function that helps create arbitrary objects.
		template<typename object_type, typename... args>
		std::shared_ptr<object_type> create_object(args&&... aArgs) {
			std::shared_ptr<object_type> NewObject(new object_type(
				this->Context,
				this,
				std::forward<args>(aArgs)...
			));
			this->Object.push_back(NewObject);
			this->ObjectLookup[NewObject->Name] = NewObject;
			return NewObject;
		}

		virtual core::gpu::submission_batch update(double aDeltaTime);
		virtual core::gpu::submission_batch render();
		std::vector<std::shared_ptr<object::draw_call>> trace(subject* aSubject);

		static std::vector<subject*> purify_by_subject(const std::vector<std::shared_ptr<object>>& aObjectList);
		static std::vector<workload> determine_thread_workload(size_t aElementCount, size_t aThreadCount);

	};

}

#endif // GEODESY_CORE_STAGE_H