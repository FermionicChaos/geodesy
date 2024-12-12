#include <geodesy/ecs/stage.h>
#include <geodesy/ecs/subject.h>

namespace geodesy::ecs {

	using namespace core;
	using namespace gcl;

	subject::subject(
		std::shared_ptr<core::gcl::context> aContext, 
		stage* aStage, 
		std::string aName, 
		core::math::vec<uint, 3> 
		aFrameResolution, 
		double aFrameRate, 
		uint32_t aFrameCount, 
		uint32_t aAttachmentCount, 
		core::math::vec<float, 3> aPosition, 
		core::math::vec<float, 2> aDirection
	) : object(aContext, aStage, aName, aPosition, aDirection) {

		this->CommandPool = std::make_shared<gcl::command_pool>(aContext, gcl::device::operation::GRAPHICS_AND_COMPUTE);
		this->SemaphorePool = std::make_shared<gcl::semaphore_pool>(aContext, 100); // ^Can be changed later
		// this->Timer;
	}

	subject::~subject() {

	}

	bool subject::is_subject() {
		return true;
	}

	bool subject::ready_to_render() {
		return this->Framechain->Timer.check();
	}

	VkResult subject::next_frame_now() {
		VkResult Result = VK_SUCCESS;
		
		VkFence Fence = this->Context->create_fence();
		// Acquire next image from swapchain.
		// Result = this->next_frame(VK_NULL_HANDLE, Fence);

		// Context->wait_and_reset(Fence);

		// Context->destroy_fence(Fence);

		return Result;
	}

	VkResult subject::present_frame_now() {
		VkResult Result = VK_SUCCESS;

		// VkPresentInfoKHR PresentInfo = this->present_frame();

		// Result = Context->present({ PresentInfo });

		return Result;
	}

	std::map<std::string, std::shared_ptr<core::gcl::image>> subject::read_frame() {
		return this->Framechain->Image[this->Framechain->ReadIndex];
	}

	std::map<std::string, std::shared_ptr<core::gcl::image>> subject::draw_frame() {
		return this->Framechain->Image[this->Framechain->DrawIndex];
	}

	std::vector<std::vector<core::gfx::draw_call>> subject::default_renderer(object* aObject) {
		std::vector<std::vector<core::gfx::draw_call>> DefaultRenderer;
		return DefaultRenderer;
	}

	command_batch subject::next_frame(std::shared_ptr<semaphore_pool> aSemaphorePool) {
		// !This method only exists to abstract over system window swapchains.
		command_batch CommandBatch;
		this->Framechain->ReadIndex = this->Framechain->DrawIndex;
		this->Framechain->DrawIndex = ((this->Framechain->DrawIndex == (Framechain->Image.size() - 1)) ? 0 : (this->Framechain->DrawIndex + 1));
		return CommandBatch;
	}

	submission_batch subject::render(stage* aStage) {
		// Acquire next image from swapchain.
		this->RenderingOperations += this->next_frame(this->SemaphorePool);

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
		this->RenderingOperations += this->present_frame(this->SemaphorePool);

		// Setup safety dependencies for default rendering system.
		for (size_t i = 0; i < this->RenderingOperations.size() - 1; i++) {
			VkPipelineStageFlags Stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			this->RenderingOperations[i + 1].depends_on(this->SemaphorePool, Stage, this->RenderingOperations[i]);
		}

		// Build submission reference object and return.
		return build(this->RenderingOperations);
	}

	std::vector<command_batch> subject::present_frame(std::shared_ptr<semaphore_pool> aSemaphorePool) {
		// !This method only exists to abstract over system window swapchains.
		std::vector<command_batch> CommandBatch(2);
		return CommandBatch;
	}

}