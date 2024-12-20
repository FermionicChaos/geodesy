#include <geodesy/core/gcl/descriptor.h>
#include <geodesy/core/gcl/pipeline.h>
#include <geodesy/core/gcl/context.h>

namespace geodesy::core::gcl {

	const VkSamplerCreateInfo descriptor::DefaultSamplerCreateInfo = {
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		NULL,
		0,
		VK_FILTER_LINEAR,
		VK_FILTER_LINEAR,
		VK_SAMPLER_MIPMAP_MODE_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VK_SAMPLER_ADDRESS_MODE_REPEAT,
		0.0f,
		VK_FALSE,
		1.0f,
		VK_FALSE,
		VK_COMPARE_OP_ALWAYS,
		0.0f,
		0.0f,
		VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		VK_FALSE
	};

	// descriptor::pool::pool(std::shared_ptr<context> aContext, std::vector<std::shared_ptr<pipeline>> aPipeline, std::size_t aMultiplier) {
	// 	VkResult Result = VK_SUCCESS;
	// 	std::size_t MaxSetCount = 0;

	// 	// Determine the total number of Descriptor Sets from each pipeline.
	// 	for (std::size_t i = 0; i < aPipeline.size(); i++) {
	// 		// Determines the set count proportional to the number of descriptor set layouts
	// 		// in each pipeline.
	// 		MaxSetCount += aPipeline[i]->DescriptorSetLayout.size();
	// 	}

	// 	// Aggregate all descriptor types require by pipeline list.
	// 	std::map<VkDescriptorType, uint32_t> DescriptorTypeCount;
	// 	for (std::size_t i = 0; i < aPipeline.size(); i++) {
	// 		std::map<VkDescriptorType, uint32_t> Temp = aPipeline[i]->descriptor_type_count();
	// 		for (auto& [Type, Count] : Temp) {
	// 			if (DescriptorTypeCount.count(Type) == 0) {
	// 				DescriptorTypeCount[Type] = 0;
	// 			}
	// 			DescriptorTypeCount[Type] += Count;
	// 		}
	// 	}

	// 	// Convert to pool size to vector data structure.
	// 	std::vector<VkDescriptorPoolSize> DescriptorPoolSize;
	// 	for (auto& [Type, Count] : DescriptorTypeCount) {
	// 		VkDescriptorPoolSize DPS{};
	// 		DPS.type = Type;
	// 		DPS.descriptorCount = Count;
	// 		DescriptorPoolSize.push_back(DPS);
	// 	}

	// 	// Apply Multiplier to Max Set count, and descriptor pool sizes.
	// 	MaxSetCount *= aMultiplier;
	// 	for (std::size_t i = 0; i < DescriptorPoolSize.size(); i++) {
	// 		DescriptorPoolSize[i].descriptorCount *= aMultiplier;
	// 	}

	// 	VkDescriptorPoolCreateInfo DPCI{};
	// 	DPCI.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	// 	DPCI.pNext				= NULL;
	// 	DPCI.flags				= 0;
	// 	DPCI.maxSets			= MaxSetCount;
	// 	DPCI.poolSizeCount		= DescriptorPoolSize.size();
	// 	DPCI.pPoolSizes			= DescriptorPoolSize.data();
	// 	Result = vkCreateDescriptorPool(aContext->Handle, &DPCI, NULL, &this->Handle);
	// }

	// descriptor::pool::~pool() {}
	
	descriptor::array::array(std::shared_ptr<context> aContext, std::shared_ptr<pipeline> aPipeline, VkSamplerCreateInfo aSamplerCreateInfo) {
		VkResult Result = VK_SUCCESS;
		this->DescriptorSetLayoutBinding = aPipeline->descriptor_set_layout_binding();
		this->Context = aContext;

		// Get Descriptor Pool Sizes based on glslang API reflection from shader stages.
		std::vector<VkDescriptorPoolSize> DescriptorPoolSize = aPipeline->descriptor_pool_sizes();

		VkDescriptorPoolCreateInfo DPCI{};
		DPCI.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		DPCI.pNext				= NULL;
		DPCI.flags				= 0;
		DPCI.maxSets			= aPipeline->DescriptorSetLayout.size();
		DPCI.poolSizeCount		= DescriptorPoolSize.size();
		DPCI.pPoolSizes			= DescriptorPoolSize.data();
		Result = vkCreateDescriptorPool(aContext->Handle, &DPCI, NULL, &this->DescriptorPool);

		// Allocate Descriptor Sets
		VkDescriptorSetAllocateInfo DSAI{};
		DSAI.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		DSAI.pNext					= NULL;
		DSAI.descriptorPool			= this->DescriptorPool;
		DSAI.descriptorSetCount		= aPipeline->DescriptorSetLayout.size();
		DSAI.pSetLayouts			= aPipeline->DescriptorSetLayout.data();
		this->DescriptorSet = std::vector<VkDescriptorSet>(aPipeline->DescriptorSetLayout.size());
		Result = vkAllocateDescriptorSets(aContext->Handle, &DSAI, this->DescriptorSet.data());

		// Create Sampler Info
		Result = vkCreateSampler(aContext->Handle, &aSamplerCreateInfo, NULL, &this->SamplingMetadata);
	}

	// descriptor::array::array(std::shared_ptr<context> aContext, std::shared_ptr<descriptor::pool> aDescriptorPool, std::shared_ptr<pipeline> aPipeline, VkSamplerCreateInfo aSamplerCreateInfo) : array(aContext, aDescriptorPool->Handle, aPipeline, aSamplerCreateInfo) {}

	descriptor::array::~array() {
		//! Not needed since freeing the pool frees the sets.
		// Free Descriptor Sets
		// vkFreeDescriptorSets(this->Context->Handle, this->DescriptorPool, this->DescriptorSet.size(), this->DescriptorSet.data());

		// Destroy Descriptor Pool
		vkDestroyDescriptorPool(this->Context->Handle, this->DescriptorPool, NULL);

		// Destroy Sampler
		vkDestroySampler(this->Context->Handle, this->SamplingMetadata, NULL);
	}

	void descriptor::array::array::bind(uint32_t aSet, uint32_t aBinding, uint32_t aArrayElement, std::shared_ptr<image> aImage, image::layout aImageLayout) {
		if ((aSet >= this->DescriptorSetLayoutBinding.size()) || (aBinding >= this->DescriptorSetLayoutBinding[aSet].size())) return;
		VkDescriptorImageInfo DII{};
		DII.imageView			= aImage->View;
		DII.imageLayout			= (VkImageLayout)aImageLayout;
		DII.sampler				= this->SamplingMetadata;
		VkWriteDescriptorSet WDS {};
		WDS.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		WDS.pNext				= NULL;
		WDS.dstSet				= this->DescriptorSet[aSet];
		WDS.dstBinding			= aBinding;
		WDS.dstArrayElement		= aArrayElement;
		WDS.descriptorCount		= 1;
		WDS.descriptorType		= this->DescriptorSetLayoutBinding[aSet][aBinding].descriptorType;
		WDS.pImageInfo			= &DII;
		WDS.pBufferInfo			= NULL;
		WDS.pTexelBufferView	= NULL;
		vkUpdateDescriptorSets(this->Context->Handle, 1, &WDS, 0, NULL);
	}
	
	void descriptor::array::bind(uint32_t aSet, uint32_t aBinding, uint32_t aArrayElement, std::shared_ptr<buffer> aBuffer, size_t aSize, size_t aOffset) {
		if ((aSet >= this->DescriptorSetLayoutBinding.size()) || (aBinding >= this->DescriptorSetLayoutBinding[aSet].size())) return;
		VkDescriptorBufferInfo DBI{};
		DBI.buffer				= aBuffer->Handle;
		DBI.offset				= aOffset;
		DBI.range				= aSize;
		VkWriteDescriptorSet WDS {};
		WDS.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		WDS.pNext				= NULL;
		WDS.dstSet				= this->DescriptorSet[aSet];
		WDS.dstBinding			= aBinding;
		WDS.dstArrayElement		= aArrayElement;
		WDS.descriptorCount		= 1;
		WDS.descriptorType		= this->DescriptorSetLayoutBinding[aSet][aBinding].descriptorType;
		WDS.pImageInfo			= NULL;
		WDS.pBufferInfo			= &DBI;
		WDS.pTexelBufferView	= NULL;
		vkUpdateDescriptorSets(this->Context->Handle, 1, &WDS, 0, NULL);	
	}
	
	void descriptor::array::bind(uint32_t aSet, uint32_t aBinding, uint32_t aArrayElement, VkBufferView aBufferView) {
		if ((aSet >= this->DescriptorSetLayoutBinding.size()) || (aBinding >= this->DescriptorSetLayoutBinding[aSet].size())) return;
		VkWriteDescriptorSet WDS {};
		WDS.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		WDS.pNext				= NULL;
		WDS.dstSet				= this->DescriptorSet[aSet];
		WDS.dstBinding			= aBinding;
		WDS.dstArrayElement		= aArrayElement;
		WDS.descriptorCount		= 1;
		WDS.descriptorType		= this->DescriptorSetLayoutBinding[aSet][aBinding].descriptorType;
		WDS.pImageInfo			= NULL;
		WDS.pBufferInfo			= NULL;
		WDS.pTexelBufferView	= &aBufferView;
		vkUpdateDescriptorSets(this->Context->Handle, 1, &WDS, 0, NULL);	
	}
	
}