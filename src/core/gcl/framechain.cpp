#include <geodesy/core/gcl/framechain.h>

namespace geodesy::core::gcl {

	// std::shared_ptr<uniform_array> pipeline::create_uniform_array() {
	// 	return std::make_shared<uniform_array>(this->Context, this);
	// }

	// frame::frame() {}

	framechain::framechain(std::shared_ptr<context> aContext, double aFrameRate, uint32_t aFrameCount) {
		this->DrawIndex = 0;
		this->ReadIndex = 0;
		this->FrameRate = aFrameRate;
		this->Timer = 1.0 / aFrameRate;
		this->Context = aContext;
		this->Image = std::vector<std::map<std::string, std::shared_ptr<image>>>(aFrameCount);
	}

}
