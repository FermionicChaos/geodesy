#include <geodesy/bltn/obj/triangle.h>

namespace geodesy::bltn::obj {

	using namespace core;
	using namespace gcl;
	using namespace gfx;

	triangle::triangle(std::shared_ptr<core::gcl::context> aContext, ecs::stage* aStage, std::string aName) : ecs::object(aContext, aStage, aName) {
		gcl::image::create_info MaterialTextureInfo;
		MaterialTextureInfo.Layout = image::layout::SHADER_READ_ONLY_OPTIMAL;
		MaterialTextureInfo.Sample = image::sample::COUNT_1;
		MaterialTextureInfo.Tiling = image::tiling::OPTIMAL;
		MaterialTextureInfo.Memory = device::memory::DEVICE_LOCAL;
		MaterialTextureInfo.Usage	 = image::usage::SAMPLED | image::usage::COLOR_ATTACHMENT | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		engine* Engine = aContext->Device->Engine;
		std::string ModelPath = "assets/models/triangle.obj";
		// Open through engine file manager.
		std::shared_ptr<core::gfx::model> HostModel = std::dynamic_pointer_cast<core::gfx::model>(Engine->FileManager.open(ModelPath));
		this->Model = std::make_shared<core::gfx::model>(aContext, MaterialTextureInfo, HostModel);
	}

}
