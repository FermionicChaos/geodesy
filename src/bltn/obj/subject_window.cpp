#include <geodesy/bltn/obj/subject_window.h>

#include <geodesy/bltn/obj/window.h>

namespace geodesy::bltn::obj {

	using namespace core;
	using namespace gcl;

	subject_window::subject_window(
		std::shared_ptr<core::gcl::context> 	aContext, 
		ecs::stage* 							aStage, 
		std::string 							aName, 
		std::shared_ptr<ecs::subject> 			aSubjectSource, 
		core::math::vec<float, 2> 				aSize,
		core::math::vec<float, 3> 				aPosition,
		core::math::vec<float, 2> 				aDirection
	) : ecs::object(
		aContext, 
		aStage, 
		aName,
		"assets/models/quad.obj",
		aPosition,
		aDirection
	) {
		this->SubjectSource = aSubjectSource;
	}

	std::vector<core::gfx::draw_call> subject_window::draw(ecs::subject* aSubjectTarget) {
		std::vector<gfx::draw_call> DrawCallList;

		// NOTE: A single draw call represents a single mesh instance in the model.
		if (this->OverridenRenderer.count(aSubjectTarget) == 0) {
			// ! No generalized renderer can draw this subject, that 
			// ! is why it has been overriden.
			this->OverridenRenderer[aSubjectTarget] = this->specialized_renderer(aSubjectTarget);
		}

		// This must get the associated draw calls not only based on the subjects renderer, but the read index of the source
		// subject this class contains. 
		DrawCallList = this->OverridenRenderer[aSubjectTarget][aSubjectTarget->Framechain->DrawIndex][this->SubjectSource->Framechain->ReadIndex];

		// Return a vector of the draw calls for the subject, and target draw index.
		return DrawCallList;
	}

	std::vector<std::vector<std::vector<core::gfx::draw_call>>> subject_window::specialized_renderer(ecs::subject* aSubjectTarget) {
		// Only used to access window uniform buffer.
		window* Window = dynamic_cast<window*>(aSubjectTarget);

		std::vector<gfx::mesh::instance*> MeshInstance = this->Model->Hierarchy.gather_mesh_instances();

		std::vector<std::vector<std::vector<gfx::draw_call>>> Renderer(aSubjectTarget->Framechain->Image.size(), std::vector<std::vector<gfx::draw_call>>(this->SubjectSource->Framechain->Image.size(), std::vector<gfx::draw_call>(MeshInstance.size())));

		// This will create a draw call per target frame, per source frame, per mesh instance.
		for (size_t i = 0; i < aSubjectTarget->Framechain->Image.size(); i++) {
			for (size_t j = 0; j < this->SubjectSource->Framechain->Image.size(); j++) {
				for (size_t k = 0; k < MeshInstance.size(); k++) {
					// Get references for readability.
					VkResult Result = VK_SUCCESS;
					std::shared_ptr<gfx::mesh> Mesh = this->Model->Mesh[MeshInstance[k]->Index];
					std::shared_ptr<gfx::material> Material = this->Model->Material[MeshInstance[k]->MaterialIndex];

					std::vector<std::shared_ptr<gcl::image>> ImageOutputList = {
						aSubjectTarget->Framechain->Image[i]["Color"],
					};
					Renderer[i][j][k].Framebuffer = this->Context->create_framebuffer(aSubjectTarget->Pipeline, ImageOutputList, aSubjectTarget->Framechain->Resolution);
					Renderer[i][j][k].DescriptorArray = this->Context->create_descriptor_array(aSubjectTarget->Pipeline);
					Renderer[i][j][k].DrawCommand = aSubjectTarget->CommandPool->allocate(); // TODO: deallocate command buffers from Command pool in base object destructor.

					// Bind Object Uniform Buffers
					Renderer[i][j][k].DescriptorArray->bind(0, 0, 0, Window->WindowUniformBuffer);		// Window Size, etc
					Renderer[i][j][k].DescriptorArray->bind(0, 1, 0, this->UniformBuffer);				// Object Position, Orientation, Scale
					Renderer[i][j][k].DescriptorArray->bind(0, 2, 0, Material->UniformBuffer); 			// Material Properties

					// Bind Material Textures.
					// ! This is where window renderer has to connect to accessing other subject's resources.
					Renderer[i][j][k].DescriptorArray->bind(1, 0, 0, this->SubjectSource->Framechain->Image[j]["OGB.Color"]);

					Result = Context->begin(Renderer[i][j][k].DrawCommand);
					std::vector<std::shared_ptr<buffer>> VertexBuffer = { Mesh->VertexBuffer, MeshInstance[k]->VertexWeightBuffer };
					aSubjectTarget->Pipeline->draw(Renderer[i][j][k].DrawCommand, Renderer[i][j][k].Framebuffer, VertexBuffer, Mesh->IndexBuffer, Renderer[i][j][k].DescriptorArray);
					Result = Context->end(Renderer[i][j][k].DrawCommand);
				}
			}
		}

		return Renderer;
	}

}
