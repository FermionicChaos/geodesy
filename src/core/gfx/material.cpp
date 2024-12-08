#include <geodesy/core/gfx/material.h>

#include <vector>

#include <geodesy/core/gcl/context.h>
#include <geodesy/core/gcl/shader.h>
#include <geodesy/core/gcl/image.h>

namespace geodesy::core::gfx {

    using namespace gcl;

    namespace {

        struct material_data{
	        alignas(4) float ParallaxScale;
	        alignas(4) int ParallaxIterations;
	        alignas(4) float VertexColorWeight;
	        alignas(4) float RefractionIndex;
            material_data();
        };

        material_data::material_data() {
            this->ParallaxScale = 0.0f;
            this->ParallaxIterations = 0;
            this->VertexColorWeight = 0.0f;
            this->RefractionIndex = 1.0f;
        }

    }

    material::material() {
        this->Name              = "";
        this->RenderingSystem   = UNKNOWN;
        this->Transparency      = OPAQUE;
    }

    material::material(std::shared_ptr<gcl::context> aContext, gcl::image::create_info aCreateInfo, std::shared_ptr<material> aMaterial) : material() {
        this->Name              = aMaterial->Name;
        this->RenderingSystem   = aMaterial->RenderingSystem;
        this->Transparency      = aMaterial->Transparency;

        // Create GPU Uniform Buffer for material properties.
        buffer::create_info UBCI;
        UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
        UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;
        material_data MaterialData;
        this->UniformBuffer = aContext->create_buffer(UBCI, sizeof(material_data), &MaterialData);
        this->UniformBuffer->map_memory(0, sizeof(material_data));

        // Copy over and create GPU instance textures.
        for (auto& Texture : aMaterial->Texture) {
            this->Texture[Texture.first] = std::make_shared<gcl::image>(aContext, aCreateInfo, Texture.second);
        }
    }

	material::~material() {}

}
