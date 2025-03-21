#include <geodesy/ecs/stage.h>
#include <geodesy/ecs/subject.h>

namespace geodesy::ecs {

	using namespace core;
	using namespace gpu;

	subject::framechain::framechain(std::shared_ptr<core::gpu::context> aContext, double aFrameRate, uint32_t aFrameCount) {
		this->DrawIndex = 0;
		this->ReadIndex = 0;
		this->FrameRate = aFrameRate;
		this->Timer = 1.0 / aFrameRate;
		this->Context = aContext;
		this->Image = std::vector<std::map<std::string, std::shared_ptr<core::gpu::image>>>(aFrameCount);
	}

	subject::framechain::~framechain() {

    for (size_t i = 0; i < this->PredrawFrameOperation.size(); i++) {
			this->Context->release_command_buffer(device::operation::GRAPHICS_AND_COMPUTE, this->PredrawFrameOperation[i].CommandBufferList);
		}
		for (size_t i = 0; i < this->PostdrawFrameOperation.size(); i++) {
			this->Context->release_command_buffer(device::operation::GRAPHICS_AND_COMPUTE, this->PostdrawFrameOperation[i].CommandBufferList);
		}
	}

	std::map<std::string, std::shared_ptr<image>> subject::framechain::read_frame() {
		return this->Image[this->ReadIndex];
	}

	std::map<std::string, std::shared_ptr<image>> subject::framechain::draw_frame() {
		return this->Image[this->DrawIndex];
	}

	bool subject::framechain::ready_to_render() {
		return this->Timer.check();
	}

	VkResult subject::framechain::next_frame_now() {
		return VK_SUCCESS;
	}

	VkResult subject::framechain::present_frame_now() {
		return VK_SUCCESS;
	}

	VkResult subject::framechain::next_frame(VkSemaphore aPresentFrameSemaphore, VkSemaphore aNextFrameSemaphore, VkFence aNextFrameFence) {
		// Make read index the previous frame that was drawn to.
		ReadIndex = DrawIndex;
		// Generate next draw index.
		DrawIndex = ((DrawIndex == (Image.size() - 1)) ? 0 : (DrawIndex + 1));
		// Return VK_SUCCESS if not system_window.
		return VK_SUCCESS;
	}

	std::vector<command_batch> subject::framechain::predraw() {
		return { PredrawFrameOperation[DrawIndex] };
	}

	std::vector<command_batch> subject::framechain::postdraw() {
		return { PostdrawFrameOperation[DrawIndex] };
	}

	subject::creator::creator() {
		this->Resolution = { 1920, 1080, 1 };
		this->FrameCount = 1;
		this->FrameRate = 60.0f;
		this->ImageUsage = image::usage::COLOR_ATTACHMENT | image::usage::SAMPLED | image::usage::TRANSFER_DST | image::usage::TRANSFER_SRC;
	}

	subject::subject(std::shared_ptr<core::gpu::context> aContext, stage* aStage, creator* aSubjectCreator) : object(aContext, aStage, aSubjectCreator) {

		this->CommandPool = std::make_shared<gpu::command_pool>(aContext, gpu::device::operation::GRAPHICS_AND_COMPUTE);
		this->SemaphorePool = std::make_shared<gpu::semaphore_pool>(aContext, 100); // ^Can be changed later
		// this->Timer;
		this->NextFrameSemaphore = VK_NULL_HANDLE;
		this->PresentFrameSemaphore = VK_NULL_HANDLE;
	}

	subject::~subject() {

	}

	bool subject::is_subject() {
		return true;
	}

	std::shared_ptr<ecs::object::renderer> subject::default_renderer(object* aObject) {
		std::shared_ptr<renderer> DefaultRenderer;
		return DefaultRenderer;
	}

	submission_batch subject::render(stage* aStage) {
		// The next frame operation will both present previously drawn frame and acquire next
		// frame. 
		VkResult Result = this->Framechain->next_frame();

		// Acquire predraw rendering operations.
		this->RenderingOperations += this->Framechain->predraw();

		// Iterate through all objects in the stage.
		gpu::command_batch StageCommandBatch;
		for (size_t i = 0; i < aStage->Object.size(); i++) {
			// Draw object.
			std::vector<std::shared_ptr<object::draw_call>> ObjectDrawCall = aStage->Object[i]->draw(this);
			std::vector<VkCommandBuffer> ObjectDrawCommand(ObjectDrawCall.size());
			for (size_t j = 0; j < ObjectDrawCall.size(); j++) {
				ObjectDrawCommand[j] = ObjectDrawCall[j]->DrawCommand;
			}
			// Group into single submission.
			StageCommandBatch += ObjectDrawCommand;
		}

		// Aggregate all rendering operations to subject.
		this->RenderingOperations += StageCommandBatch;

		// Acquire image transition and present frame if exists.
		this->RenderingOperations += this->Framechain->postdraw();

		// Setup safety dependencies for default rendering system.
		for (size_t i = 0; i < this->RenderingOperations.size() - 1; i++) {
			VkPipelineStageFlags Stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			this->RenderingOperations[i + 1].depends_on(this->SemaphorePool, Stage, this->RenderingOperations[i]);
		}

		// Build submission reference object and return.
		return build(this->RenderingOperations);
	}

}