#include <geodesy/bltn/obj/subject_window.h>

namespace geodesy::bltn::obj {

	subject_window::subject_window(std::shared_ptr<core::gcl::context> aContext, ecs::stage* aStage, std::string aName, std::shared_ptr<ecs::subject> aSubject) : ecs::object(aContext, aStage, aName) {
		
	}

}
