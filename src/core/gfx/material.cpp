#include <geodesy/core/gfx/material.h>

#include <vector>

#include <geodesy/core/gcl/shader.h>
#include <geodesy/core/gcl/image.h>

namespace geodesy::core::gfx {

    material::material() {
        this->Name          = "";
        this->Transparency  = OPAQUE;
    }

    material::material(std::shared_ptr<gcl::context> aContext, gcl::image::create_info aCreateInfo, std::shared_ptr<material> aMaterial) : material() {
        for (auto& Texture : aMaterial->Texture) {
            this->Texture[Texture.first] = std::make_shared<gcl::image>(aContext, aCreateInfo, Texture.second);
        }
    }

	material::~material() {}

}
