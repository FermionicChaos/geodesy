#include <geodesy/ecs/stage.h>
#include <geodesy/ecs/subject.h>

namespace geodesy::ecs {

	using namespace core;
	using namespace gcl;

	subject::creator::creator() {
		this->Resolution = { 1920, 1080, 1 };
		this->FrameCount = 1;
		this->FrameRate = 60.0f;
		this->ImageUsage = image::usage::COLOR_ATTACHMENT | image::usage::SAMPLED | image::usage::TRANSFER_DST | image::usage::TRANSFER_SRC;
	}

	subject::subject(std::shared_ptr<core::gcl::context> aContext, stage* aStage, creator* aSubjectCreator) : object(aContext, aStage, aSubjectCreator) {

		this->CommandPool = std::make_shared<gcl::command_pool>(aContext, gcl::device::operation::GRAPHICS_AND_COMPUTE);
		this->SemaphorePool = std::make_shared<gcl::semaphore_pool>(aContext, 100); // ^Can be changed later
		// this->Timer;
	}

	subject::~subject() {

	}

	bool subject::is_subject() {
		return true;
	}

	std::vector<std::vector<core::gfx::draw_call>> subject::default_renderer(object* aObject) {
		std::vector<std::vector<core::gfx::draw_call>> DefaultRenderer;
		return DefaultRenderer;
	}

	submission_batch subject::render(stage* aStage) {
		// Acquire next image from swapchain.
		this->RenderingOperations += this->Framechain->next_frame();

		// Iterate through all objects in the stage.
		gcl::command_batch StageCommandBatch;
		for (size_t i = 0; i < aStage->Object.size(); i++) {
			// Draw object.
			std::vector<gfx::draw_call> ObjectDrawCall = aStage->Object[i]->draw(this);
			std::vector<VkCommandBuffer> ObjectDrawCommand(ObjectDrawCall.size());
			for (size_t j = 0; j < ObjectDrawCall.size(); j++) {
				ObjectDrawCommand[j] = ObjectDrawCall[j].DrawCommand;
			}
			// Group into single submission.
			StageCommandBatch += ObjectDrawCommand;
		}

		// Aggregate all rendering operations to subject.
		this->RenderingOperations += StageCommandBatch;

		// Acquire image transition and present frame if exists.
		this->RenderingOperations += this->Framechain->present_frame();

		// Setup safety dependencies for default rendering system.
		for (size_t i = 0; i < this->RenderingOperations.size() - 1; i++) {
			VkPipelineStageFlags Stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			this->RenderingOperations[i + 1].depends_on(this->SemaphorePool, Stage, this->RenderingOperations[i]);
		}

		// Build submission reference object and return.
		return build(this->RenderingOperations);
	}

}