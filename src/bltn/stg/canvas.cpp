#include <geodesy/bltn/stg/canvas.h>

#include <iostream>

namespace geodesy::bltn::stg {

	canvas::canvas(std::shared_ptr<core::gcl::context> aContext, std::string aName, std::shared_ptr<obj::window> aWindow) : ecs::stage(aContext, aName) {
		this->Window = aWindow;	
	}

	ecs::subject::render_info canvas::render() {
		// Rendering in canvas is non standard. The window in canvas is purely a target
		// and not an object.
		VkResult Result = VK_SUCCESS;
		ecs::subject::render_info RenderInfo;

		// Get next frame of primary window.
		Result = Window->next_frame();

		std::vector<VkSubmitInfo> SubmitInfo = Window->render(this);

		VkPresentInfoKHR PresentInfo = Window->present_frame();

		// Append to RenderInfo
		RenderInfo.SubmitInfo.insert(RenderInfo.SubmitInfo.end(), SubmitInfo.begin(), SubmitInfo.end());
		RenderInfo.PresentInfo.push_back(PresentInfo);
		
		return RenderInfo;
	}

}
