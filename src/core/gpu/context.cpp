#include <geodesy/engine.h>
#include <geodesy/core/gpu/context.h>

namespace geodesy::core::gpu {

	using namespace util;

	const std::set<std::string> context::RayTracingExtensions = {
		"VK_KHR_acceleration_structure",
		"VK_KHR_ray_tracing_pipeline",
		"VK_KHR_buffer_device_address",
		"VK_KHR_deferred_host_operations",
		"VK_EXT_descriptor_indexing",
		"VK_KHR_pipeline_library"
	};

	// Function names for Vulkan ray tracing extensions
	static const std::vector<const char*> VulkanRayTracingFunctionNames = {
		// VK_KHR_acceleration_structure functions
		"vkCreateAccelerationStructureKHR",
		"vkDestroyAccelerationStructureKHR",
		"vkGetAccelerationStructureBuildSizesKHR",
		"vkGetAccelerationStructureDeviceAddressKHR",
		"vkCmdBuildAccelerationStructuresKHR",
		"vkCmdBuildAccelerationStructuresIndirectKHR",
		"vkBuildAccelerationStructuresKHR",
		"vkCopyAccelerationStructureKHR",
		"vkCopyAccelerationStructureToMemoryKHR",
		"vkCopyMemoryToAccelerationStructureKHR",
		"vkWriteAccelerationStructuresPropertiesKHR",
		"vkCmdCopyAccelerationStructureKHR",
		"vkCmdCopyAccelerationStructureToMemoryKHR",
		"vkCmdCopyMemoryToAccelerationStructureKHR",
		"vkGetDeviceAccelerationStructureCompatibilityKHR",
		"vkCmdWriteAccelerationStructuresPropertiesKHR",

		// VK_KHR_ray_tracing_pipeline functions
		"vkCreateRayTracingPipelinesKHR",
		"vkGetRayTracingShaderGroupHandlesKHR",
		"vkGetRayTracingCaptureReplayShaderGroupHandlesKHR",
		"vkCmdTraceRaysKHR",
		"vkCmdTraceRaysIndirectKHR",
		"vkGetRayTracingShaderGroupStackSizeKHR",
		"vkCmdSetRayTracingPipelineStackSizeKHR",

		// VK_KHR_buffer_device_address functions
		"vkGetBufferDeviceAddressKHR",
		"vkGetBufferOpaqueCaptureAddressKHR",
		"vkGetDeviceMemoryOpaqueCaptureAddressKHR",

		// VK_KHR_deferred_host_operations functions
		"vkCreateDeferredOperationKHR",
		"vkDestroyDeferredOperationKHR",
		"vkGetDeferredOperationMaxConcurrencyKHR",
		"vkGetDeferredOperationResultKHR",
		"vkDeferredOperationJoinKHR",

		// VK_EXT_descriptor_indexing
		// (This extension primarily modifies structures and doesn't add
		// many new functions, but here's the one it does add)
		"vkGetDescriptorEXT",

		// VK_KHR_pipeline_library
		// (This extension mainly adds to VkPipelineCreateFlags and
		// doesn't introduce new functions, it's primarily used with
		// ray tracing pipelines to modularize shader groups)
	};

	context::context(std::shared_ptr<device> aDevice, std::vector<uint> aOperationBitfieldList, std::set<std::string> aLayerList, std::set<std::string> aExtensionList) {
		VkResult Result = VK_SUCCESS;
		std::vector<math::vec<uint32_t, 2>> QI(aOperationBitfieldList.size());

		this->Device = aDevice;

		// Is a list of the allocated Queue Indices. This can be used to determine total queue allocation.
		// 1. Use for total Queue allocation in device creation.
		// 2. Use for accessing queue post device creation.
		{
			std::vector<device::qfp> QFP = aDevice->QueueFamilyProperty;
			std::vector<int> OT(QFP.size(), 0);
			for (size_t i = 0; i < aOperationBitfieldList.size(); i++) {
				// Select Queue Family Index.
				int QFI = aDevice->qfi(aOperationBitfieldList[i]);
				QI[i][0] = QFI;
				QI[i][1] = OT[QFI];
				OT[QFI] = (OT[QFI] == (QFP[QFI].QueueCount - 1)) ? 0 : OT[QFI] + 1;
			}
		}

		// Create Device Context.
		{
			VkResult 									Result = VK_SUCCESS;
			std::vector<int>							UQFI;
			std::vector<std::vector<float>> 	 		QP;
			std::vector<VkDeviceQueueCreateInfo> 		QCI;
			std::vector<const char*>					Layer;
			std::vector<const char*>					Extension;
			VkPhysicalDeviceDynamicRenderingFeatures 	DRF{};
			VkDeviceCreateInfo 					 		CI{};

			// Convert std::set into std::vector.
			for (const std::string& L : aLayerList) {
				Layer.push_back(L.c_str());
			}

			// Convert std::set into std::vector.
			for (const std::string& E : aExtensionList) {
				Extension.push_back(E.c_str());
			}

			// Get Unique Queue Family Indices (UQFI).
			{
				std::set<int> UQFIS;
				// Get set of unique queue family indices
				for (math::vec<uint32_t, 2> IP : QI) {
					UQFIS.insert(IP[0]);
				}
				UQFI = std::vector<int>(UQFIS.begin(), UQFIS.end());
			}

			QP.resize(UQFI.size());
			QCI.resize(UQFI.size());

			// Get Queue Family Count for each UQFI
			for (size_t i = 0; i < UQFI.size(); i++) {
				std::set<int> UQI;
				for (math::vec<uint32_t, 2> IP : QI) {
					if (UQFI[i] == IP[0]) {
						UQI.insert(IP[1]);
					}
				}
				QP[i].resize(UQI.size());
				for (size_t j = 0; j < QP[i].size(); j++) {
					QP[i][j] = 1.0f;
				}
			}

			for (size_t i = 0; i < QCI.size(); i++) {
				QCI[i].sType					= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				QCI[i].pNext					= NULL;
				QCI[i].flags					= 0;
				QCI[i].queueFamilyIndex			= UQFI[i];
				QCI[i].queueCount				= QP[i].size();
				QCI[i].pQueuePriorities			= QP[i].data();
			}

			void* Next = NULL;
			// Buffer Device Address Features (required for acceleration structures)
			VkPhysicalDeviceBufferDeviceAddressFeatures PDBDAF{};
			if (aExtensionList.count("VK_KHR_buffer_device_address") > 0) {
				PDBDAF.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
				PDBDAF.pNext = Next;
				PDBDAF.bufferDeviceAddress = VK_TRUE;
				Next = &PDBDAF;
			}
			
			// Descriptor Indexing Features (required for ray tracing shader binding tables)
			VkPhysicalDeviceDescriptorIndexingFeatures PDDIF{};
			if (aExtensionList.count("VK_EXT_descriptor_indexing") > 0) {
				PDDIF.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
				PDDIF.pNext = Next;
				PDDIF.runtimeDescriptorArray = VK_TRUE;
				PDDIF.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
				Next = &PDDIF;
			}
			
			// Acceleration Structure Features
			VkPhysicalDeviceAccelerationStructureFeaturesKHR PDASF{};
			if (aExtensionList.count("VK_KHR_acceleration_structure") > 0) {
				PDASF.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
				PDASF.pNext = Next;
				PDASF.accelerationStructure = VK_TRUE;
				Next = &PDASF;
			}
			
			// Ray Tracing Pipeline Features
			VkPhysicalDeviceRayTracingPipelineFeaturesKHR PDRTPF{};
			if (aExtensionList.count("VK_KHR_ray_tracing_pipeline") > 0) {
				PDRTPF.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
				PDRTPF.pNext = Next;
				PDRTPF.rayTracingPipeline = VK_TRUE;
				Next = &PDRTPF;
			}
			
			// Deferred Host Operations (improves performance for ray tracing)
			// VkPhysicalDeviceDeferredHostOperationsFeaturesKHR PDHOF{};
			// VkPhysicalDeviceDeferredHostOperationsFeaturesKHR deferredHostOpsFeatures{};
			// deferredHostOpsFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEFERRED_HOST_OPERATIONS_FEATURES_KHR;
			// deferredHostOpsFeatures.pNext = Next;
			// deferredHostOpsFeatures.deferredHostOperations = VK_TRUE;
			// Next = &deferredHostOpsFeatures;
			
			// Finally, link to the physical device features
			VkPhysicalDeviceFeatures2 DF2{};
			DF2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			DF2.pNext = Next;
			DF2.features = aDevice->Features;
			
			CI.sType						= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			CI.pNext 						= &DF2;
			CI.flags						= 0;
			CI.queueCreateInfoCount			= QCI.size();
			CI.pQueueCreateInfos			= QCI.data();
			CI.enabledLayerCount			= Layer.size();
			CI.ppEnabledLayerNames			= Layer.data();
			CI.enabledExtensionCount		= Extension.size();
			CI.ppEnabledExtensionNames		= Extension.data();
			CI.pEnabledFeatures				= NULL;

			Result = vkCreateDevice(aDevice->Handle, &CI, NULL, &this->Handle);
			if (Result != VK_SUCCESS) {
				aDevice->Engine->Logger << log::message(log::VULKAN, Result, "Error: Failed to create device context!");
				throw aDevice->Engine->Logger;
			}

			// Save a copy of loaded layers and extensions.
			this->Layers = aLayerList;
			this->Extensions = aExtensionList;

			// Load function pointers to ray tracing functions.
			for (const char* FunctionName : VulkanRayTracingFunctionNames) {
				this->FunctionPointer[FunctionName] = vkGetDeviceProcAddr(this->Handle, FunctionName);
			}
		}

		// Load Queues to map.
		for (size_t i = 0; i < aOperationBitfieldList.size(); i++) {
			VkCommandPoolCreateInfo CreateInfo {};
			Queue[aOperationBitfieldList[i]] 			= VK_NULL_HANDLE;
			InFlight[aOperationBitfieldList[i]] 		= false;
			ExecutionFence[aOperationBitfieldList[i]] 	= this->create_fence();
			CommandPool[aOperationBitfieldList[i]] 		= VK_NULL_HANDLE;
			CommandBuffer[aOperationBitfieldList[i]] 	= std::set<VkCommandBuffer>();
			vkGetDeviceQueue(Handle, QI[i][0], QI[i][1], &Queue[aOperationBitfieldList[i]]);
			CreateInfo.sType 							= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			CreateInfo.queueFamilyIndex 				= Device->qfi(aOperationBitfieldList[i]);
			Result = vkCreateCommandPool(Handle, &CreateInfo, NULL, &CommandPool[aOperationBitfieldList[i]]);
		}

	}

	context::~context() {
		for (VkDeviceMemory DM : Memory) {
			vkFreeMemory(Handle, DM, NULL);
		}
		for (VkFence F : Fence) {
			vkDestroyFence(Handle, F, NULL);
		}
		for (VkSemaphore S : Semaphore) {
			vkDestroySemaphore(Handle, S, NULL);
		}
		// Destroy command buffers.
		for (auto& [Operation, CommandBufferSet] : CommandBuffer) {
			for (VkCommandBuffer CB : CommandBufferSet) {
				vkFreeCommandBuffers(Handle, CommandPool[Operation], 1, &CB);
			}
			vkDestroyCommandPool(Handle, CommandPool[Operation], NULL);
		}
		vkDestroyDevice(Handle, NULL);
	}

	VkMemoryRequirements context::get_buffer_memory_requirements(VkBuffer aBufferHandle) const {
		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements(this->Handle, aBufferHandle, &MemoryRequirements);
		return MemoryRequirements;
	}

	VkMemoryRequirements context::get_image_memory_requirements(VkImage aImageHandle) const {
		VkMemoryRequirements MemoryRequirements;
		vkGetImageMemoryRequirements(this->Handle, aImageHandle, &MemoryRequirements);
		return MemoryRequirements;
	}

	bool context::extension_enabled(const std::string& aExtensionName) {
		if (this->Extensions.count(aExtensionName) > 0) {
			return true;
		}
		else {
			return false;
		}
	}

	VkCommandBuffer context::allocate_command_buffer(device::operation aOperation, VkCommandBufferLevel aLevel) {
		return this->allocate_command_buffer(aOperation, 1, aLevel)[0];
	}

	std::vector<VkCommandBuffer> context::allocate_command_buffer(device::operation aOperation, uint32_t aCount, VkCommandBufferLevel aLevel) {
		VkResult Result = VK_SUCCESS;
		VkCommandBufferAllocateInfo AllocateInfo {};
		std::vector<VkCommandBuffer> NewCommandBuffer(aCount);
		AllocateInfo.sType 					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		AllocateInfo.commandPool 			= this->CommandPool[aOperation];
		AllocateInfo.level 					= aLevel;
		AllocateInfo.commandBufferCount 	= aCount;
		Result = vkAllocateCommandBuffers(this->Handle, &AllocateInfo, NewCommandBuffer.data());
		this->CommandBuffer[aOperation].insert(NewCommandBuffer.begin(), NewCommandBuffer.end());
		return NewCommandBuffer;
	}

	void context::release_command_buffer(device::operation aOperation, VkCommandBuffer& aCommandBuffer) {
		std::vector<VkCommandBuffer> CommandBuffer = { aCommandBuffer };
		this->release_command_buffer(aOperation, CommandBuffer);
		aCommandBuffer = VK_NULL_HANDLE;
	}

	void context::release_command_buffer(device::operation aOperation, std::vector<VkCommandBuffer>& aCommandBuffer) {
		for (VkCommandBuffer CommandBuffer : aCommandBuffer) {
			this->CommandBuffer[aOperation].erase(CommandBuffer);
		}
		if (aCommandBuffer.size() > 0) {
			vkFreeCommandBuffers(this->Handle, this->CommandPool[aOperation], aCommandBuffer.size(), aCommandBuffer.data());
		}
		aCommandBuffer.clear();
	}

	VkSemaphore context::create_semaphore(VkSemaphoreCreateFlags aSemaphoreCreateFlags) {
		std::vector<VkSemaphore> SemaphoreList = this->create_semaphore(1, aSemaphoreCreateFlags);
		return SemaphoreList[0];
	}

	std::vector<VkSemaphore> context::create_semaphore(int aCount, VkSemaphoreCreateFlags aSemaphoreCreateFlags) {
		VkResult Result = VK_SUCCESS;
		std::vector<VkSemaphore> SemaphoreList(aCount);
		VkSemaphoreCreateInfo SemaphoreCreateInfo{};
		SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		SemaphoreCreateInfo.pNext = NULL;
		SemaphoreCreateInfo.flags = aSemaphoreCreateFlags;
		for (size_t i = 0; i < SemaphoreList.size(); i++) {
			Result = vkCreateSemaphore(this->Handle, &SemaphoreCreateInfo, NULL, &SemaphoreList[i]);
		}
		this->Semaphore.insert(SemaphoreList.begin(), SemaphoreList.end());
		return SemaphoreList;
	}

	void context::destroy_semaphore(VkSemaphore& aSemaphore) {
		std::vector<VkSemaphore> SemaphoreList = { aSemaphore };
		this->destroy_semaphore(SemaphoreList);
		aSemaphore = VK_NULL_HANDLE;
	}

	void context::destroy_semaphore(std::vector<VkSemaphore>& aSemaphoreList) {
		for (VkSemaphore S : aSemaphoreList) {
			vkDestroySemaphore(this->Handle, S, NULL);
			this->Semaphore.erase(S);
		}
		aSemaphoreList.clear();
	}


	VkFence context::create_fence(VkFenceCreateFlags aFenceCreateFlags) {
		std::vector<VkFence> FenceList = this->create_fence(1, aFenceCreateFlags);
		return FenceList[0];
	}

	std::vector<VkFence> context::create_fence(int aCount, VkFenceCreateFlags aFenceCreateFlags) {
		VkResult Result = VK_SUCCESS;
		std::vector<VkFence> FenceList(aCount);
		VkFenceCreateInfo FenceCreateInfo{};
		FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceCreateInfo.pNext = NULL;
		FenceCreateInfo.flags = aFenceCreateFlags;
		for (size_t i = 0; i < FenceList.size(); i++) {
			Result = vkCreateFence(this->Handle, &FenceCreateInfo, NULL, &FenceList[i]);
		}
		this->Fence.insert(FenceList.begin(), FenceList.end());
		return FenceList;
	}

	void context::destroy_fence(VkFence& aFence) {
		std::vector<VkFence> FenceList = { aFence };
		this->destroy_fence(FenceList);
		aFence = VK_NULL_HANDLE;
	}

	void context::destroy_fence(std::vector<VkFence>& aFenceList) {
		for (VkFence F : aFenceList) {
			vkDestroyFence(this->Handle, F, NULL);
			this->Fence.erase(F);
		}
		aFenceList.clear();
	}

	// Memory Allocation.
	VkDeviceMemory context::allocate_memory(VkMemoryRequirements aMemoryRequirements, uint aMemoryType) {
		VkResult Result = VK_SUCCESS;
		VkDeviceMemory MemoryHandle = VK_NULL_HANDLE;
		// This structure is needed for ray tracing.
		VkMemoryAllocateFlagsInfo MemoryAllocateFlagsInfo{};
		MemoryAllocateFlagsInfo.sType			= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		MemoryAllocateFlagsInfo.pNext			= NULL;
		MemoryAllocateFlagsInfo.flags			= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
		VkMemoryAllocateInfo AllocateInfo{};
		AllocateInfo.sType						= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		if (this->extension_enabled("VK_KHR_buffer_device_address")) {
			AllocateInfo.pNext					= &MemoryAllocateFlagsInfo;
		} else {
			AllocateInfo.pNext					= NULL;
		}
		AllocateInfo.allocationSize				= aMemoryRequirements.size;
		AllocateInfo.memoryTypeIndex			= this->Device->get_memory_type_index(aMemoryRequirements, aMemoryType);
		Result = vkAllocateMemory(this->Handle, &AllocateInfo, NULL, &MemoryHandle);
		this->Memory.insert(MemoryHandle);
		return MemoryHandle;
	}

	void context::free_memory(VkDeviceMemory& aMemoryHandle) {
		vkFreeMemory(this->Handle, aMemoryHandle, NULL);
		this->Memory.erase(aMemoryHandle);
		aMemoryHandle = VK_NULL_HANDLE;
	}

	// Buffer createion

	std::shared_ptr<buffer> context::create_buffer(buffer::create_info aCreateInfo, int aVertexCount, util::variable aVertexLayout, void* aVertexData) {
		return this->create_buffer(aCreateInfo.Memory, aCreateInfo.Usage, aVertexCount * aVertexLayout.size(), aVertexData);
	}

	std::shared_ptr<buffer> context::create_buffer(uint aMemoryType, uint aBufferUsage, int aVertexCount, util::variable aVertexLayout, void* aVertexData) {
		return this->create_buffer(aMemoryType, aBufferUsage, aVertexCount * aVertexLayout.size(), aVertexData);
	}

	std::shared_ptr<buffer> context::create_buffer(buffer::create_info aCreateInfo, size_t aBufferSize, void* aBufferData) {
		return this->create_buffer(aCreateInfo.Memory, aCreateInfo.Usage, aCreateInfo.ElementCount, aBufferSize, aBufferData);
	}

	std::shared_ptr<buffer> context::create_buffer(uint aMemoryType, uint aBufferUsage, size_t aBufferSize, void* aBufferData) {
		return this->create_buffer(aMemoryType, aBufferUsage, 1, aBufferSize, aBufferData);
	}

	std::shared_ptr<buffer> context::create_buffer(uint aMemoryType, uint aBufferUsage, size_t aElementCount, size_t aBufferSize, void* aBufferData) {
		std::shared_ptr<buffer> NewDeviceResource = geodesy::make<buffer>(this->shared_from_this(), aMemoryType, aBufferUsage, aElementCount, aBufferSize, aBufferData);
		return NewDeviceResource;
	}

	std::shared_ptr<image> context::create_image(image::create_info aCreateInfo, std::shared_ptr<image> aHostImage) {
		return this->create_image(aCreateInfo, (image::format)aHostImage->CreateInfo.format, aHostImage->CreateInfo.extent.width, aHostImage->CreateInfo.extent.height, aHostImage->CreateInfo.extent.depth, aHostImage->CreateInfo.arrayLayers, aHostImage->HostData);
	}

	std::shared_ptr<image> context::create_image(image::create_info aCreateInfo, image::format aFormat, uint aX, uint aY, uint aZ, uint aT, void* aTextureData) {
		std::shared_ptr<image> NewDeviceResource = geodesy::make<image>(this->shared_from_this(), aCreateInfo, aFormat, aX, aY, aZ, aT, aTextureData);
		return NewDeviceResource;
	}

	std::shared_ptr<descriptor::array> context::create_descriptor_array(std::shared_ptr<pipeline> aPipeline, VkSamplerCreateInfo aSamplerCreateInfo) {
		std::shared_ptr<descriptor::array> NewDeviceResource = geodesy::make<descriptor::array>(this->shared_from_this(), aPipeline, aSamplerCreateInfo);
		return NewDeviceResource;
	}

	std::shared_ptr<framebuffer> context::create_framebuffer(std::shared_ptr<pipeline> aPipeline, std::vector<std::shared_ptr<image>> aImageAttachements, math::vec<uint, 3> aResolution) {
		std::shared_ptr<framebuffer> NewDeviceResource = geodesy::make<framebuffer>(this->shared_from_this(), aPipeline, aImageAttachements, aResolution);
		return NewDeviceResource;
	}

	std::shared_ptr<framebuffer> context::create_framebuffer(std::shared_ptr<pipeline> aPipeline, std::map<std::string, std::shared_ptr<image>> aImage, std::vector<std::string> aAttachmentSelection, math::vec<uint, 3> aResolution) {
		std::shared_ptr<framebuffer> NewDeviceResource = geodesy::make<framebuffer>(this->shared_from_this(), aPipeline, aImage, aAttachmentSelection, aResolution);
		return NewDeviceResource;
	}

	std::shared_ptr<pipeline> context::create_pipeline(std::shared_ptr<pipeline::rasterizer> aRasterizer, VkRenderPass aRenderPass, uint32_t aSubpassIndex) {
		std::shared_ptr<pipeline> NewDeviceResource = geodesy::make<pipeline>(this->shared_from_this(), aRasterizer, aRenderPass, aSubpassIndex);
		return NewDeviceResource;
	}

	std::shared_ptr<pipeline> context::create_pipeline(std::shared_ptr<pipeline::raytracer> aRayTracer) {
		std::shared_ptr<pipeline> NewDeviceResource = geodesy::make<pipeline>(this->shared_from_this(), aRayTracer);
		return NewDeviceResource;
	}

	std::shared_ptr<gfx::model> context::create_model(std::shared_ptr<gfx::model> aModel, gpu::image::create_info aCreateInfo) {
		std::shared_ptr<gfx::model> NewDeviceResource = geodesy::make<gfx::model>(this->shared_from_this(), aModel, aCreateInfo);
		return NewDeviceResource;
	}

	VkResult context::begin(VkCommandBuffer aCommandBuffer) {
		VkCommandBufferBeginInfo BeginInfo{};
		BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		BeginInfo.pNext = NULL;
		BeginInfo.flags = 0;
		BeginInfo.pInheritanceInfo = NULL;
		return vkBeginCommandBuffer(aCommandBuffer, &BeginInfo);
	}

	VkResult context::end(VkCommandBuffer aCommandBuffer) {
		return vkEndCommandBuffer(aCommandBuffer);
	}

	void context::begin_rendering(VkCommandBuffer aCommandBuffer, VkRect2D aRenderArea, std::vector<VkImageView> aColorAttachments, VkImageView aDepthAttachment, VkImageView aStencilAttachment) {
		std::vector<VkRenderingAttachmentInfo> ColorAttachmentInfo(aColorAttachments.size());
		VkRenderingAttachmentInfo DepthAttachmentInfo{};
		VkRenderingAttachmentInfo StencilAttachmentInfo{};
		VkRenderingInfo RenderingInfo{};

		for (size_t i = 0; i < aColorAttachments.size(); i++) {
			ColorAttachmentInfo[i].sType				= VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			ColorAttachmentInfo[i].pNext				= NULL;
			ColorAttachmentInfo[i].imageView			= aColorAttachments[i];
			ColorAttachmentInfo[i].imageLayout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			ColorAttachmentInfo[i].resolveMode			= VK_RESOLVE_MODE_NONE;
			ColorAttachmentInfo[i].resolveImageView		= VK_NULL_HANDLE;
			ColorAttachmentInfo[i].resolveImageLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			ColorAttachmentInfo[i].loadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			ColorAttachmentInfo[i].storeOp				= VK_ATTACHMENT_STORE_OP_STORE;
			//ColorAttachmentInfo[i].clearValue			= { 0.0f, 0.0f, 0.0f, 0.0f };
		}

		DepthAttachmentInfo.sType					= VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		DepthAttachmentInfo.pNext					= NULL;
		DepthAttachmentInfo.imageView				= aDepthAttachment;
		DepthAttachmentInfo.imageLayout				= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		DepthAttachmentInfo.resolveMode				= VK_RESOLVE_MODE_NONE;
		DepthAttachmentInfo.resolveImageView		= VK_NULL_HANDLE;
		DepthAttachmentInfo.resolveImageLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
		DepthAttachmentInfo.loadOp					= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		DepthAttachmentInfo.storeOp					= VK_ATTACHMENT_STORE_OP_STORE;
		//DepthAttachmentInfo.clearValue				= { 0.0f, 0.0f, 0.0f, 0.0f };

		StencilAttachmentInfo.sType					= VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		StencilAttachmentInfo.pNext					= NULL;
		StencilAttachmentInfo.imageView				= aDepthAttachment;
		StencilAttachmentInfo.imageLayout			= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		StencilAttachmentInfo.resolveMode			= VK_RESOLVE_MODE_NONE;
		StencilAttachmentInfo.resolveImageView		= VK_NULL_HANDLE;
		StencilAttachmentInfo.resolveImageLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
		StencilAttachmentInfo.loadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		StencilAttachmentInfo.storeOp				= VK_ATTACHMENT_STORE_OP_STORE;
		//StencilAttachmentInfo.clearValue			= { 0.0f, 0.0f, 0.0f, 0.0f };

		RenderingInfo.sType							= VK_STRUCTURE_TYPE_RENDERING_INFO;
		RenderingInfo.pNext							= NULL;
		RenderingInfo.flags							= 0;
		RenderingInfo.renderArea					= aRenderArea;
		RenderingInfo.layerCount					= 1;
		RenderingInfo.viewMask						= 1;
		RenderingInfo.colorAttachmentCount			= ColorAttachmentInfo.size();
		RenderingInfo.pColorAttachments				= ColorAttachmentInfo.data();
		RenderingInfo.pDepthAttachment				= &DepthAttachmentInfo;
		RenderingInfo.pStencilAttachment			= &StencilAttachmentInfo;
		vkCmdBeginRendering(aCommandBuffer, &RenderingInfo);
	}

	void context::end_rendering(VkCommandBuffer aCommandBuffer) {
		vkCmdEndRenderPass(aCommandBuffer);
	}

	void context::bind_vertex_buffers(VkCommandBuffer aCommandBuffer, std::vector<VkBuffer> aBufferList, const VkDeviceSize* aOffset) {
		vkCmdBindVertexBuffers(aCommandBuffer, 0, aBufferList.size(), aBufferList.data(), aOffset);
	}

	void context::bind_index_buffer(VkCommandBuffer aCommandBuffer, VkBuffer aBufferHandle, VkIndexType aIndexType) {
		vkCmdBindIndexBuffer(aCommandBuffer, aBufferHandle, 0, aIndexType);
	}

	void context::bind_descriptor_sets(VkCommandBuffer aCommandBuffer, VkPipelineBindPoint aPipelineBindPoint, VkPipelineLayout aPipelineLayout, std::vector<VkDescriptorSet> aDescriptorSetList, std::vector<uint32_t> aDynamicOffsetList) {
		vkCmdBindDescriptorSets(aCommandBuffer, aPipelineBindPoint, aPipelineLayout, 0, aDescriptorSetList.size(), aDescriptorSetList.data(), aDynamicOffsetList.size(), aDynamicOffsetList.data());
	}

	void context::bind_pipeline(VkCommandBuffer aCommandBuffer, VkPipelineBindPoint aPipelineBindPoint, VkPipeline aPipelineHandle) {
		vkCmdBindPipeline(aCommandBuffer, aPipelineBindPoint, aPipelineHandle);
	}

	void context::draw_indexed(VkCommandBuffer aCommandBuffer, uint32_t aIndexCount, uint32_t aInstanceCount, uint32_t aFirstIndex, uint32_t aVertexOffset, uint32_t aFirstInstance) {
		vkCmdDrawIndexed(aCommandBuffer, aIndexCount, aInstanceCount, aFirstIndex, aVertexOffset, aFirstInstance);
	}

	VkResult context::wait() {
		return vkDeviceWaitIdle(this->Handle);
	}

	VkResult context::wait(device::operation aDeviceOperation) {
		return vkQueueWaitIdle(this->Queue[aDeviceOperation]);
	}
	
	VkResult context::wait(VkFence aFence) {
		std::vector<VkFence> FenceList = { aFence };
		return this->wait(FenceList, true);
	}

	VkResult context::wait(std::vector<VkFence> aFenceList, VkBool32 aWaitOnAll) {
		return vkWaitForFences(this->Handle, aFenceList.size(), aFenceList.data(), aWaitOnAll, UINT64_MAX);
	}

	VkResult context::reset(VkFence aFence) {
		std::vector<VkFence> FenceList = { aFence };
		return this->reset(FenceList);
	}

	VkResult context::reset(std::vector<VkFence> aFenceList) {
		return vkResetFences(this->Handle, aFenceList.size(), aFenceList.data());
	}

	VkResult context::wait_and_reset(VkFence aFence) {
		std::vector<VkFence> FenceList = { aFence };
		return this->wait_and_reset(FenceList);
	}

	VkResult context::wait_and_reset(std::vector<VkFence> aFenceList, VkBool32 aWaitOnAll) {
		VkResult Result = VK_SUCCESS;
		Result = this->wait(aFenceList, aWaitOnAll);
		Result = this->reset(aFenceList);
		return Result;
	}

	VkResult context::execute(device::operation aDeviceOperation, VkCommandBuffer aCommandBuffer, VkFence aFence) {
		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.pNext				= NULL;
		SubmitInfo.waitSemaphoreCount	= 0;
		SubmitInfo.pWaitSemaphores		= NULL;
		SubmitInfo.pWaitDstStageMask	= NULL;
		SubmitInfo.commandBufferCount	= 1;
		SubmitInfo.pCommandBuffers		= &aCommandBuffer;
		SubmitInfo.signalSemaphoreCount	= 0;
		SubmitInfo.pSignalSemaphores	= NULL;
		std::vector<VkSubmitInfo> Submission;
		Submission.push_back(SubmitInfo);
		return this->execute(aDeviceOperation, Submission, aFence);
	}

	VkResult context::execute(device::operation aDeviceOperation, const std::vector<VkSubmitInfo>& aSubmissionList, VkFence aFence) {
		if (aSubmissionList.size() == 0) return VK_SUCCESS;
		return vkQueueSubmit(Queue[aDeviceOperation], aSubmissionList.size(), aSubmissionList.data(), aFence);
	}

	VkResult context::execute_and_wait(device::operation aDeviceOperation, VkCommandBuffer aCommandBuffer) {
		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.pNext				= NULL;
		SubmitInfo.waitSemaphoreCount	= 0;
		SubmitInfo.pWaitSemaphores		= NULL;
		SubmitInfo.pWaitDstStageMask	= NULL;
		SubmitInfo.commandBufferCount	= 1;
		SubmitInfo.pCommandBuffers		= &aCommandBuffer;
		SubmitInfo.signalSemaphoreCount	= 0;
		SubmitInfo.pSignalSemaphores	= NULL;
		std::vector<VkSubmitInfo> Submission;
		Submission.push_back(SubmitInfo);
		return this->execute_and_wait(aDeviceOperation, Submission);
	}

	VkResult context::execute_and_wait(device::operation aDeviceOperation, const std::vector<VkSubmitInfo>& aSubmissionList) {
		VkResult Result = VK_SUCCESS;
		VkFence Fence = this->create_fence();
		Result = this->execute(aDeviceOperation, aSubmissionList, Fence);
		Result = this->wait(Fence);
		this->destroy_fence(Fence);
		return Result;
	}

	VkResult context::engine_wait(std::vector<device::operation> aDeviceOperation) {
		VkResult Result = VK_SUCCESS;
		for (device::operation Op : aDeviceOperation) {
			if ((InFlight.count(Op) > 0) && InFlight[Op]) {
				Result = this->wait_and_reset(ExecutionFence[Op]);
				InFlight[Op] = false;
			}
		}
		return Result;
	}

	VkResult context::engine_execute(device::operation aDeviceOperation, const std::vector<VkSubmitInfo>& aSubmissionList) {
		VkResult Result = VK_SUCCESS;
		if (aSubmissionList.size() > 0) {
			this->execute(aDeviceOperation, aSubmissionList, ExecutionFence[aDeviceOperation]);
			this->InFlight[aDeviceOperation] = true;
		}
		return Result;
	}

}