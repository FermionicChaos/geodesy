#include <geodesy/core/gpu/descriptor.h>
#include <geodesy/core/gpu/pipeline.h>
#include <geodesy/core/gpu/context.h>

namespace geodesy::core::gpu {

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

	descriptor::array::~array() {
		//! Not needed since freeing the pool frees the sets.
		// Free Descriptor Sets
		// vkFreeDescriptorSets(this->Context->Handle, this->DescriptorPool, this->DescriptorSet.size(), this->DescriptorSet.data());

		// Destroy Descriptor Pool
		vkDestroyDescriptorPool(this->Context->Handle, this->DescriptorPool, NULL);

		// Destroy Sampler
		vkDestroySampler(this->Context->Handle, this->SamplingMetadata, NULL);
	}

	void descriptor::array::array::bind(int aSet, int aBinding, int aArrayElement, std::shared_ptr<image> aImage, image::layout aImageLayout) {
		if ((!this->exists(aSet, aBinding)) || (aImage == nullptr)) return;
		VkDescriptorSetLayoutBinding DSLB = this->get_descriptor_set_layout_binding(aSet, aBinding);
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
		WDS.descriptorType		= DSLB.descriptorType;
		WDS.pImageInfo			= &DII;
		WDS.pBufferInfo			= NULL;
		WDS.pTexelBufferView	= NULL;
		vkUpdateDescriptorSets(this->Context->Handle, 1, &WDS, 0, NULL);
	}
	
	void descriptor::array::bind(int aSet, int aBinding, int aArrayElement, std::shared_ptr<buffer> aBuffer, size_t aSize, size_t aOffset) {
		if ((!this->exists(aSet, aBinding)) || (aBuffer == nullptr)) return;
		VkDescriptorSetLayoutBinding DSLB = this->get_descriptor_set_layout_binding(aSet, aBinding);
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
		WDS.descriptorType		= DSLB.descriptorType;
		WDS.pImageInfo			= NULL;
		WDS.pBufferInfo			= &DBI;
		WDS.pTexelBufferView	= NULL;
		vkUpdateDescriptorSets(this->Context->Handle, 1, &WDS, 0, NULL);	
	}

	void descriptor::array::bind(int aSet, int aBinding, int aArrayElement, std::shared_ptr<acceleration_structure> aAccelerationStructure) {
		if ((!this->exists(aSet, aBinding)) || (aAccelerationStructure == nullptr)) return;
		VkDescriptorSetLayoutBinding DSLB = this->get_descriptor_set_layout_binding(aSet, aBinding);
		VkWriteDescriptorSetAccelerationStructureKHR WDSAS{};
		WDSAS.sType								= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		WDSAS.pNext								= NULL;
		WDSAS.accelerationStructureCount		= 1;
		WDSAS.pAccelerationStructures			= &aAccelerationStructure->Handle;
		VkWriteDescriptorSet WDS {};
		WDS.sType								= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		WDS.pNext								= &WDSAS;
		WDS.dstSet								= this->DescriptorSet[aSet];
		WDS.dstBinding							= aBinding;
		WDS.dstArrayElement						= aArrayElement;
		WDS.descriptorCount						= 1;
		WDS.descriptorType						= DSLB.descriptorType;
		WDS.pImageInfo							= NULL;
		WDS.pBufferInfo							= NULL;
		WDS.pTexelBufferView					= NULL;
	}
	
	void descriptor::array::bind(int aSet, int aBinding, int aArrayElement, VkBufferView aBufferView) {
		if ((!this->exists(aSet, aBinding)) || (aBufferView == VK_NULL_HANDLE)) return;
		VkDescriptorSetLayoutBinding DSLB = this->get_descriptor_set_layout_binding(aSet, aBinding);
		VkWriteDescriptorSet WDS {};
		WDS.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		WDS.pNext				= NULL;
		WDS.dstSet				= this->DescriptorSet[aSet];
		WDS.dstBinding			= aBinding;
		WDS.dstArrayElement		= aArrayElement;
		WDS.descriptorCount		= 1;
		WDS.descriptorType		= DSLB.descriptorType;
		WDS.pImageInfo			= NULL;
		WDS.pBufferInfo			= NULL;
		WDS.pTexelBufferView	= &aBufferView;
		vkUpdateDescriptorSets(this->Context->Handle, 1, &WDS, 0, NULL);	
	}

	bool descriptor::array::exists(int aSet, int aBinding) {
		bool SetBindingExists = false;
		if (aSet < this->DescriptorSetLayoutBinding.size()) {
			for (const VkDescriptorSetLayoutBinding& DSLB : this->DescriptorSetLayoutBinding[aSet]) {
				SetBindingExists |= (aBinding == DSLB.binding);
			}
		}
		return SetBindingExists;
	}

	// This is a safe way to get the descriptor set layout binding for a specific set and binding.
	// in case compiler optimizes out uniform variables and bindings have gaps.
	VkDescriptorSetLayoutBinding descriptor::array::get_descriptor_set_layout_binding(int aSet, int aBinding) {
		VkDescriptorSetLayoutBinding DSLB{};
		if (aSet < this->DescriptorSetLayoutBinding.size()) {
			for (const VkDescriptorSetLayoutBinding& DSLB : this->DescriptorSetLayoutBinding[aSet]) {
				if (aBinding == DSLB.binding) {
					return DSLB;
				}
			}
		}
		return DSLB;
	}
	
}