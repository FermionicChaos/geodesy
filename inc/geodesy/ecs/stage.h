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

namespace geodesy::ecs {
	
	class stage {
	public:

		struct workload {
			size_t Start;
			size_t Count;
		};

		std::string									Name;
		std::shared_ptr<core::gcl::context> 		Context;
		std::vector<std::shared_ptr<object>>		Object;

		stage(std::shared_ptr<core::gcl::context> aContext, std::string aName);
		~stage();

		virtual object::update_info update(double aDeltaTime);
		virtual subject::render_info render();

		static std::vector<subject*> purify_by_subject(const std::vector<std::shared_ptr<object>>& aObjectList);
		static std::vector<workload> determine_thread_workload(size_t aElementCount, size_t aThreadCount);

	};

}

#endif // GEODESY_CORE_STAGE_H