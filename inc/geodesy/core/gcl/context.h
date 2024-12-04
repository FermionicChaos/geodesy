#pragma once
#ifndef GEODESY_CORE_GCL_CONTEXT_H
#define GEODESY_CORE_GCL_CONTEXT_H

#include "../../config.h"
#include "../math.h"
#include "device.h"
#include "command_pool.h"
#include "buffer.h"
#include "image.h"
#include "shader.h"
#include "descriptor.h"
#include "pipeline.h"
#include "swapchain.h"

namespace geodesy::core::gcl {

	class context : public std::enable_shared_from_this<context>{
	public:

		std::mutex									Mutex;
		std::shared_ptr<device> 					Device;
		VkDevice									Handle;
		std::map<uint, VkQueue>						Queue;
		std::map<uint, bool> 						InFlight;
		std::map<uint, VkFence> 					ExecutionFence;
		std::map<uint, VkCommandPool> 				CommandPool;
		std::map<uint, std::set<VkCommandBuffer>> 	CommandBuffer;
		std::set<VkSemaphore>						Semaphore;
		std::set<VkFence>							Fence;
		std::set<VkDeviceMemory>					Memory;

		context(std::shared_ptr<device> aDevice, std::vector<uint> aOperationBitfieldList, std::vector<const char*> aLayerList = {}, std::vector<const char*> aExtensionList = {});
		~context();

		VkMemoryRequirements get_buffer_memory_requirements(VkBuffer aBufferHandle) const;
		VkMemoryRequirements get_image_memory_requirements(VkImage aImageHandle) const;

		// ----- Device Resource Management ----- //

		VkCommandBuffer allocate_command_buffer(device::operation aOperation, VkCommandBufferLevel aLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		std::vector<VkCommandBuffer> allocate_command_buffer(device::operation aOperation, uint32_t aCount, VkCommandBufferLevel aLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		void release_command_buffer(device::operation aOperation, VkCommandBuffer& aCommandBuffer);
		void release_command_buffer(device::operation aOperation, std::vector<VkCommandBuffer>& aCommandBuffer);

		VkSemaphore create_semaphore(VkSemaphoreCreateFlags aSemaphoreCreateFlags = 0);
		std::vector<VkSemaphore> create_semaphore(int aCount, VkSemaphoreCreateFlags aSemaphoreCreateFlags = 0);
		void destroy_semaphore(VkSemaphore& aSemaphore);
		void destroy_semaphore(std::vector<VkSemaphore>& aSemaphoreList);

		VkFence create_fence(VkFenceCreateFlags aFenceCreateFlags = 0);
		std::vector<VkFence> create_fence(int aCount, VkFenceCreateFlags aFenceCreateFlags = 0);
		void destroy_fence(VkFence& aFence);
		void destroy_fence(std::vector<VkFence>& aFenceList);

		VkDeviceMemory allocate_memory(VkMemoryRequirements aMemoryRequirements, uint aMemoryType);
		void free_memory(VkDeviceMemory& aMemoryHandle);

		std::shared_ptr<buffer> create_buffer(buffer::create_info aCreateInfo, int aVertexCount, util::variable aVertexLayout, void* aVertexData = NULL);
		std::shared_ptr<buffer> create_buffer(uint aMemoryType, uint aBufferUsage, int aVertexCount, util::variable aVertexLayout, void* aVertexData = NULL);
		std::shared_ptr<buffer> create_buffer(buffer::create_info aCreateInfo, size_t aBufferSize, void* aBufferData = NULL);
		std::shared_ptr<buffer> create_buffer(uint aMemoryType, uint aBufferUsage, size_t aBufferSize, void* aBufferData = NULL);
		std::shared_ptr<buffer> create_buffer(uint aMemoryType, uint aBufferUsage, size_t aElementCount, size_t aBufferSize, void* aBufferData = NULL);

		std::shared_ptr<image> create_image(image::create_info aCreateInfo, std::string aFilePath);
		std::shared_ptr<image> create_image(image::create_info aCreateInfo, std::shared_ptr<image> aHostImage);
		std::shared_ptr<image> create_image(image::create_info aCreateInfo, image::format aFormat, uint aX, uint aY = 1, uint aZ = 1, uint aT = 1, void* aTextureData = NULL);

		std::shared_ptr<descriptor::array> create_descriptor_array(std::shared_ptr<pipeline> aPipeline, VkSamplerCreateInfo aSamplerCreateInfo = descriptor::DefaultSamplerCreateInfo);

		std::shared_ptr<framebuffer> create_framebuffer(std::shared_ptr<pipeline> aPipeline, std::vector<std::shared_ptr<image>> aImageAttachements, math::vec<uint, 3> aResolution);
		std::shared_ptr<framebuffer> create_framebuffer(std::shared_ptr<pipeline> aPipeline, std::map<std::string, std::shared_ptr<image>> aImage, std::vector<std::string> aAttachmentSelection, math::vec<uint, 3> aResolution);

		std::shared_ptr<pipeline> create_pipeline(std::shared_ptr<pipeline::rasterizer> aRasterizer, VkRenderPass aRenderPass = VK_NULL_HANDLE, uint32_t aSubpassIndex = 0);

		// std::shared_ptr<gfx::model> create_model(gcl::image::create_info aCreateInfo, std::string aFilePath);
		std::shared_ptr<gfx::model> create_model(std::shared_ptr<gfx::model> aModel, gcl::image::create_info aCreateInfo = {});

		// ----- Command Buffer Recording ----- //

		VkResult begin(VkCommandBuffer aCommandBuffer);
		VkResult end(VkCommandBuffer aCommandBuffer);

		void begin_rendering(VkCommandBuffer aCommandBuffer, VkRect2D aRenderArea, std::vector<VkImageView> aColorAttachments, VkImageView aDepthAttachment = VK_NULL_HANDLE, VkImageView aStencilAttachment = VK_NULL_HANDLE);
		void end_rendering(VkCommandBuffer aCommandBuffer);

		// TODO: Maybe use forward declaration? 
		void bind_vertex_buffers(VkCommandBuffer aCommandBuffer, std::vector<VkBuffer> aBufferList, const VkDeviceSize* aOffset = NULL);
		void bind_index_buffer(VkCommandBuffer aCommandBuffer, VkBuffer aBufferHandle, VkIndexType aIndexType);
		void bind_descriptor_sets(VkCommandBuffer aCommandBuffer, VkPipelineBindPoint aPipelineBindPoint, VkPipelineLayout aPipelineLayout, std::vector<VkDescriptorSet> aDescriptorSetList, std::vector<uint32_t> aDynamicOffsetList = std::vector<uint32_t>(0));
		void bind_pipeline(VkCommandBuffer aCommandBuffer, VkPipelineBindPoint aPipelineBindPoint, VkPipeline aPipelineHandle);
		void draw_indexed(VkCommandBuffer aCommandBuffer, uint32_t aIndexCount, uint32_t aInstanceCount = 1, uint32_t aFirstIndex = 0, uint32_t aVertexOffset = 0, uint32_t aFirstInstance = 0);

		// ----- Command Buffer Execution ----- //

		VkResult wait(VkFence aFence);
		VkResult wait(std::vector<VkFence> aFenceList, VkBool32 aWaitOnAll = VK_TRUE);

		VkResult reset(VkFence aFence);
		VkResult reset(std::vector<VkFence> aFenceList);

		VkResult wait_and_reset(VkFence aFence);
		VkResult wait_and_reset(std::vector<VkFence> aFenceList, VkBool32 aWaitOnAll = VK_TRUE);
		
		VkResult execute(device::operation aDeviceOperation, VkCommandBuffer aCommandBuffer, VkFence aFence = VK_NULL_HANDLE);
		VkResult execute(device::operation aDeviceOperation, const std::vector<VkSubmitInfo>& aSubmissionList, VkFence aFence = VK_NULL_HANDLE);
		VkResult present(const std::vector<VkPresentInfoKHR>& aPresentationList);
		VkResult execute(device::operation aDeviceOperation, const std::vector<VkSubmitInfo>& aSubmissionList, const std::vector<VkPresentInfoKHR>& aPresentationList = {}, VkFence aFence = VK_NULL_HANDLE);

		VkResult execute_and_wait(device::operation aDeviceOperation, VkCommandBuffer aCommandBuffer);
		VkResult execute_and_wait(device::operation aDeviceOperation, const std::vector<VkSubmitInfo>& aSubmissionList);
		VkResult present_and_wait(const std::vector<VkPresentInfoKHR>& aPresentationList);
		VkResult execute_and_wait(device::operation aDeviceOperation, const std::vector<VkSubmitInfo>& aSubmissionList, const std::vector<VkPresentInfoKHR>& aPresentationList);
		
		VkResult engine_wait(std::vector<device::operation> aDeviceOperation);
		VkResult engine_execute(device::operation aDeviceOperation, const std::vector<VkSubmitInfo>& aSubmissionList);
		//VkResult engine_execute(device::operation aDeviceOperation, const std::vector<VkPresentInfoKHR>& aPresentationList);

	};

}

#endif // GEODESY_CORE_GCL_CONTEXT_H