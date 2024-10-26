#include <geodesy/bltn/stg/canvas.h>

#include <iostream>

namespace geodesy::bltn::stg {

	canvas::canvas(std::shared_ptr<core::gcl::context> aContext, std::string aName) : ecs::stage(aContext, aName) {

	}

	ecs::subject::render_info canvas::render() {
		// Rendering in canvas is non standard. The window in canvas is purely a target
		// and not an object.
		VkResult Result = VK_SUCCESS;
		ecs::subject::render_info RenderInfo;
		for (std::shared_ptr<obj::window> Win : this->Window) {

			// Check if window is ready to be drawn to.
			if (!Win->ready_to_render()) continue;

			// TODO: Figure out how to use semaphores to synchronize the rendering of the window.
			Result = Win->next_frame();

			std::vector<VkSubmitInfo> SubmitInfo = Win->render(this);

			VkPresentInfoKHR PresentInfo = Win->present_frame();

			// Append to RenderInfo
			RenderInfo.SubmitInfo.insert(RenderInfo.SubmitInfo.end(), SubmitInfo.begin(), SubmitInfo.end());
			RenderInfo.PresentInfo.push_back(PresentInfo);
		}
		return RenderInfo;
	}

}
