#include <geodesy/bltn/obj/subject_window.h>

#include <geodesy/bltn/obj/window.h>

namespace geodesy::bltn::obj {

	using namespace core;
	using namespace gpu;

	subject_window::forward_draw_call::forward_draw_call(
		object* 							aObject, 
		core::gfx::mesh::instance* 			aMeshInstance,
		runtime::subject* 					aSubjectSource,
		size_t 								aSourceFrameIndex,
		window* 							aSubjectTarget,
		size_t 								aTargetFrameIndex
	) {
		// Get references for readability.
		VkResult Result = VK_SUCCESS;
		std::shared_ptr<core::gpu::context> Context = aObject->Context;
		std::shared_ptr<gfx::mesh> Mesh = aObject->Model->Mesh[aMeshInstance->MeshIndex];
		std::shared_ptr<gfx::material> Material = aObject->Model->Material[aMeshInstance->MaterialIndex];

		std::vector<std::shared_ptr<gpu::image>> ImageOutputList = {
			aSubjectTarget->Framechain->Image[aTargetFrameIndex]["Color"],
		};
		// Acquire Mesh Vertex Buffer, and Mesh Instance Vertex Weight Buffer.
		std::vector<std::shared_ptr<buffer>> VertexBuffer = { Mesh->VertexBuffer, aMeshInstance->VertexWeightBuffer };

		Framebuffer 		= Context->create_framebuffer(aSubjectTarget->Pipeline, ImageOutputList, aSubjectTarget->Framechain->Resolution);
		DescriptorArray 	= Context->create_descriptor_array(aSubjectTarget->Pipeline);
		DrawCommand 		= aSubjectTarget->CommandPool->allocate();

		// Bind Object Uniform Buffers
		DescriptorArray->bind(0, 0, 0, aSubjectTarget->WindowUniformBuffer);		// Window Size, etc
		DescriptorArray->bind(0, 1, 0, aObject->UniformBuffer);						// Object Position, Orientation, Scale
		DescriptorArray->bind(0, 2, 0, Material->UniformBuffer); 					// Material Properties

		// Bind Material Textures.
		// ! This is where the contents of another render target are forwarded.
		DescriptorArray->bind(1, 0, 0, aSubjectSource->Framechain->Image[aSourceFrameIndex]["OGB.Color"]);

		Result = Context->begin(DrawCommand);
		aSubjectTarget->Pipeline->draw(DrawCommand, Framebuffer, VertexBuffer, Mesh->IndexBuffer, DescriptorArray);
		Result = Context->end(DrawCommand);
	}

	// Do not get confused, subject_window is not a render target, it forwards the outputs of other render targets.
	subject_window::forward_renderer::forward_renderer(
		runtime::object* 	aObject, 			// SubjectWindow
		runtime::subject* 	aSubjectSource, 	// SubjectWindow source
		runtime::subject* 	aSubjectTarget 		// Actual Render Target
	) : runtime::object::renderer(aObject, aSubjectTarget) {
		// Gather Mesh instances of the object.
		std::vector<gfx::mesh::instance*> MeshInstance = aObject->Model->Hierarchy->gather_instances();

		this->OverridenDrawCallList = std::vector<std::vector<std::vector<std::shared_ptr<draw_call>>>>(
			aSubjectTarget->Framechain->Image.size(),
			std::vector<std::vector<std::shared_ptr<draw_call>>>(
				aSubjectSource->Framechain->Image.size(),
				std::vector<std::shared_ptr<draw_call>>(MeshInstance.size())
			)
		);

		for (size_t i = 0; i < aSubjectTarget->Framechain->Image.size(); i++) {
			for (size_t j = 0; j < aSubjectSource->Framechain->Image.size(); j++) {
				for (size_t k = 0; k < MeshInstance.size(); k++) {
					// Convert Subject Target to base class window.
					window* Window = dynamic_cast<window*>(aSubjectTarget);
					// Load draw calls per target frame, per source frame, per mesh instance.
					OverridenDrawCallList[i][j][k] = geodesy::make<forward_draw_call>(aObject, MeshInstance[k], aSubjectSource, j, Window, i);
				}
			}
		}
	}

	subject_window::forward_renderer::~forward_renderer() {
		// Clear out all command buffers.
		for (size_t i = 0; i < this->OverridenDrawCallList.size(); i++) {
			for (size_t j = 0; j < this->OverridenDrawCallList[i].size(); j++) {
				for (size_t k = 0; k < this->OverridenDrawCallList[i][j].size(); k++) {
					this->Subject->CommandPool->release(this->OverridenDrawCallList[i][j][k]->DrawCommand);
				}
			}
		}
		// Clear out all GPU interface resources (framebuffers, descriptor arrays, etc).
		this->OverridenDrawCallList.clear();
	}

	subject_window::creator::creator() {
		this->Subject = nullptr;
	}

	subject_window::subject_window(std::shared_ptr<core::gpu::context> aContext, runtime::stage* aStage, creator* aSubjectWindowCreator) : runtime::object(aContext, aStage, aSubjectWindowCreator) {
		this->SubjectSource = aSubjectWindowCreator->Subject;
	}

	std::vector<std::shared_ptr<runtime::object::draw_call>> subject_window::draw(runtime::subject* aSubjectTarget) {
		std::vector<std::shared_ptr<runtime::object::draw_call>> DrawCallList;

		// NOTE: A single draw call represents a single mesh instance in the model.
		if (this->Renderer.count(aSubjectTarget) == 0) {
			// ! No generalized renderer can draw this subject, that 
			// ! is why it has been overriden.
			this->Renderer[aSubjectTarget] = geodesy::make<forward_renderer>(this, this->SubjectSource.get(), aSubjectTarget);
		}

		// This must get the associated draw calls not only based on the subjects renderer, but the read index of the source
		// subject this class contains. 
		std::shared_ptr<forward_renderer> ForwardRenderer = std::dynamic_pointer_cast<forward_renderer>(this->Renderer[aSubjectTarget]);
		DrawCallList = ForwardRenderer->OverridenDrawCallList[aSubjectTarget->Framechain->DrawIndex][this->SubjectSource->Framechain->ReadIndex];

		// Return a vector of the draw calls for the subject, and target draw index.
		return DrawCallList;
	}

}
