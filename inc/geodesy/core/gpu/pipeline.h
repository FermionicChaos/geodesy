#pragma once
#ifndef GEODESY_CORE_GPU_PIPELINE_H
#define GEODESY_CORE_GPU_PIPELINE_H

#include "../../config.h"
#include "../util/variable.h"

#include "config.h"
#include "device.h"
#include "buffer.h"
#include "image.h"
#include "shader.h"
#include "descriptor.h"
#include "framebuffer.h"

/*
Flow Chart of Pipeline Creation.
Start:		     Vertex       Pixel
			[    Source       Source
Shader		[	    |			|
			[      AST         AST
					  \       / 
			[          Program
Rasterizer	[		  /       \
			[     SPIRV       SPIRV
			        |           |
			[     Module      Module
Device		[         \        / 
			[		   Pipeline
*/

namespace geodesy::core::gpu {

	class framebuffer;

	class pipeline : public std::enable_shared_from_this<pipeline> {
	public:

		// The point of these structure is a straightforward api to map vulkan managed resources,
		// to slots indicated by the shaders provided.
		enum stage : uint {
			TOP_OF_PIPE 							= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			DRAW_INDIRECT 							= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
			VERTEX_INPUT 							= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
			VERTEX_SHADER 							= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
			TESSELLATION_CONTROL_SHADER 			= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
			TESSELLATION_EVALUATION_SHADER 			= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
			GEOMETRY_SHADER 						= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
			FRAGMENT_SHADER 						= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			EARLY_FRAGMENT_TESTS 					= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			LATE_FRAGMENT_TESTS 					= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			COLOR_ATTACHMENT_OUTPUT 				= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			COMPUTE_SHADER 							= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			TRANSFER 								= VK_PIPELINE_STAGE_TRANSFER_BIT,
			BOTTOM_OF_PIPE 							= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			HOST 									= VK_PIPELINE_STAGE_HOST_BIT,
			ALL_GRAPHICS 							= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			ALL_COMMANDS 							= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			NONE 									= VK_PIPELINE_STAGE_NONE,
			// TRANSFORM_FEEDBACK_EXT 					= VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,
			// CONDITIONAL_RENDERING_EXT 				= VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
			// ACCELERATION_STRUCTURE_BUILD_KHR 		= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			// RAY_TRACING_SHADER_KHR 					= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
			// FRAGMENT_DENSITY_PROCESS_EXT 			= VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
			// FRAGMENT_SHADING_RATE_ATTACHMENT_KHR 	= VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
			// COMMAND_PREPROCESS_NV 					= VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV,
			// TASK_SHADER_EXT 						= VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT,
			// MESH_SHADER_EXT 						= VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT,
			// SHADING_RATE_IMAGE_NV 					= VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV,
			// RAY_TRACING_SHADER_NV 					= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
			// ACCELERATION_STRUCTURE_BUILD_NV 		= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
			// TASK_SHADER_NV 							= VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
			// MESH_SHADER_NV 							= VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
			// NONE_KHR 								= VK_PIPELINE_STAGE_NONE_KHR,
		};

		enum type : int {
			UNKNOWN 							= -1,
			RASTERIZER							= VK_PIPELINE_BIND_POINT_GRAPHICS,
			COMPUTE 							= VK_PIPELINE_BIND_POINT_COMPUTE,
			RAY_TRACER 							= VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		};

		enum input_rate {
			VERTEX 								= VK_VERTEX_INPUT_RATE_VERTEX,
			INSTANCE 							= VK_VERTEX_INPUT_RATE_INSTANCE,
		};

		struct create_info {
		public:

			// I think this interface data exists in all pipelines types.
			// Uniform metatdata
			std::vector<std::vector<VkDescriptorSetLayoutBinding>>		DescriptorSetLayoutBinding;			// Vulkan Spec Minimum Req: 4 Descriptor Sets
			std::map<std::pair<int, int>, util::variable> 				DescriptorSetVariable;

			type														BindPoint;
			std::vector<std::shared_ptr<shader>> 						Shader;								// 5 Stages For Rasterization Graphics (3 are Optional)
			std::shared_ptr<glslang::TProgram> 							Program;							// glslang linked program.
			std::vector<std::vector<uint>> 								ByteCode; 							// SPIRV Byte Code for each Shader Stage.

			virtual ~create_info() = default;

			void generate_descriptor_set_layout_binding();

		};

		// Pre creation options for a rasterizer pipeline.
		struct rasterizer : public create_info {

			friend class pipeline;

			struct attribute {
				VkVertexInputAttributeDescription 		Description;
				util::variable 							Variable;
			};

			struct attachment {
				VkAttachmentDescription 				Description;
				util::variable 							Variable;
			};

			// Vulkan Objects
			VkPipelineInputAssemblyStateCreateInfo						InputAssembly;
			VkPipelineTessellationStateCreateInfo						Tesselation;
			VkPipelineViewportStateCreateInfo							Viewport;
			VkPipelineRasterizationStateCreateInfo						Rasterizer;
			VkPipelineMultisampleStateCreateInfo						Multisample;
			VkPipelineDepthStencilStateCreateInfo						DepthStencil;
			VkPipelineColorBlendStateCreateInfo							ColorBlend;
			VkPipelineDynamicStateCreateInfo							DynamicState;

			math::vec<uint, 3> 											Resolution;
			std::vector<VkViewport> 									DefaultViewport;
			std::vector<VkRect2D> 										DefaultScissor;
			std::vector<VkPipelineColorBlendAttachmentState> 			AttachmentBlendingRules;

			// Vertex metadata
			std::vector<VkVertexInputBindingDescription> 				VertexBufferBindingDescription;		// Vulkan Spec Minimum Req: 16 Vertex Buffer Bindings
			std::vector<attribute> 										VertexAttribute;					// Vulkan Spec Minimum Req: 16 Vertex Attributes

			// Attachment metadata
			std::vector<attachment> 									ColorAttachment;					// Vulkan Spec Minimum Req: 4 Attachments
			attachment 													DepthStencilAttachment;

			rasterizer();
			rasterizer(std::vector<std::shared_ptr<shader>> aShaderList, math::vec<uint, 3> aResolution);

			// bind maps the vertex attributes in the shader to where the vertex buffer is intended to be bound.
			void bind(uint32_t aBindingIndex, size_t aVertexStride, uint32_t aLocationIndex, size_t aVertexOffset, input_rate aInputRate = input_rate::VERTEX);

			// attach attaches an image to a pipeline's output, conveying the format and layout of the image during rendering.
			void attach(uint32_t aAttachmentIndex, std::shared_ptr<image> aAttachmentImage, image::layout aImageLayout = image::layout::SHADER_READ_ONLY_OPTIMAL);
			void attach(uint32_t aAttachmentIndex, image::format aFormat, image::layout aImageLayout = image::layout::SHADER_READ_ONLY_OPTIMAL);

			void resize(math::vec<uint, 3> aResolution);

		};

		// Pre creation options for a raytracer pipeline.
		struct raytracer : public create_info {
			
			struct shader_group {

				// enum type {
				// 	GENERAL 				= VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
				// 	TRIANGLES_HIT_GROUP 	= VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR,
				// 	PROCEDURAL_HIT_GROUP 	= VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR,
				// };

				// type 						Type;
				std::shared_ptr<shader> 		GeneralShader;
				std::shared_ptr<shader> 		ClosestHitShader;
				std::shared_ptr<shader> 		AnyHitShader;
				std::shared_ptr<shader> 		IntersectionShader;
			};

			struct shader_binding_table {
				std::shared_ptr<buffer> 		Buffer;
				VkStridedDeviceAddressRegionKHR Raygen;
				VkStridedDeviceAddressRegionKHR Miss;
				VkStridedDeviceAddressRegionKHR Hit;
				VkStridedDeviceAddressRegionKHR Callable;
			};

			uint32_t 						MaxRecursionDepth;
			std::vector<shader_group> 		ShaderGroup;

			raytracer();
			raytracer(std::vector<shader_group> aShaderGroup, uint32_t aMaxRecursionDepth);

		};

		// Pre creation options for a compute pipeline.
		// Requires on a single compute shader.
		struct compute : public create_info {

			math::vec<uint, 3> 											GroupCount; // Number of Groups
			math::vec<uint, 3> 											GroupSize; //  Number of Items
			VkComputePipelineCreateInfo									CreateInfo{};

			compute();
			//compute(shader* aComputeShader);

		private:

			std::shared_ptr<shader>										Shader;
			std::shared_ptr<glslang::TProgram>							ProgramHandle;
			std::vector<uint>											ByteCode;

		};

		// Reference to Host Memory
		std::shared_ptr<create_info> 							CreateInfo;

		// Pipline Construction.
		std::shared_ptr<context>								Context;
		std::vector<VkPipelineShaderStageCreateInfo> 			Stage;
		VkPipelineBindPoint 									BindPoint;
		VkPipelineLayout 										Layout;
		VkPipelineCache 										Cache;
		VkPipeline 												Handle;
		VkDescriptorPool 										DescriptorPool;
		std::vector<VkDescriptorSetLayout> 						DescriptorSetLayout;
		// Rasterizer Specific
		VkRenderPass											RenderPass;
		// Raytracer Specific
		raytracer::shader_binding_table 						ShaderBindingTable;

		pipeline();

		// Creates rasterizer pipeline.
		pipeline(std::shared_ptr<context> aContext, std::shared_ptr<rasterizer> aRasterizer, VkRenderPass aRenderPass = VK_NULL_HANDLE, uint32_t aSubpassIndex = 0);

		// Creates raytracer pipeline.
		pipeline(std::shared_ptr<context> aContext, std::shared_ptr<raytracer> aRaytracer);

		// Creates compute pipeline.
		pipeline(std::shared_ptr<context> aContext, std::shared_ptr<compute> aCompute);

		// Destructor
		~pipeline();

		static void barrier(
			VkCommandBuffer aCommandBuffer,
			uint aSrcStage, uint aDstStage,
			uint aSrcAccess, uint aDstAccess
		);
		static void barrier(
			VkCommandBuffer aCommandBuffer, 
			uint aSrcStage, uint aDstStage, 
			const std::vector<VkMemoryBarrier>& aMemoryBarrier = {}, 
			const std::vector<VkBufferMemoryBarrier>& aBufferBarrier = {}, 
			const std::vector<VkImageMemoryBarrier>& aImageBarrier = {}
		);
		void begin(VkCommandBuffer aCommandBuffer, std::shared_ptr<framebuffer> aFrame, VkRect2D aRenderArea, VkSubpassContents aSubpassContents = VK_SUBPASS_CONTENTS_INLINE);
		void bind(
			VkCommandBuffer 											aCommandBuffer, 
			std::vector<std::shared_ptr<buffer>> 						aVertexBuffer = {}, 
			std::shared_ptr<buffer> 									aIndexBuffer = nullptr, 
			std::shared_ptr<descriptor::array> 							aDescriptorArray = nullptr
		);
		void end(VkCommandBuffer aCommandBuffer);
		void draw(
			VkCommandBuffer 											aCommandBuffer,
			std::shared_ptr<framebuffer> 								aFramebuffer,
			std::vector<std::shared_ptr<buffer>> 						aVertexBuffer = {},
			std::shared_ptr<buffer> 									aIndexBuffer = nullptr,
			std::shared_ptr<descriptor::array> 							aDescriptorArray = nullptr
		);

		VkResult draw(
			std::shared_ptr<framebuffer> 								aFramebuffer,
			std::vector<std::shared_ptr<buffer>> 						aVertexBuffer = {},
			std::shared_ptr<buffer> 									aIndexBuffer = nullptr,
			std::shared_ptr<descriptor::array> 							aDescriptorArray = nullptr
		);
		VkResult draw(
			std::vector<std::shared_ptr<image>> 						aImage,
			std::vector<std::shared_ptr<buffer>> 						aVertexBuffer = {},
			std::shared_ptr<buffer> 									aIndexBuffer = nullptr,
			std::map<std::pair<int, int>, std::shared_ptr<buffer>> 		aUniformBuffer = {},
			std::map<std::pair<int, int>, std::shared_ptr<image>> 		aSamplerImage = {}
		);

		void raytrace(
			VkCommandBuffer 											aCommandBuffer,
			math::vec<uint, 3> 											aResolution
		);
		VkResult raytrace(
			std::shared_ptr<image> 										aOutputImage,
			std::shared_ptr<acceleration_structure> 					aTLAS,
			std::map<std::pair<int, int>, std::shared_ptr<buffer>> 		aUniformBuffer = {},
			std::map<std::pair<int, int>, std::shared_ptr<image>> 		aSamplerImage = {}
		);

		std::vector<VkDescriptorPoolSize> descriptor_pool_sizes() const;
		std::map<VkDescriptorType, uint32_t> descriptor_type_count() const;
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptor_set_layout_binding() const;

	private:

		VkResult shader_stage_create(std::shared_ptr<create_info> aCreateInfo);
		VkResult create_pipeline_layout(std::vector<std::vector<VkDescriptorSetLayoutBinding>> aDescriptorSetLayoutBinding);

	};

}

#endif // !GEODESY_CORE_GPU_PIPELINE_H
