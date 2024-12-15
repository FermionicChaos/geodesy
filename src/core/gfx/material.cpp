#include <geodesy/core/gfx/material.h>

#include <vector>

#include <geodesy/core/gcl/context.h>
#include <geodesy/core/gcl/shader.h>
#include <geodesy/core/gcl/image.h>

namespace geodesy::core::gfx {

    using namespace gcl;

    namespace {

        struct material_data {
			alignas(16) math::vec<float, 3> 	Color;					// Base Color of the Material
			alignas(16) math::vec<float, 3> 	Emissive;				// Emissive Color of the Material
			alignas(16) math::vec<float, 3> 	Ambient;				// Ambient Color of the Material
			alignas(16) math::vec<float, 3> 	Specular;				// Specular Color of the Material
			alignas(4) float 					Opacity;
			alignas(4) float 					RefractionIndex;
			alignas(4) float 					Shininess;
			alignas(4) float 					Metallic;
			alignas(4) float 					Roughness;
			alignas(4) float 					VertexColorWeight;
            alignas(4) float 					MaterialColorWeight;
			alignas(4) float 					ParallaxScale;
			alignas(4) int 						ParallaxIterationCount;
			material_data();
			material_data(const material* aMaterial);
        };

        material_data::material_data() {
			this->Color                     = { 1.0f, 0.0f, 1.0f };
			this->Emissive                  = { 0.0f, 0.0f, 0.0f };
			this->Ambient                   = { 0.0f, 0.0f, 0.0f };
			this->Specular                  = { 0.0f, 0.0f, 0.0f };
			this->Opacity                   = 1.0f;
			this->RefractionIndex           = 1.0f;
			this->Shininess                 = 0.0f;
			this->Metallic                  = 0.0f;
			this->Roughness                 = 0.0f;
			this->VertexColorWeight         = 0.0f;
            this->MaterialColorWeight       = 0.0f;
			this->ParallaxScale             = 0.0f;
			this->ParallaxIterationCount    = 0;
        }

        material_data::material_data(const material* aMaterial) {
            this->Color                     = aMaterial->Color;
            this->Emissive                  = aMaterial->Emissive;
            this->Ambient                   = aMaterial->Ambient;
            this->Specular                  = aMaterial->Specular;
            this->Opacity                   = aMaterial->Opacity;
            this->RefractionIndex           = aMaterial->RefractionIndex;
            this->VertexColorWeight         = aMaterial->VertexColorWeight;
            this->MaterialColorWeight       = aMaterial->MaterialColorWeight;
            this->ParallaxScale             = aMaterial->ParallaxScale;
            this->ParallaxIterationCount    = aMaterial->ParallaxIterationCount;
        }

    }

    material::material() {
        this->Name                      = "";
        this->RenderingSystem           = UNKNOWN;
        this->Transparency              = OPAQUE;
        
		this->Color                     = { 1.0f, 0.0f, 1.0f };
		this->Emissive                  = { 0.0f, 0.0f, 0.0f };
		this->Ambient                   = { 0.0f, 0.0f, 0.0f };
		this->Specular                  = { 0.0f, 0.0f, 0.0f };
		this->Opacity                   = 1.0f;
		this->RefractionIndex           = 1.0f;
		this->Shininess                 = 0.0f;
		this->Metallic                  = 0.0f;
		this->Roughness                 = 0.0f;
		this->VertexColorWeight         = 0.0f;
        this->MaterialColorWeight       = 0.0f;
		this->ParallaxScale             = 0.0f;
		this->ParallaxIterationCount    = 0;
    }

    material::material(std::shared_ptr<gcl::context> aContext, gcl::image::create_info aCreateInfo, std::shared_ptr<material> aMaterial) : material() {
        this->Name              = aMaterial->Name;
        this->RenderingSystem   = aMaterial->RenderingSystem;
        this->Transparency      = aMaterial->Transparency;

        // Create GPU Uniform Buffer for material properties.
        buffer::create_info UBCI;
        UBCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
        UBCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_SRC | buffer::usage::TRANSFER_DST;

        material_data MaterialData = material_data(aMaterial.get());
        this->UniformBuffer = aContext->create_buffer(UBCI, sizeof(material_data), &MaterialData);
        this->UniformBuffer->map_memory(0, sizeof(material_data));

        // Copy over and create GPU instance textures.
        for (auto& Texture : aMaterial->Texture) {
            this->Texture[Texture.first] = std::make_shared<gcl::image>(aContext, aCreateInfo, Texture.second);
        }
    }

	material::~material() {}

    void material::update(double aDeltaTime) {
        // Update Material Properties
        material_data MaterialData = material_data(this);
        memcpy(this->UniformBuffer->Ptr, &MaterialData, sizeof(material_data));
    }

}
