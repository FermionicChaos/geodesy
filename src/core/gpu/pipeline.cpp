#include <geodesy/core/gpu/pipeline.h>
#include <geodesy/core/gpu/context.h>

#include "glslang_util.h"

#include <iostream>

namespace geodesy::core::gpu {

	using namespace util;

	void pipeline::create_info::generate_descriptor_set_layout_binding() {
		// This reduces code redundancy in DSLB for each pipeline type. Now all pipeline types
		// will extract uniform metadata in the same fashion. 

		// Get Uniform Blocks (uniform buffers).
		for (size_t i = 0; i < this->Program->getNumUniformBlocks(); i++) {
			VkDescriptorSetLayoutBinding DSLB{};
			const glslang::TObjectReflection& Variable = this->Program->getUniformBlock(i);
			const glslang::TType* Type = Variable.getType();
			const glslang::TArraySizes* ArraySize = Type->getArraySizes();
			size_t DescriptorCount = 0;
			if (ArraySize != NULL) {
				for (int j = 0; j < ArraySize->getNumDims(); j++) {
					DescriptorCount += ArraySize->getDimSize(j);
				}
			} else {
				DescriptorCount = 1;
			}

			int SetIndex = Type->getQualifier().layoutSet;
			int BindingIndex = Type->getQualifier().layoutBinding;
			std::pair<int, int> SetBinding = std::make_pair(SetIndex, BindingIndex);

			// Generate Bindings for Uniform Buffers.
			DSLB.binding = BindingIndex;
			DSLB.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			DSLB.descriptorCount = DescriptorCount;
			DSLB.stageFlags = glslang_shader_stage_to_vulkan(Variable.stages);
			DSLB.pImmutableSamplers = NULL;

			// Resize DescriptorSetLayoutBinding if SetIndex does not exist.
			if (SetIndex >= this->DescriptorSetLayoutBinding.size()) {
				this->DescriptorSetLayoutBinding.resize(SetIndex + 1);
			}

			this->DescriptorSetLayoutBinding[SetIndex].push_back(DSLB);
			this->DescriptorSetVariable[SetBinding] = convert_to_variable(Type, Variable.name.c_str());
		}
		
		// Gets buffer blocks (storage buffers).
		for (size_t i = 0; i < this->Program->getNumBufferBlocks(); i++) {
			VkDescriptorSetLayoutBinding DSLB{};
			const glslang::TObjectReflection& Variable = this->Program->getBufferBlock(i);
			const glslang::TType* Type = Variable.getType();
			const glslang::TArraySizes* ArraySize = Type->getArraySizes();
			size_t DescriptorCount = 0;
			if (ArraySize != NULL) {
				for (int j = 0; j < ArraySize->getNumDims(); j++) {
					DescriptorCount += ArraySize->getDimSize(j);
				}
			} else {
				DescriptorCount = 1;
			}
		
			int SetIndex = Type->getQualifier().layoutSet;
			int BindingIndex = Type->getQualifier().layoutBinding;
			std::pair<int, int> SetBinding = std::make_pair(SetIndex, BindingIndex);
		
			// Generate Bindings for Storage Buffers.
			DSLB.binding = BindingIndex;
			DSLB.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			DSLB.descriptorCount = DescriptorCount;
			DSLB.stageFlags = glslang_shader_stage_to_vulkan(Variable.stages);
			DSLB.pImmutableSamplers = NULL;
		
			// Resize DescriptorSetLayoutBinding if SetIndex does not exist.
			if (SetIndex >= this->DescriptorSetLayoutBinding.size()) {
				this->DescriptorSetLayoutBinding.resize(SetIndex + 1);
			}
		
			this->DescriptorSetLayoutBinding[SetIndex].push_back(DSLB);
			this->DescriptorSetVariable[SetBinding] = convert_to_variable(Type, Variable.name.c_str());
		}
		
		// Acquires uniform images, samplers, acceleration structures.
		for (int i = 0; i < this->Program->getNumUniformVariables(); i++) {
			const glslang::TObjectReflection& Variable = this->Program->getUniform(i);
			const glslang::TType* Type = Variable.getType();				
			const glslang::TArraySizes* ArraySize = Type->getArraySizes();
			size_t DescriptorCount = 0;
			if (ArraySize != NULL) {
				for (int j = 0; j < ArraySize->getNumDims(); j++) {
					DescriptorCount += ArraySize->getDimSize(j);
				}
			} else {
				DescriptorCount = 1;
			}

			int SetIndex = Type->getQualifier().layoutSet;
			int BindingIndex = Type->getQualifier().layoutBinding;
			std::pair<int, int> SetBinding = std::make_pair(SetIndex, BindingIndex);

			VkDescriptorSetLayoutBinding DSLB{};
			DSLB.binding = BindingIndex;
			switch(Type->getBasicType()){
			case glslang::TBasicType::EbtSampler:
				if (Type->getSampler().isImage()) {
					// Storage Image (read/write image)
					DSLB.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				} else if (Type->getSampler().isBuffer()) {
					// Texel buffer - either uniform (read-only) or storage (read-write)
					if (Type->getSampler().isImageClass()) {
						// Storage texel buffer (read-write)
						DSLB.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
					} else {
						// Uniform texel buffer (read-only)
						DSLB.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
					}
				} else if (Type->getSampler().isPureSampler()) {
					// Standalone sampler
					DSLB.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				} else {
					// Regular Combined Image Sampler
					DSLB.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				}
				break;
			case glslang::TBasicType::EbtAccStruct:
				// Acceleration Structure (top-level or bottom-level)
				DSLB.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
				break;
			default:
				// TODO: Add other variables later.
				// Skip other types we don't care for.
				continue;
			}
			DSLB.descriptorCount = DescriptorCount;
			DSLB.stageFlags = glslang_shader_stage_to_vulkan(Variable.stages);
			DSLB.pImmutableSamplers = NULL;

			// Resize DescriptorSetLayoutBinding if SetIndex does not exist
			if (SetIndex >= this->DescriptorSetLayoutBinding.size()) {
				this->DescriptorSetLayoutBinding.resize(SetIndex + 1);
			}

			this->DescriptorSetLayoutBinding[SetIndex].push_back(DSLB);
			this->DescriptorSetVariable[SetBinding] = convert_to_variable(Type, Variable.name.c_str());
		}
	}

	pipeline::rasterizer::rasterizer() {
		this->BindPoint 									= type::RASTERIZER;
		this->InputAssembly 								= {};
		this->Tesselation 									= {};
		this->Viewport 										= {};
		this->Rasterizer 									= {};
		this->Multisample 									= {};
		this->DepthStencil 									= {};
		this->ColorBlend 									= {};
		this->DynamicState 									= {};

		this->InputAssembly.sType 							= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		this->InputAssembly.pNext 							= NULL;
		this->InputAssembly.flags 							= 0;

		this->Tesselation.sType 							= VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		this->Tesselation.pNext 							= NULL;
		this->Tesselation.flags 							= 0;

		this->Viewport.sType 								= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		this->Viewport.pNext 								= NULL;
		this->Viewport.flags 								= 0;

		this->Rasterizer.sType 								= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		this->Rasterizer.pNext 								= NULL;
		this->Rasterizer.flags 								= 0;

		this->Rasterizer.depthClampEnable 					= VK_FALSE;
		this->Rasterizer.rasterizerDiscardEnable 			= VK_FALSE;
		this->Rasterizer.polygonMode 						= VK_POLYGON_MODE_FILL;
		this->Rasterizer.cullMode 							= VK_CULL_MODE_NONE;
		this->Rasterizer.frontFace 							= VK_FRONT_FACE_COUNTER_CLOCKWISE;
		this->Rasterizer.depthBiasEnable 					= VK_FALSE;
		this->Rasterizer.depthBiasConstantFactor 			= 0.0f;
		this->Rasterizer.depthBiasClamp 					= 0.0f;
		this->Rasterizer.depthBiasSlopeFactor 				= 0.0f;
		this->Rasterizer.lineWidth 							= 1.0f;

		this->Multisample.sType 							= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		this->Multisample.pNext 							= NULL;
		this->Multisample.flags 							= 0;
		//this->Multisample.rasterizationSamples		= (VkSampleCountFlagBits)0;
		this->Multisample.sampleShadingEnable 				= VK_FALSE;
		this->Multisample.minSampleShading 					= 1.0f;
		this->Multisample.pSampleMask 						= NULL;
		this->Multisample.alphaToCoverageEnable 			= VK_FALSE;
		this->Multisample.alphaToOneEnable 					= VK_FALSE;

		this->DepthStencil.sType 							= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		this->DepthStencil.pNext 							= NULL;
		this->DepthStencil.flags 							= 0;
		this->DepthStencil.depthTestEnable 					= VK_FALSE;
		this->DepthStencil.depthWriteEnable 				= VK_FALSE;
		this->DepthStencil.depthCompareOp 					= VK_COMPARE_OP_GREATER_OR_EQUAL; // Camera, +z is closer.
		this->DepthStencil.depthBoundsTestEnable 			= VK_FALSE;
		this->DepthStencil.stencilTestEnable 				= VK_FALSE;
		this->DepthStencil.front 							= {};
		this->DepthStencil.back 							= {};
		this->DepthStencil.minDepthBounds 					= 0.0f;
		this->DepthStencil.maxDepthBounds 					= 1.0f;

		this->ColorBlend.sType 								= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		this->ColorBlend.pNext 								= NULL;
		this->ColorBlend.flags 								= 0;
		this->ColorBlend.logicOpEnable 						= VK_FALSE;
		this->ColorBlend.logicOp 							= VK_LOGIC_OP_COPY;
		this->ColorBlend.attachmentCount 					= 0;
		this->ColorBlend.pAttachments 						= NULL;
		this->ColorBlend.blendConstants[0] 					= 0.0f;
		this->ColorBlend.blendConstants[1] 					= 0.0f;
		this->ColorBlend.blendConstants[2] 					= 0.0f;
		this->ColorBlend.blendConstants[3] 					= 0.0f;

		this->DynamicState.sType 							= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		this->DynamicState.pNext 							= NULL;
		this->DynamicState.flags 							= 0;

		this->DepthStencilAttachment.Description.flags 				= 0;
		this->DepthStencilAttachment.Description.format 			= VK_FORMAT_UNDEFINED;
		this->DepthStencilAttachment.Description.samples 			= VK_SAMPLE_COUNT_1_BIT;
		this->DepthStencilAttachment.Description.loadOp 			= VK_ATTACHMENT_LOAD_OP_LOAD;
		this->DepthStencilAttachment.Description.storeOp 			= VK_ATTACHMENT_STORE_OP_STORE;
		this->DepthStencilAttachment.Description.stencilLoadOp 		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		this->DepthStencilAttachment.Description.stencilStoreOp 	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		this->DepthStencilAttachment.Description.initialLayout 		= VK_IMAGE_LAYOUT_UNDEFINED;
		this->DepthStencilAttachment.Description.finalLayout 		= VK_IMAGE_LAYOUT_UNDEFINED;
	}

	pipeline::rasterizer::rasterizer(std::vector<std::shared_ptr<shader>> aShaderList, math::vec<uint, 3> aResolution) : rasterizer() {
		bool Success = true;

		this->Resolution = aResolution;
		this->DefaultViewport.push_back({ 0.0f, 0.0f, (float)aResolution[0], (float)aResolution[1], 0.0f, 1.0f });
		this->DefaultScissor.push_back({ { 0, 0 }, { aResolution[0], aResolution[1] } });

		// Load shaders.
		this->Shader = aShaderList;

		// Link Shader Stages.
		if (Success) {
			EShMessages Message = (EShMessages)(
				EShMessages::EShMsgAST |
				EShMessages::EShMsgSpvRules |
				EShMessages::EShMsgVulkanRules |
				EShMessages::EShMsgDebugInfo |
				EShMessages::EShMsgBuiltinSymbolTable
			);

			// Link various shader stages together.
			this->Program = std::make_shared<glslang::TProgram>();
			for (std::shared_ptr<shader> Shd : Shader) {
				this->Program->addShader(Shd->Handle.get());
			}

			// Link Shader Stages
			Success = this->Program->link(Message);

			// Check if Link was successful
			if (!Success) {
				std::cout << this->Program->getInfoLog() << std::endl;
			}
		}
		else {
			// No shaders provided.
			Success = false;
		}

		// Build API reflection.
		if (Success) {
			// Generates API Reflection.
			this->Program->buildReflection(EShReflectionAllIOVariables);

			// DISABLED: Create Descriptor Set Layouts to max spec required minimum (4).
			// this->DescriptorSetLayoutBinding.resize(GPU_DESCRIPTOR_SET_COUNT);

			// -------------------- START -------------------- //

			// Get Vertex Attribute Inputs
			this->VertexAttribute = std::vector<attribute>(this->Program->getNumPipeInputs());
			for (size_t i = 0; i < this->VertexAttribute.size(); i++) {
				const glslang::TObjectReflection& Variable 				= this->Program->getPipeInput(i);
				const glslang::TType* Type 								= Variable.getType();

				int Location 											= Type->getQualifier().layoutLocation;

				if (Location >= this->VertexAttribute.size()) {
					// Throw runtime error, locations must be continguous.
					throw std::runtime_error("Vertex Attribute Locations must be contiguous.");
				}

				this->VertexAttribute[Location].Variable 				= convert_to_variable(Type, Variable.name.c_str());

				// Load Attribute Descriptions
				this->VertexAttribute[Location].Description.location 	= Location;
				this->VertexAttribute[Location].Description.binding 	= 0;
				this->VertexAttribute[Location].Description.format 		= (VkFormat)image::t2f(this->VertexAttribute[Location].Variable.Type.ID);
				this->VertexAttribute[Location].Description.offset 		= 0;
			}

			// Get Framebuffer Attachment Outputs
			this->ColorAttachment = std::vector<struct attachment>(this->Program->getNumPipeOutputs());
			for (size_t i = 0; i < this->ColorAttachment.size(); i++) {
				const glslang::TObjectReflection& Variable 						= this->Program->getPipeOutput(i);
				const glslang::TType* Type 										= Variable.getType();

				int Location 													= Type->getQualifier().layoutLocation;

				if (Location >= this->ColorAttachment.size()) {
					// Throw runtime error, locations must be continguous.
					throw std::runtime_error("Color Attachment Locations must be contiguous.");
				}

				this->ColorAttachment[Location].Variable 						= convert_to_variable(Type, Variable.name.c_str());
				this->ColorAttachment[Location].Description.flags				= 0;
				this->ColorAttachment[Location].Description.format				= (VkFormat)image::t2f(this->ColorAttachment[Location].Variable.Type.ID);
				this->ColorAttachment[Location].Description.samples				= VK_SAMPLE_COUNT_1_BIT;
				this->ColorAttachment[Location].Description.loadOp				= VK_ATTACHMENT_LOAD_OP_LOAD;
				this->ColorAttachment[Location].Description.storeOp				= VK_ATTACHMENT_STORE_OP_STORE;
				this->ColorAttachment[Location].Description.stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				this->ColorAttachment[Location].Description.stencilStoreOp		= VK_ATTACHMENT_STORE_OP_DONT_CARE;
				this->ColorAttachment[Location].Description.initialLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
				this->ColorAttachment[Location].Description.finalLayout			= VK_IMAGE_LAYOUT_UNDEFINED;
			}

			// Generates Descriptor Set Layout Bindings.
			this->generate_descriptor_set_layout_binding();

			std::cout << "// -------------------- Rasterization Pipeline Reflection Start -------------------- \\\\" << std::endl << std::endl;

			// Print Inputs
			std::cout << "----- Vertex Input Attributes -----" << std::endl;
			for (size_t i = 0; i < this->VertexAttribute.size(); i++) {
				std::cout << "layout (location = " << i << ") in " << this->VertexAttribute[i].Variable;
			}
			std::cout << std::endl;

			// Print Uniforms
			std::cout << "----- Uniform Objects -----" << std::endl;
			for (std::pair<std::pair<int, int>, util::variable> Variable : this->DescriptorSetVariable) {
				std::cout << "layout (set = " << Variable.first.first << ", binding = " << Variable.first.second << ") uniform " << Variable.second;
			}
			std::cout << std::endl;

			// Print Outputs
			std::cout << "----- Framebuffer Attachment Outputs -----" << std::endl;
			for (size_t i = 0; i < this->ColorAttachment.size(); i++) {
				std::cout << "layout (location = " << i << ") out " << this->ColorAttachment[i].Variable;
			}
			std::cout << std::endl;

			std::cout << "\\\\ -------------------- Rasterization Pipeline Reflection End -------------------- //" << std::endl;
		}
		else {
			// Linking failed.
			Success = false;
		}

		this->AttachmentBlendingRules = std::vector<VkPipelineColorBlendAttachmentState>(this->ColorAttachment.size());
		for (size_t i = 0; i < AttachmentBlendingRules.size(); i++) {
			// Default blending operations I like.
			AttachmentBlendingRules[i].blendEnable					= VK_FALSE;
			AttachmentBlendingRules[i].srcColorBlendFactor			= VK_BLEND_FACTOR_SRC_ALPHA;
			AttachmentBlendingRules[i].dstColorBlendFactor			= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			AttachmentBlendingRules[i].colorBlendOp					= VK_BLEND_OP_ADD;
			AttachmentBlendingRules[i].srcAlphaBlendFactor			= VK_BLEND_FACTOR_ONE;
			AttachmentBlendingRules[i].dstAlphaBlendFactor			= VK_BLEND_FACTOR_ZERO;
			AttachmentBlendingRules[i].alphaBlendOp					= VK_BLEND_OP_ADD;
			AttachmentBlendingRules[i].colorWriteMask				= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		}

		this->Viewport.viewportCount = this->DefaultViewport.size();
		this->Viewport.pViewports = this->DefaultViewport.data();
		this->Viewport.scissorCount = this->DefaultScissor.size();
		this->Viewport.pScissors = this->DefaultScissor.data();
		this->ColorBlend.attachmentCount = this->AttachmentBlendingRules.size();
		this->ColorBlend.pAttachments = this->AttachmentBlendingRules.data();

		// Generate SPIRV code.
		if (Success) {
			glslang::SpvOptions Option;
			spv::SpvBuildLogger Logger;
			this->ByteCode = std::vector<std::vector<uint>>(this->Shader.size());
			for (size_t i = 0; i < this->Shader.size(); i++) {
				glslang::GlslangToSpv(*this->Program->getIntermediate(this->Shader[i]->Handle->getStage()), this->ByteCode[i], &Logger, &Option);
			}
		}
		else {
			// failed to gather meta data.
			Success = false;
		}
	}

	void pipeline::rasterizer::bind(uint32_t aBindingIndex, size_t aVertexStride, uint32_t aLocationIndex, size_t aVertexOffset, input_rate aInputRate) {
		// Check if binding already exists in set.
		bool ExistsInSet = false;
		for (const VkVertexInputBindingDescription& Buffer : this->VertexBufferBindingDescription) {
			ExistsInSet |= (aBindingIndex == Buffer.binding);
		}

		if (!ExistsInSet) {
			// Does not exist in set, add it.
			VkVertexInputBindingDescription Buffer{};
			Buffer.binding		= aBindingIndex;
			Buffer.stride		= aVertexStride;
			Buffer.inputRate	= (VkVertexInputRate)aInputRate;
			this->VertexBufferBindingDescription.push_back(Buffer);
		}
		else {
			// Exists in set, overwrite it.
			for (VkVertexInputBindingDescription& Buffer : this->VertexBufferBindingDescription) {
				if (aBindingIndex == Buffer.binding) {
					Buffer.binding		= aBindingIndex;
					Buffer.stride		= aVertexStride;
					Buffer.inputRate	= (VkVertexInputRate)aInputRate;
				}
			}
		}

		for (attribute& Attribute : this->VertexAttribute) {
			if (aLocationIndex == Attribute.Description.location) {
				Attribute.Description.location			= aLocationIndex;
				Attribute.Description.binding			= aBindingIndex;
				//Attribute.Description.format		; // Gets set in Meta Data Section.
				Attribute.Description.offset			= aVertexOffset;
				break;
			}
		}
	}

	void pipeline::rasterizer::attach(uint32_t aAttachmentIndex, std::shared_ptr<image> aAttachmentImage, image::layout aImageLayout) {
		// TODO: Maybe check that image sample count matches rasterizer sample count?
		this->attach(aAttachmentIndex, (image::format)aAttachmentImage->CreateInfo.format, aImageLayout);
	}

	void pipeline::rasterizer::attach(uint32_t aAttachmentIndex, image::format aFormat, image::layout aImageLayout) {
		if (aAttachmentIndex < this->ColorAttachment.size()) {
			this->ColorAttachment[aAttachmentIndex].Description.format			= (VkFormat)aFormat;
			this->ColorAttachment[aAttachmentIndex].Description.samples			= (VkSampleCountFlagBits)this->Multisample.rasterizationSamples;
			this->ColorAttachment[aAttachmentIndex].Description.initialLayout	= (VkImageLayout)aImageLayout;
			this->ColorAttachment[aAttachmentIndex].Description.finalLayout		= (VkImageLayout)aImageLayout;
		}
		else if (aAttachmentIndex == this->ColorAttachment.size()) {
			this->DepthStencilAttachment.Description.format						= (VkFormat)aFormat;
			this->DepthStencilAttachment.Description.samples					= (VkSampleCountFlagBits)this->Multisample.rasterizationSamples;
			this->DepthStencilAttachment.Description.initialLayout				= (VkImageLayout)aImageLayout;
			this->DepthStencilAttachment.Description.finalLayout				= (VkImageLayout)aImageLayout;
		}
	}

	void pipeline::rasterizer::resize(math::vec<uint, 3> aResolution) {
		this->Resolution = aResolution;
		this->DefaultScissor[0] = { { 0, 0 }, { aResolution[0], aResolution[1] } };
		this->DefaultViewport[0] = { 0.0f, 0.0f, (float)aResolution[0], (float)aResolution[1], 0.0f, 1.0f };
	}

	pipeline::raytracer::raytracer() {
		this->MaxRecursionDepth = 0;
	}

	pipeline::raytracer::raytracer(std::vector<shader_group> aShaderGroup, uint32_t aMaxRecursionDepth) : raytracer() {
		bool Success = true;
		this->ShaderGroup = aShaderGroup;
		this->MaxRecursionDepth = aMaxRecursionDepth;
		
		// Now convert shader groups from pointers into std::vector<VkRayTracingShaderGroupCreateInfoKHR> & std::vector<std::shared_ptr<shader>>.
		{
			std::set<std::shared_ptr<shader>> LinearizedShaderSet;
			for (size_t i = 0; i < this->ShaderGroup.size(); i++) {
				std::vector<std::shared_ptr<shader>> ShaderList = { this->ShaderGroup[i].GeneralShader, this->ShaderGroup[i].ClosestHitShader, this->ShaderGroup[i].AnyHitShader, this->ShaderGroup[i].IntersectionShader };
				// Check if any of the shaders in ShaderList exist in this->Shader.
				for (size_t j = 0; j < ShaderList.size(); j++) {
					if (ShaderList[j] != nullptr) {
						LinearizedShaderSet.insert(ShaderList[j]);
					}
				}
			}
	
			// Convert into an std::vector<std::shared_ptr<shader>>.
			this->Shader = std::vector<std::shared_ptr<shader>>(LinearizedShaderSet.begin(), LinearizedShaderSet.end());
		}

		// Compile shaders.
		// Link Shader Stages.
		if (Success) {
			EShMessages Message = (EShMessages)(
				EShMessages::EShMsgAST |
				EShMessages::EShMsgSpvRules |
				EShMessages::EShMsgVulkanRules |
				EShMessages::EShMsgDebugInfo |
				EShMessages::EShMsgBuiltinSymbolTable
			);

			// Link various shader stages together.
			this->Program = std::make_shared<glslang::TProgram>();
			for (std::shared_ptr<shader> Shd : Shader) {
				this->Program->addShader(Shd->Handle.get());
			}

			// Link Shader Stages
			Success = this->Program->link(Message);

			// Check if Link was successful
			if (!Success) {
				std::cout << this->Program->getInfoLog() << std::endl;
			}
		}

		// TODO: Build and acquire reflection variables.
		if (Success) {
			this->Program->buildReflection(EShReflectionAllIOVariables);

			// Generates Descriptor Set Layout Bindings.
			this->generate_descriptor_set_layout_binding();
		
			// Debug output for ray tracing pipeline reflection
			std::cout << "// -------------------- Ray Tracing Pipeline Reflection Start -------------------- \\\\" << std::endl << std::endl;
		
			// Print Uniforms
			std::cout << "----- Uniform Objects -----" << std::endl;
			for (std::pair<std::pair<int, int>, util::variable> Variable : this->DescriptorSetVariable) {
				std::cout << "layout (set = " << Variable.first.first << ", binding = " << Variable.first.second << ") uniform " << Variable.second;
			}
			std::cout << std::endl;
		
			std::cout << "\\\\ -------------------- Ray Tracing Pipeline Reflection End -------------------- //" << std::endl;
		}

		// Generate SPIRV code.
		if (Success) {
			glslang::SpvOptions Option;
			spv::SpvBuildLogger Logger;
			this->ByteCode = std::vector<std::vector<uint>>(this->Shader.size());
			for (size_t i = 0; i < this->Shader.size(); i++) {
				glslang::GlslangToSpv(*this->Program->getIntermediate(this->Shader[i]->Handle->getStage()), this->ByteCode[i], &Logger, &Option);
			}
		}
	}

	pipeline::compute::compute() {}

	pipeline::pipeline() {
		// Public data.
		this->BindPoint					= VK_PIPELINE_BIND_POINT_MAX_ENUM;
		this->Layout					= VK_NULL_HANDLE;
		this->Cache						= VK_NULL_HANDLE;
		this->Handle					= VK_NULL_HANDLE;

		// Pipline Specific Construction Data.
		this->Context					= nullptr;
	}

	pipeline::pipeline(std::shared_ptr<context> aContext, std::shared_ptr<rasterizer> aRasterizer, VkRenderPass aRenderPass, uint32_t aSubpassIndex) : pipeline() {
		VkResult Result = VK_SUCCESS;

		this->BindPoint							= VK_PIPELINE_BIND_POINT_GRAPHICS;
		this->Context							= aContext;
		this->CreateInfo 						= aRasterizer;

		// Create Render Pass.
		{
			// Attachments
			std::vector<VkAttachmentDescription> AttachmentDescription(aRasterizer->ColorAttachment.size());

			// References
			std::vector<VkAttachmentReference> ColorAttachmentReference(aRasterizer->ColorAttachment.size());
			VkAttachmentReference DepthAttachmentReference{};
			VkAttachmentReference StencilAttachmentReference{};

			// Load all Color Attachments.
			for (size_t i = 0; i < aRasterizer->ColorAttachment.size(); i++) {
				AttachmentDescription[i] = aRasterizer->ColorAttachment[i].Description;
			}
			
			// If depth stencil format is not undefined, add to attachment description.
			if (aRasterizer->DepthStencilAttachment.Description.format != VK_FORMAT_UNDEFINED) {
				DepthAttachmentReference = { (uint32_t)AttachmentDescription.size(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
				AttachmentDescription.push_back(aRasterizer->DepthStencilAttachment.Description);
			}

			// Convert to references. 
			for (size_t i = 0; i < aRasterizer->ColorAttachment.size(); i++) {
				ColorAttachmentReference[i].attachment	= i;
				ColorAttachmentReference[i].layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			// Subpass Description
			std::vector<VkSubpassDescription> SubpassDescription(1);
			SubpassDescription[0].flags							= 0;
			SubpassDescription[0].pipelineBindPoint				= VK_PIPELINE_BIND_POINT_GRAPHICS;
			SubpassDescription[0].inputAttachmentCount			= 0;
			SubpassDescription[0].pInputAttachments				= NULL;
			SubpassDescription[0].colorAttachmentCount			= ColorAttachmentReference.size();
			SubpassDescription[0].pColorAttachments				= ColorAttachmentReference.data();
			SubpassDescription[0].pResolveAttachments			= NULL;
			if (aRasterizer->DepthStencilAttachment.Description.format != VK_FORMAT_UNDEFINED) {
				SubpassDescription[0].pDepthStencilAttachment		= &DepthAttachmentReference;
			}
			else {
				SubpassDescription[0].pDepthStencilAttachment		= NULL;
			}

			// Subpass Dependency
			std::vector<VkSubpassDependency> SubpassDependency(1);
			SubpassDependency[0].srcSubpass						= VK_SUBPASS_EXTERNAL;
			SubpassDependency[0].dstSubpass						= 0;
			SubpassDependency[0].srcStageMask					= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			SubpassDependency[0].dstStageMask					= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			SubpassDependency[0].srcAccessMask					= 0;
			SubpassDependency[0].dstAccessMask					= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			SubpassDependency[0].dependencyFlags				= 0;
			
			VkRenderPassCreateInfo RPCI{};
			RPCI.sType											= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			RPCI.pNext											= NULL;
			RPCI.flags											= 0;
			RPCI.attachmentCount								= AttachmentDescription.size();
			RPCI.pAttachments									= AttachmentDescription.data();
			RPCI.subpassCount									= SubpassDescription.size();
			RPCI.pSubpasses										= SubpassDescription.data();
			RPCI.dependencyCount								= SubpassDependency.size();
			RPCI.pDependencies									= SubpassDependency.data();

			Result = vkCreateRenderPass(aContext->Handle, &RPCI, NULL, &this->RenderPass);
		}

		// Create Respective Shader Modules.
		Result = this->shader_stage_create(aRasterizer);

		// Generate Descriptor Set Layouts from Meta Data gathered from shaders.
		Result = this->create_pipeline_layout(aRasterizer->DescriptorSetLayoutBinding);

		// Create Pipeline
		if (Result == VK_SUCCESS) {
			bool TesselationControlShaderExists = false;
			bool TesselationEvaluationShaderExists = false;
			for (std::shared_ptr<shader> Shdr : aRasterizer->Shader) {
				if (Shdr->Stage == shader::stage::TESSELLATION_CONTROL) {
					TesselationControlShaderExists = true;
				}
				if (Shdr->Stage == shader::stage::TESSELLATION_CONTROL) {
					TesselationEvaluationShaderExists = true;
				}
			}

			// Load attributes from aRasterizer->
			std::vector<VkVertexInputAttributeDescription> VertexAttributeDescription;
			for (rasterizer::attribute& Attribute : aRasterizer->VertexAttribute) {
				VertexAttributeDescription.push_back(Attribute.Description);
			}

			// Loading Vertex Binding & Attribute Data
			VkPipelineVertexInputStateCreateInfo Input{};
			Input.sType													= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			Input.pNext													= NULL;
			Input.flags													= 0;
			Input.vertexBindingDescriptionCount							= aRasterizer->VertexBufferBindingDescription.size();
			Input.pVertexBindingDescriptions							= aRasterizer->VertexBufferBindingDescription.data();
			Input.vertexAttributeDescriptionCount						= VertexAttributeDescription.size();
			Input.pVertexAttributeDescriptions							= VertexAttributeDescription.data();

			// Create Rasterizer Create Info Struct.
			VkGraphicsPipelineCreateInfo RasterizerCreateInfo {};
			RasterizerCreateInfo.sType							= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			RasterizerCreateInfo.pNext							= NULL;// &RenderingCreateInfo;
			RasterizerCreateInfo.flags							= 0;
			RasterizerCreateInfo.stageCount						= this->Stage.size();
			RasterizerCreateInfo.pStages						= this->Stage.data();
			RasterizerCreateInfo.pVertexInputState				= &Input;
			RasterizerCreateInfo.pInputAssemblyState			= &aRasterizer->InputAssembly;
			if (TesselationControlShaderExists && TesselationEvaluationShaderExists) {
				RasterizerCreateInfo.pTessellationState				= &aRasterizer->Tesselation;
			}
			else {
				RasterizerCreateInfo.pTessellationState				= NULL;
			}
			RasterizerCreateInfo.pViewportState					= &aRasterizer->Viewport;
			RasterizerCreateInfo.pRasterizationState			= &aRasterizer->Rasterizer;
			RasterizerCreateInfo.pMultisampleState				= &aRasterizer->Multisample;
			if (aRasterizer->DepthStencilAttachment.Description.format != VK_FORMAT_UNDEFINED) {
				RasterizerCreateInfo.pDepthStencilState				= &aRasterizer->DepthStencil;
			}
			else {
				RasterizerCreateInfo.pDepthStencilState				= NULL;
			}
			RasterizerCreateInfo.pColorBlendState					= &aRasterizer->ColorBlend;
			if (false) {
				RasterizerCreateInfo.pDynamicState					= &aRasterizer->DynamicState;
			}
			else {
				RasterizerCreateInfo.pDynamicState					= NULL;
			}
			RasterizerCreateInfo.layout							= this->Layout;
			RasterizerCreateInfo.renderPass						= this->RenderPass;
			RasterizerCreateInfo.subpass						= 0;
			RasterizerCreateInfo.basePipelineHandle				= VK_NULL_HANDLE;
			RasterizerCreateInfo.basePipelineIndex				= 0;
			
			// Create Rasterization Pipeline.
			Result = vkCreateGraphicsPipelines(this->Context->Handle, this->Cache, 1, &RasterizerCreateInfo, NULL, &this->Handle);
		}
	}

	pipeline::pipeline(std::shared_ptr<context> aContext, std::shared_ptr<raytracer> aRaytracer) : pipeline() {
		VkResult Result = VK_SUCCESS;

		this->BindPoint				= VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
		this->Context				= aContext;
		this->CreateInfo			= aRaytracer;

		// Generate GPU shader modules.
		Result = this->shader_stage_create(aRaytracer);

		// Generate Shader Group information.
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> RSGCI(aRaytracer->ShaderGroup.size());
		for (size_t i = 0; i < aRaytracer->ShaderGroup.size(); i++) {
			RSGCI[i].sType									= VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			RSGCI[i].pNext									= NULL;
			// Use inference to determine shader group type.
			if (aRaytracer->ShaderGroup[i].IntersectionShader != nullptr) {
				RSGCI[i].type								= VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
			}
			else if ((aRaytracer->ShaderGroup[i].AnyHitShader != nullptr) || (aRaytracer->ShaderGroup[i].ClosestHitShader != nullptr)) {
				RSGCI[i].type 								= VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			}
			else {
				RSGCI[i].type								= VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			}
			// TODO: Clean up this code later for index searching template code.
			{
				// find general shader location index.
				int Index = -1;
				{
					auto it = std::find(aRaytracer->Shader.begin(), aRaytracer->Shader.end(), aRaytracer->ShaderGroup[i].GeneralShader);
					if (it != aRaytracer->Shader.end()) {
						// Search for index of where this shader exists in aRaytracer->Shader.
						Index = (int)(it - aRaytracer->Shader.begin());
					}
				}
				if (Index >= 0) {
					RSGCI[i].generalShader							= Index;
				}
				else {
					RSGCI[i].generalShader							= VK_SHADER_UNUSED_KHR;
				}
			}
			{
				// find closest hit shader location index.
				int Index = -1;
				{
					auto it = std::find(aRaytracer->Shader.begin(), aRaytracer->Shader.end(), aRaytracer->ShaderGroup[i].ClosestHitShader);
					if (it != aRaytracer->Shader.end()) {
						// Search for index of where this shader exists in aRaytracer->Shader.
						Index = (int)(it - aRaytracer->Shader.begin());
					}
				}
				if (Index >= 0) {
					RSGCI[i].closestHitShader						= Index;
				}
				else {
					RSGCI[i].closestHitShader						= VK_SHADER_UNUSED_KHR;
				}
			}
			{
				// find any hit shader location index.
				int Index = -1;
				{
					auto it = std::find(aRaytracer->Shader.begin(), aRaytracer->Shader.end(), aRaytracer->ShaderGroup[i].AnyHitShader);
					if (it != aRaytracer->Shader.end()) {
						// Search for index of where this shader exists in aRaytracer->Shader.
						Index = (int)(it - aRaytracer->Shader.begin());
					}
				}
				if (Index >= 0) {
					RSGCI[i].anyHitShader							= Index;
				}
				else {
					RSGCI[i].anyHitShader							= VK_SHADER_UNUSED_KHR;
				}
			}
			{
				// Intersection Shader.
				int Index = -1;
				{
					auto it = std::find(aRaytracer->Shader.begin(), aRaytracer->Shader.end(), aRaytracer->ShaderGroup[i].IntersectionShader);
					if (it != aRaytracer->Shader.end()) {
						// Search for index of where this shader exists in aRaytracer->Shader.
						Index = (int)(it - aRaytracer->Shader.begin());
					}
				}
				if (Index >= 0) {
					RSGCI[i].intersectionShader						= Index;
				}
				else {
					RSGCI[i].intersectionShader						= VK_SHADER_UNUSED_KHR;
				}
			}			
		}

		// Create Pipeline Layout.
		Result = this->create_pipeline_layout(aRaytracer->DescriptorSetLayoutBinding);

		// Create Actual Ray Tracing Pipeline.
		if (Result == VK_SUCCESS) {
			VkRayTracingPipelineCreateInfoKHR RTPCI{};
			RTPCI.sType									= VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
			RTPCI.pNext									= NULL;
			RTPCI.flags									= 0;
			RTPCI.stageCount							= this->Stage.size();
			RTPCI.pStages								= this->Stage.data();
			RTPCI.groupCount							= RSGCI.size();
			RTPCI.pGroups								= RSGCI.data();
			RTPCI.maxPipelineRayRecursionDepth			= aRaytracer->MaxRecursionDepth;
			RTPCI.pLibraryInfo							= NULL;
			RTPCI.pLibraryInterface						= NULL;
			RTPCI.pDynamicState							= NULL;
			RTPCI.layout								= this->Layout;
			RTPCI.basePipelineHandle					= VK_NULL_HANDLE;
			RTPCI.basePipelineIndex						= 0;
			
			// Requires loading function.
			// TODO: Deferred Host Operations?
			PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)aContext->FunctionPointer["vkCreateRayTracingPipelinesKHR"];
			Result = vkCreateRayTracingPipelinesKHR(aContext->Handle, VK_NULL_HANDLE, this->Cache, 1, &RTPCI, NULL, &this->Handle);
		}

		// Create Shader Binding Table.
		if (Result == VK_SUCCESS) {
			VkPhysicalDeviceRayTracingPipelinePropertiesKHR PDRTPP{};
			PDRTPP.sType								= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
			PDRTPP.pNext								= NULL;

			VkPhysicalDeviceProperties2 PDP2{};
			PDP2.sType									= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			PDP2.pNext									= &PDRTPP;

			// Get Physical Device Properties.
			vkGetPhysicalDeviceProperties2(aContext->Device->Handle, &PDP2);

			// Get Shader Group Handle Size.
			uint32_t HandleSize						= PDRTPP.shaderGroupHandleSize;
			uint32_t HandleAlignment				= PDRTPP.shaderGroupHandleAlignment;
			uint32_t BaseAlignment 					= PDRTPP.shaderGroupBaseAlignment;
			uint32_t GroupCount						= (uint32_t)RSGCI.size();

			// Get Shader Group Handles.
			std::vector<uint8_t> ShaderGroupHandle(GroupCount * HandleSize);
			PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = (PFN_vkGetRayTracingShaderGroupHandlesKHR)aContext->FunctionPointer["vkGetRayTracingShaderGroupHandlesKHR"];
			Result = vkGetRayTracingShaderGroupHandlesKHR(aContext->Handle, this->Handle, 0, GroupCount, HandleSize * GroupCount, ShaderGroupHandle.data());

			// Get Shader Region Counts
			uint32_t RayGenCount = 0, MissCount = 0, HitCount = 0, CallableCount = 0;
			for (size_t i = 0; i < aRaytracer->Shader.size(); i++) {
				switch (aRaytracer->Shader[i]->Stage) {
				case shader::stage::RAYGEN:
					RayGenCount++;
					break;
				case shader::stage::MISS:
					MissCount++;
					break;
				case shader::stage::CLOSEST_HIT:
				case shader::stage::ANY_HIT:
				case shader::stage::INTERSECTION:
					HitCount++;
					break;
				case shader::stage::CALLABLE:
					CallableCount++;
					break;
				default:
					break;
				}
			}
			
			// TODO: Maybe generalize to alignment function general?
			auto align_up = [](uint32_t aValue, uint32_t aAlignment) -> uint32_t {
				return (aValue + aAlignment - 1) & ~(aAlignment - 1);
			};
			
			// Calculate Region Sizes based on alignment.
			uint32_t HandleSizeAligned 		= align_up(HandleSize, HandleAlignment);
			uint32_t RayGenSize 			= align_up(RayGenCount * HandleSizeAligned, BaseAlignment);
			uint32_t MissSize 				= align_up(MissCount * HandleSizeAligned, BaseAlignment);
			uint32_t HitSize 				= align_up(HitCount * HandleSizeAligned, BaseAlignment);
			uint32_t CallableSize 			= align_up(CallableCount * HandleSizeAligned, BaseAlignment);
			uint32_t TotalSize = RayGenSize + MissSize + HitSize + CallableSize;

			// Calculate offsets into SBTHM.
			uint32_t RayGenOffset = 0;
			uint32_t MissOffset = RayGenSize;
			uint32_t HitOffset = MissOffset + MissSize;
			uint32_t CallableOffset = HitOffset + HitSize;

			// Get Index Map.
			std::vector<uint32_t> RayGenIndices;
			std::vector<uint32_t> MissIndices;
			std::vector<uint32_t> HitIndices;
			std::vector<uint32_t> CallableIndices;
			for (size_t i = 0; i < aRaytracer->Shader.size(); i++) {
				switch(aRaytracer->Shader[i]->Stage) {
				case shader::stage::RAYGEN:
					RayGenIndices.push_back(i);
					break;
				case shader::stage::MISS:
					MissIndices.push_back(i);
					break;
				case shader::stage::CLOSEST_HIT:
				case shader::stage::ANY_HIT:
				case shader::stage::INTERSECTION:
					HitIndices.push_back(i);
					break;
				case shader::stage::CALLABLE:
					CallableIndices.push_back(i);
					break;
				default:
					break;
				}
			}
			
			// Load Host memory object with handles.
			std::vector<uint8_t> SBTHM(TotalSize);
			for (size_t i = 0; i < RayGenIndices.size(); i++) {
				memcpy(
					SBTHM.data() + RayGenOffset + i*HandleSizeAligned,
					ShaderGroupHandle.data() + RayGenIndices[i]*HandleSize, 
					HandleSize
				);
			}
			for (size_t i = 0; i < MissIndices.size(); i++) {
				memcpy(
					SBTHM.data() + MissOffset + i*HandleSizeAligned,
					ShaderGroupHandle.data() + MissIndices[i]*HandleSize, 
					HandleSize
				);
			}
			for (size_t i = 0; i < HitIndices.size(); i++) {
				memcpy(
					SBTHM.data() + HitOffset + i*HandleSizeAligned,
					ShaderGroupHandle.data() + HitIndices[i]*HandleSize, 
					HandleSize
				);
			}
			for (size_t i = 0; i < CallableIndices.size(); i++) {
				memcpy(
					SBTHM.data() + CallableOffset + i*HandleSizeAligned,
					ShaderGroupHandle.data() + CallableIndices[i]*HandleSize, 
					HandleSize
				);
			}
			
			// Create Shader Binding Table.
			buffer::create_info SBTCI{};
			SBTCI.Usage = buffer::usage::SHADER_BINDING_TABLE_KHR | buffer::usage::SHADER_DEVICE_ADDRESS_KHR | buffer::usage::TRANSFER_DST | buffer::usage::TRANSFER_SRC;
			SBTCI.Memory = device::memory::DEVICE_LOCAL;

			// Create Actual Buffer.
			this->ShaderBindingTable.Buffer = aContext->create_buffer(SBTCI, TotalSize, SBTHM.data());

			// Get addresses for each region for vkCmdTraceRaysKHR().
			VkDeviceAddress SBTBaseAddress = this->ShaderBindingTable.Buffer->device_address();

			this->ShaderBindingTable.Raygen.deviceAddress 		= SBTBaseAddress + RayGenOffset;
			this->ShaderBindingTable.Raygen.stride 				= HandleSizeAligned;
			this->ShaderBindingTable.Raygen.size 				= RayGenSize;

			this->ShaderBindingTable.Miss.deviceAddress 		= SBTBaseAddress + MissOffset;
			this->ShaderBindingTable.Miss.stride 				= HandleSizeAligned;
			this->ShaderBindingTable.Miss.size 					= MissSize;

			this->ShaderBindingTable.Hit.deviceAddress 			= SBTBaseAddress + HitOffset;
			this->ShaderBindingTable.Hit.stride 				= HandleSizeAligned;
			this->ShaderBindingTable.Hit.size 					= HitSize;

			this->ShaderBindingTable.Callable.deviceAddress 	= SBTBaseAddress + CallableOffset;
			this->ShaderBindingTable.Callable.stride 			= HandleSizeAligned;
			this->ShaderBindingTable.Callable.size 				= CallableSize;
		}
	}

	pipeline::pipeline(std::shared_ptr<context> aContext, std::shared_ptr<compute> aCompute) : pipeline() {
		VkResult Result = VK_SUCCESS;

		BindPoint	= VK_PIPELINE_BIND_POINT_COMPUTE;
		Context		= aContext;

		//Result = vkCreateComputePipelines(Context->handle(), Cache, 1, &Compute.CreateInfo, NULL, &Handle);
	}

	pipeline::~pipeline() {
		// Delete all vulkan allocated resources.
		if (this->Handle != VK_NULL_HANDLE) {
			vkDestroyPipeline(this->Context->Handle, this->Handle, NULL);
		}
		if (this->Cache != VK_NULL_HANDLE) {
			vkDestroyPipelineCache(this->Context->Handle, this->Cache, NULL);
		}
		if (this->Layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(this->Context->Handle, this->Layout, NULL);
		}
		if (this->DescriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(this->Context->Handle, this->DescriptorPool, NULL);
		}
		if (this->DescriptorSetLayout.size() != 0) {
			for (size_t i = 0; i < this->DescriptorSetLayout.size(); i++) {
				vkDestroyDescriptorSetLayout(this->Context->Handle, this->DescriptorSetLayout[i], NULL);
			}
		}
		if (this->Stage.size() != 0) {
			for (size_t i = 0; i < this->Stage.size(); i++) {
				vkDestroyShaderModule(this->Context->Handle, this->Stage[i].module, NULL);
			}
		}
		if (this->RenderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(this->Context->Handle, this->RenderPass, NULL);
		}		
	}

	void pipeline::barrier(
		VkCommandBuffer aCommandBuffer,
		uint aSrcStage, uint aDstStage,
		uint aSrcAccess, uint aDstAccess
	) {
		VkMemoryBarrier MemoryBarrier{};
		MemoryBarrier.sType				= VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		MemoryBarrier.pNext				= NULL;
		MemoryBarrier.srcAccessMask		= aSrcAccess;
		MemoryBarrier.dstAccessMask		= aDstAccess;
		std::vector<VkMemoryBarrier> MemoryBarrierVector = { MemoryBarrier };
		pipeline::barrier(aCommandBuffer, aSrcStage, aDstStage, MemoryBarrierVector);
	}

	void pipeline::barrier(
		VkCommandBuffer aCommandBuffer, 
		uint aSrcStage, uint aDstStage, 
		const std::vector<VkMemoryBarrier>& aMemoryBarrier, 
		const std::vector<VkBufferMemoryBarrier>& aBufferBarrier, 
		const std::vector<VkImageMemoryBarrier>& aImageBarrier
	) {
		vkCmdPipelineBarrier(
			aCommandBuffer, 
			(VkPipelineStageFlags)aSrcStage, (VkPipelineStageFlags)aDstStage, 
			0, 
			aMemoryBarrier.size(), aMemoryBarrier.data(), 
			aBufferBarrier.size(), aBufferBarrier.data(), 
			aImageBarrier.size(), aImageBarrier.data()
		);
	}

	void pipeline::begin(VkCommandBuffer aCommandBuffer, std::shared_ptr<framebuffer> aFramebuffer, VkRect2D aRenderArea, VkSubpassContents aSubpassContents) {
		VkRenderPassBeginInfo RPBI{};
		RPBI.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		RPBI.pNext				= NULL;
		RPBI.renderPass			= this->RenderPass;
		RPBI.framebuffer		= aFramebuffer->Handle;
		RPBI.renderArea			= aRenderArea;
		RPBI.clearValueCount	= aFramebuffer->ClearValue.size();
		RPBI.pClearValues		= aFramebuffer->ClearValue.data();
		vkCmdBeginRenderPass(aCommandBuffer, &RPBI, aSubpassContents);
	}

	void pipeline::bind(
			VkCommandBuffer 						aCommandBuffer, 
			std::vector<std::shared_ptr<buffer>> 	aVertexBuffer, 
			std::shared_ptr<buffer> 				aIndexBuffer, 
			std::shared_ptr<descriptor::array> 		aDescriptorArray
	) {
		vkCmdBindPipeline(aCommandBuffer, this->BindPoint, this->Handle);
		if ((aDescriptorArray != nullptr) ? (aDescriptorArray->DescriptorSet.size() > 0) : false) {
			vkCmdBindDescriptorSets(aCommandBuffer, this->BindPoint, this->Layout, 0, aDescriptorArray->DescriptorSet.size(), aDescriptorArray->DescriptorSet.data(), 0, NULL);
		}
		if (aVertexBuffer.size() > 0) {
			std::vector<VkBuffer> VertexBuffer(aVertexBuffer.size());
			std::vector<VkDeviceSize> VertexBufferOffset(aVertexBuffer.size(), 0);
			for (size_t i = 0; i < aVertexBuffer.size(); i++) {
				VertexBuffer[i] = aVertexBuffer[i]->Handle;
			}
			vkCmdBindVertexBuffers(aCommandBuffer, 0, VertexBuffer.size(), VertexBuffer.data(), VertexBufferOffset.data());
		}
		if (aIndexBuffer != nullptr) {
			VkIndexType IndexType = (aVertexBuffer[0]->ElementCount <= (1 << 16)) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
			vkCmdBindIndexBuffer(aCommandBuffer, aIndexBuffer->Handle, 0, IndexType);
		}
	}

	void pipeline::end(VkCommandBuffer aCommandBuffer) {
		vkCmdEndRenderPass(aCommandBuffer);
	}

	void pipeline::draw(
		VkCommandBuffer 											aCommandBuffer,
		std::shared_ptr<framebuffer> 								aFramebuffer,
		std::vector<std::shared_ptr<buffer>> 						aVertexBuffer,
		std::shared_ptr<buffer> 									aIndexBuffer,
		std::shared_ptr<descriptor::array> 							aDescriptorArray
	) {
		std::shared_ptr<rasterizer> Rasterizer = std::dynamic_pointer_cast<rasterizer>(this->CreateInfo);
		VkRect2D RenderArea = { { 0, 0 }, { Rasterizer->Resolution[0], Rasterizer->Resolution[1] } };
		this->begin(aCommandBuffer, aFramebuffer, RenderArea);
		this->bind(aCommandBuffer, aVertexBuffer, aIndexBuffer, aDescriptorArray);
		if (aIndexBuffer != nullptr) {
			vkCmdDrawIndexed(aCommandBuffer, aIndexBuffer->ElementCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(aCommandBuffer, aVertexBuffer[0]->ElementCount, 1, 0, 0);
		}
		this->end(aCommandBuffer);
	}

	VkResult pipeline::draw(
		std::shared_ptr<framebuffer> 								aFramebuffer,
		std::vector<std::shared_ptr<buffer>> 						aVertexBuffer,
		std::shared_ptr<buffer> 									aIndexBuffer,
		std::shared_ptr<descriptor::array> 							aDescriptorArray
	) {
		VkResult Result = VK_SUCCESS;

		device::operation DeviceOperation = device::operation::GRAPHICS_AND_COMPUTE;

		VkCommandBuffer DrawCommand = this->Context->allocate_command_buffer(DeviceOperation);

		Result = this->Context->begin(DrawCommand);
		this->draw(DrawCommand, aFramebuffer, aVertexBuffer, aIndexBuffer, aDescriptorArray);
		Result = this->Context->end(DrawCommand);

		Result = this->Context->execute_and_wait(DeviceOperation, DrawCommand);

		return Result;
	}

	VkResult pipeline::draw(
		std::vector<std::shared_ptr<image>> 						aImage,
		std::vector<std::shared_ptr<buffer>> 						aVertexBuffer,
		std::shared_ptr<buffer> 									aIndexBuffer,
		std::map<std::pair<int, int>, std::shared_ptr<buffer>> 		aUniformBuffer,
		std::map<std::pair<int, int>, std::shared_ptr<image>> 		aSamplerImage
	) {
		// Error code tracking.
		VkResult Result = VK_SUCCESS;

		// Operation which the GPU will execute
		device::operation DeviceOperation = device::operation::GRAPHICS_AND_COMPUTE;

		// TODO: Get Rasterizer resolution, change later.
		std::shared_ptr<rasterizer> Rasterizer = std::dynamic_pointer_cast<rasterizer>(this->CreateInfo);

		// Allocated GPU Resources needed to execute.
		VkCommandBuffer CommandBuffer = this->Context->allocate_command_buffer(DeviceOperation);
		std::shared_ptr<framebuffer> Framebuffer = Context->create_framebuffer(this->shared_from_this(), aImage, Rasterizer->Resolution);
		std::shared_ptr<descriptor::array> DescriptorArray = Context->create_descriptor_array(this->shared_from_this());

		// Bind uniform buffers.
		for (auto& [SetBinding, Buffer] : aUniformBuffer) {
			DescriptorArray->bind(SetBinding.first, SetBinding.second, 0, Buffer);
		}

		// Bind sampler images.
		for (auto& [SetBinding, Image] : aSamplerImage) {
			DescriptorArray->bind(SetBinding.first, SetBinding.second, 0, Image, image::layout::SHADER_READ_ONLY_OPTIMAL);
		}

		// Write Command Buffer here.
		Result = this->Context->begin(CommandBuffer);
		this->draw(CommandBuffer, Framebuffer, aVertexBuffer, aIndexBuffer, DescriptorArray);
		Result = this->Context->end(CommandBuffer);

		// Execute Command Buffer here.
		Result = this->Context->execute_and_wait(DeviceOperation, CommandBuffer);

		// Release Command buffer and other resources.
		this->Context->release_command_buffer(DeviceOperation, CommandBuffer);

		return Result;
	}

	void pipeline::raytrace(
		VkCommandBuffer 											aCommandBuffer,
		std::shared_ptr<descriptor::array> 							aDescriptorArray,
		math::vec<uint, 3> 											aResolution
	) {
		PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)this->Context->FunctionPointer["vkCmdTraceRaysKHR"];
		this->bind(aCommandBuffer, {}, nullptr, aDescriptorArray);
		vkCmdTraceRaysKHR(
			aCommandBuffer, 
			&this->ShaderBindingTable.Raygen,
			&this->ShaderBindingTable.Miss,
			&this->ShaderBindingTable.Hit,
			&this->ShaderBindingTable.Callable,
			aResolution[0], 
			aResolution[1], 
			aResolution[2]
		);
	}

	VkResult pipeline::raytrace(
		std::shared_ptr<image> 										aOutputImage,
		std::shared_ptr<acceleration_structure> 					aTLAS,
		std::map<std::pair<int, int>, std::shared_ptr<buffer>> 		aUniformBuffer,
		std::map<std::pair<int, int>, std::shared_ptr<image>> 		aSamplerImage
	) {
		VkResult Result = VK_SUCCESS;
		// TODO: This is an immediate mode execution.
		return Result;
	}

	std::vector<VkDescriptorPoolSize> pipeline::descriptor_pool_sizes() const {
		std::map<VkDescriptorType, uint32_t> DescriptorTypeCount = this->descriptor_type_count();
		// Convert to pool size to vector data structure.
		std::vector<VkDescriptorPoolSize> PoolSize;
		for (auto& [Type, Count] : DescriptorTypeCount) {
			VkDescriptorPoolSize DPS{};
			DPS.type = Type;
			DPS.descriptorCount = Count;
			PoolSize.push_back(DPS);
		}

		return PoolSize;
	}

	std::map<VkDescriptorType, uint32_t> pipeline::descriptor_type_count() const {
		// Calculate pool sizes based on pipeline descriptor set layout binding info.
		std::map<VkDescriptorType, uint32_t> DescriptorTypeCount;
		for (size_t j = 0; j < this->CreateInfo->DescriptorSetLayoutBinding.size(); j++) {
			for (size_t k = 0; k < this->CreateInfo->DescriptorSetLayoutBinding[j].size(); k++) {
				VkDescriptorSetLayoutBinding DSLB = this->CreateInfo->DescriptorSetLayoutBinding[j][k];
				if (DescriptorTypeCount.count(DSLB.descriptorType) == 0) {
					DescriptorTypeCount[DSLB.descriptorType] = 0;
				}
				DescriptorTypeCount[DSLB.descriptorType] += DSLB.descriptorCount;
			}
		}
		return DescriptorTypeCount;
	}

	std::vector<std::vector<VkDescriptorSetLayoutBinding>> pipeline::descriptor_set_layout_binding() const {
		return this->CreateInfo->DescriptorSetLayoutBinding;
	}

	VkResult pipeline::shader_stage_create(std::shared_ptr<create_info> aCreateInfo) {
		VkResult Result = VK_SUCCESS;
		// Generate GPU Shader Modules.
		this->Stage = std::vector<VkPipelineShaderStageCreateInfo>(aCreateInfo->Shader.size());
		for (size_t i = 0; i < this->Stage.size(); i++) {
			// Generate Shader Module Info
			VkShaderModuleCreateInfo ShaderModuleCreateInfo{};
			ShaderModuleCreateInfo.sType				= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			ShaderModuleCreateInfo.pNext				= NULL;
			ShaderModuleCreateInfo.flags				= 0;
			ShaderModuleCreateInfo.codeSize				= aCreateInfo->ByteCode[i].size() * sizeof(uint);
			ShaderModuleCreateInfo.pCode				= aCreateInfo->ByteCode[i].data();
			// Load Shader Stage Meta Data
			this->Stage[i] 								= aCreateInfo->Shader[i]->pipeline_shader_stage_create_info();
			// Create Shader Module
			Result = vkCreateShaderModule(Context->Handle, &ShaderModuleCreateInfo, NULL, &this->Stage[i].module);
		}
		return Result;
	}

	VkResult pipeline::create_pipeline_layout(std::vector<std::vector<VkDescriptorSetLayoutBinding>> aDescriptorSetLayoutBinding) {
		VkResult Result = VK_SUCCESS;
		// Generate Descriptor Set Layouts from Meta Data gathered from shaders.
		if (Result == VK_SUCCESS) {
			this->DescriptorSetLayout = std::vector<VkDescriptorSetLayout>(aDescriptorSetLayoutBinding.size());
			for (size_t i = 0; i < aDescriptorSetLayoutBinding.size(); i++) {
				VkDescriptorSetLayoutCreateInfo CreateInfo{};
				CreateInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				CreateInfo.pNext			= NULL;
				CreateInfo.flags			= 0;
				if (aDescriptorSetLayoutBinding[i].size() > 0) {
					CreateInfo.bindingCount		= aDescriptorSetLayoutBinding[i].size();
					CreateInfo.pBindings		= aDescriptorSetLayoutBinding[i].data();
				}
				else {
					CreateInfo.bindingCount		= 0;
					CreateInfo.pBindings		= NULL;
				}
				// ! NOTE: We got our answer, we cannot use VK_NULL_HANDLE for empty descriptor set layouts. We have to 
				// ! construct them even if they are empty. The only exception is if extension VK_EXT_graphics_pipeline_library 
				// ! is enabled, then we can use VK_NULL_HANDLE to construct pipeline layouts and descriptor sets.
				Result = vkCreateDescriptorSetLayout(Context->Handle, &CreateInfo, NULL, &this->DescriptorSetLayout[i]);
			}
		}

		// Generate Descriptor Set Pool.
		if (Result == VK_SUCCESS) {
			std::vector<VkDescriptorPoolSize> DescriptorPoolSize = this->descriptor_pool_sizes();
			VkDescriptorPoolCreateInfo DPCI{};
			DPCI.sType 				= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			DPCI.pNext 				= NULL;
			DPCI.flags 				= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			DPCI.maxSets 			= aDescriptorSetLayoutBinding.size();
			DPCI.poolSizeCount 		= DescriptorPoolSize.size();
			DPCI.pPoolSizes 		= DescriptorPoolSize.data();
			Result = vkCreateDescriptorPool(Context->Handle, &DPCI, NULL, &this->DescriptorPool);
		}

		// Create Pipeline Layout.
		if (Result == VK_SUCCESS) {
			// Create Pipeline Layout.
			VkPipelineLayoutCreateInfo CreateInfo{};
			CreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			CreateInfo.pNext						= NULL;
			CreateInfo.flags						= 0;
			CreateInfo.setLayoutCount				= this->DescriptorSetLayout.size();
			CreateInfo.pSetLayouts					= this->DescriptorSetLayout.data();
			CreateInfo.pushConstantRangeCount		= 0;
			CreateInfo.pPushConstantRanges			= NULL;

			Result = vkCreatePipelineLayout(this->Context->Handle, &CreateInfo, NULL, &this->Layout);
		}

		return Result;
	}

}
