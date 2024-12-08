#pragma once
#ifndef GEODESY_CORE_GCL_DESCRIPTOR_H
#define GEODESY_CORE_GCL_DESCRIPTOR_H

#include "../../config.h"
#include "config.h"

#include "buffer.h"
#include "image.h"

namespace geodesy::core::gcl {

	class pipeline;

	class descriptor {
	public:
		
		static const VkSamplerCreateInfo 								DefaultSamplerCreateInfo;
		
		// class pool {
		// public:
		//
		// 	VkDescriptorPool Handle;
		//
		// 	pool(std::shared_ptr<context> aContext, std::vector<std::shared_ptr<pipeline>> aPipeline, std::size_t aMultiplier = 1);
		// 	~pool();
		//
		// };

		// This class descriptor::array is intended to carry a series of descriptor sets designed specifically
		// for binding to a specific pipeline. 
		class array {
		public:

			std::vector<std::vector<VkDescriptorSetLayoutBinding>> 		DescriptorSetLayoutBinding;		// Contains map of available (set, binding) pairs which resources can be bound to.
			std::shared_ptr<context> 									Context;						// Context for GPU managed objects.
			VkDescriptorPool 											DescriptorPool;					// Descriptor Pool for managing descriptor sets.
			std::vector<VkDescriptorSet> 								DescriptorSet;					// DS used for binding resource references to pipeline.
			VkSampler 													SamplingMetadata;				// Sampling metadata for the descriptor set.

			array() {}
			array(std::shared_ptr<context> aContext, std::shared_ptr<pipeline> aPipeline, VkSamplerCreateInfo aSamplerCreateInfo = DefaultSamplerCreateInfo);
			~array();

			void bind(uint32_t aSet, uint32_t aBinding, uint32_t aArrayElement, std::shared_ptr<image> aImage, image::layout aImageLayout = image::layout::SHADER_READ_ONLY_OPTIMAL);
			void bind(uint32_t aSet, uint32_t aBinding, uint32_t aArrayElement, std::shared_ptr<buffer> aBuffer, size_t aSize = VK_WHOLE_SIZE, size_t aOffset = 0);
			void bind(uint32_t aSet, uint32_t aBinding, uint32_t aArrayElement, VkBufferView aBufferView);

		};

	};

	// This object is an abstraction over vulkan descriptor sets, intended to be used and bound
	// to a single pipeline/subpass. Since these are opaque handles to GPU objects, theire lifetimes
	// have to be memory managed. This is a crude RAII wrapper for descriptor pools, sets, and sampling info.
	// class uniform_array {
	// public:

	// 	std::vector<std::vector<VkDescriptorSetLayoutBinding>> 		DescriptorSetLayoutBinding;

	// 	std::shared_ptr<context> 									Context;
	// 	VkDescriptorPool 											DescriptorPool;
	// 	std::vector<VkDescriptorSet> 								DescriptorSet;
	// 	std::vector<std::vector<VkSampler>> 						SamplingMetadata;

	// 	uniform_array(std::shared_ptr<context> aContext, VkDescriptorPool aDescriptorPool, std::shared_ptr<pipeline> aPipeline);
	// 	~uniform_array();

	// 	void bind(uint32_t aSet, uint32_t aBinding, uint32_t aArrayElement, VkImageView aImageView, VkImageLayout aImageLayout, VkSamplerCreateInfo aSamplerCreateInfo);
	// 	void bind(uint32_t aSet, uint32_t aBinding, uint32_t aArrayElement, std::shared_ptr<buffer> aBuffer, size_t aSize = VK_WHOLE_SIZE, size_t aOffset = 0);
	// 	void bind(uint32_t aSet, uint32_t aBinding, uint32_t aArrayElement, VkBufferView aBufferView);

	// };

}

#endif // GEODESY_CORE_GCL_DESCRIPTOR_H
