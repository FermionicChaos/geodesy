#include <geodesy/bltn/obj/triangle.h>

namespace geodesy::bltn::obj {

	using namespace core;
	using namespace gcl;
	using namespace gfx;

	triangle::triangle(std::shared_ptr<core::gcl::context> aContext, ecs::stage* aStage, std::string aName) : ecs::object(aContext, aStage, aName) {
		engine* Engine = aContext->Device->Engine;

		std::vector<std::string> AssetList = {
			"../glTF-Sample-Models/2.0/Box/glTF/Box.gltf",
			// "../glTF-Sample-Models/2.0/BoomBox/glTF/BoomBox.gltf",
			// "../glTF-Sample-Models/2.0/WaterBottle/glTF/WaterBottle.gltf",
		};

		this->Asset = Engine->FileManager.open(AssetList);

		// Get Host model copy.
		std::shared_ptr<core::gfx::model> HostModel = std::dynamic_pointer_cast<core::gfx::model>(Asset[0]);

		// Set color texture

		gcl::image::create_info MaterialTextureInfo;
		MaterialTextureInfo.Layout 		= image::layout::SHADER_READ_ONLY_OPTIMAL;
		MaterialTextureInfo.Memory 		= device::memory::DEVICE_LOCAL;
		MaterialTextureInfo.Usage	 	= image::usage::SAMPLED | image::usage::COLOR_ATTACHMENT | image::usage::TRANSFER_SRC | image::usage::TRANSFER_DST;

		//this->Model = Context->create_model();
		this->Model = std::make_shared<core::gfx::model>(aContext, HostModel, MaterialTextureInfo);
		// std::shared_ptr<core::io::file> OpenedFile = this->Engine->FileManager.open("../glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf");
	}

}
