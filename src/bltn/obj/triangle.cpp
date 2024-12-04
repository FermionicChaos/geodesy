#include <geodesy/bltn/obj/triangle.h>

namespace geodesy::bltn::obj {

	using namespace core;
	using namespace gcl;
	using namespace gfx;

	triangle::triangle(std::shared_ptr<core::gcl::context> aContext, ecs::stage* aStage, std::string aName) : ecs::object(aContext, aStage, aName) {
		engine* Engine = aContext->Device->Engine;

		std::vector<std::string> AssetList = {
			"assets/models/quad.obj",
			"assets/images/wall.jpg"
		};

		this->Asset = Engine->FileManager.open(AssetList);

		// Get Host model copy.
		std::shared_ptr<core::gfx::model> HostModel = std::dynamic_pointer_cast<core::gfx::model>(Asset[0]);

		// TODO: Change Hostmodel's Texture to wall.jpg
		HostModel->Material[0]->Texture["Color"] = std::dynamic_pointer_cast<core::gcl::image>(Asset[1]);

		gcl::image::create_info MaterialTextureInfo;
		MaterialTextureInfo.Layout 		= image::layout::SHADER_READ_ONLY_OPTIMAL;
		MaterialTextureInfo.Memory 		= device::memory::DEVICE_LOCAL;
		MaterialTextureInfo.Usage	 	= image::usage::SAMPLED | image::usage::COLOR_ATTACHMENT | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		//this->Model = Context->create_model();
		this->Model = std::make_shared<core::gfx::model>(aContext, HostModel, MaterialTextureInfo);
	}

}
