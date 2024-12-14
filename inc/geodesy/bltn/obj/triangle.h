#ifndef GEODESY_BLTN_OBJ_TRIANGLE_H
#define GEODESY_BLTN_OBJ_TRIANGLE_H

#include <geodesy/engine.h>

namespace geodesy::bltn::obj {

	class triangle : public ecs::object {
	public:

		triangle(
			std::shared_ptr<core::gcl::context> 	aContext, 
			ecs::stage* 							aStage, 
			std::string 							aName
		);

	};	

}

#endif // !GEODESY_BLTN_OBJ_TRIANGLE_H