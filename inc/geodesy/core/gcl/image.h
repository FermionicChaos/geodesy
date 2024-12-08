#pragma once
#ifndef GEODESY_CORE_GCL_IMAGE_H
#define GEODESY_CORE_GCL_IMAGE_H

#include "../../config.h"

#include "../util/variable.h"

#include "../io/file.h"

#include "device.h"
#include "buffer.h"

namespace geodesy::core::obj {
	class system_window;
}

namespace geodesy::core::gcl {

	class image : public io::file {
	public:

		enum sample {
      		COUNT_1  										= VK_SAMPLE_COUNT_1_BIT,
      		COUNT_2  										= VK_SAMPLE_COUNT_2_BIT,
      		COUNT_4  										= VK_SAMPLE_COUNT_4_BIT,
      		COUNT_8  										= VK_SAMPLE_COUNT_8_BIT,
      		COUNT_16 										= VK_SAMPLE_COUNT_16_BIT,
      		COUNT_32 										= VK_SAMPLE_COUNT_32_BIT,
      		COUNT_64 										= VK_SAMPLE_COUNT_64_BIT,
		};

		enum tiling {
      		OPTIMAL                 						= VK_IMAGE_TILING_OPTIMAL,
      		LINEAR                  						= VK_IMAGE_TILING_LINEAR,
      		DRM_FORMAT_MODIFIER_EXT 						= VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT,
		};

		enum usage : int {
			TRANSFER_SRC 									= VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			TRANSFER_DST 									= VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			SAMPLED 										= VK_IMAGE_USAGE_SAMPLED_BIT,
			STORAGE 										= VK_IMAGE_USAGE_STORAGE_BIT,
			COLOR_ATTACHMENT 								= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			DEPTH_STENCIL_ATTACHMENT 						= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			TRANSIENT_ATTACHMENT 							= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
			INPUT_ATTACHMENT 								= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VIDEO_DECODE_DST 								= VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
			VIDEO_DECODE_SRC 								= VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR,
			VIDEO_DECODE_DPB 								= VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
			FRAGMENT_DENSITY_MAP 							= VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
			FRAGMENT_SHADING_RATE_ATTACHMENT 				= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
		};

		enum format {			
			FORMAT_UNDEFINED                                = VK_FORMAT_UNDEFINED,
			R4G4_UNORM_PACK8                           		= VK_FORMAT_R4G4_UNORM_PACK8,
			R4G4B4A4_UNORM_PACK16                      		= VK_FORMAT_R4G4B4A4_UNORM_PACK16,
			B4G4R4A4_UNORM_PACK16                      		= VK_FORMAT_B4G4R4A4_UNORM_PACK16,
			R5G6B5_UNORM_PACK16                        		= VK_FORMAT_R5G6B5_UNORM_PACK16,
			B5G6R5_UNORM_PACK16                        		= VK_FORMAT_B5G6R5_UNORM_PACK16,
			R5G5B5A1_UNORM_PACK16                      		= VK_FORMAT_R5G5B5A1_UNORM_PACK16,
			B5G5R5A1_UNORM_PACK16                      		= VK_FORMAT_B5G5R5A1_UNORM_PACK16,
			A1R5G5B5_UNORM_PACK16                      		= VK_FORMAT_A1R5G5B5_UNORM_PACK16,
			R8_UNORM                                   		= VK_FORMAT_R8_UNORM,
			R8_SNORM                                   		= VK_FORMAT_R8_SNORM,
			R8_USCALED                                 		= VK_FORMAT_R8_USCALED,
			R8_SSCALED                                 		= VK_FORMAT_R8_SSCALED,
			R8_UINT                                    		= VK_FORMAT_R8_UINT,
			R8_SINT                                    		= VK_FORMAT_R8_SINT,
			R8_SRGB                                    		= VK_FORMAT_R8_SRGB,
			R8G8_UNORM                                 		= VK_FORMAT_R8G8_UNORM,
			R8G8_SNORM                                 		= VK_FORMAT_R8G8_SNORM,
			R8G8_USCALED                               		= VK_FORMAT_R8G8_USCALED,
			R8G8_SSCALED                               		= VK_FORMAT_R8G8_SSCALED,
			R8G8_UINT                                  		= VK_FORMAT_R8G8_UINT,
			R8G8_SINT                                  		= VK_FORMAT_R8G8_SINT,
			R8G8_SRGB                                  		= VK_FORMAT_R8G8_SRGB,
			R8G8B8_UNORM                               		= VK_FORMAT_R8G8B8_UNORM,
			R8G8B8_SNORM                               		= VK_FORMAT_R8G8B8_SNORM,
			R8G8B8_USCALED                             		= VK_FORMAT_R8G8B8_USCALED,
			R8G8B8_SSCALED                             		= VK_FORMAT_R8G8B8_SSCALED,
			R8G8B8_UINT                                		= VK_FORMAT_R8G8B8_UINT,
			R8G8B8_SINT                                		= VK_FORMAT_R8G8B8_SINT,
			R8G8B8_SRGB                                		= VK_FORMAT_R8G8B8_SRGB,
			B8G8R8_UNORM                               		= VK_FORMAT_B8G8R8_UNORM,
			B8G8R8_SNORM                               		= VK_FORMAT_B8G8R8_SNORM,
			B8G8R8_USCALED                             		= VK_FORMAT_B8G8R8_USCALED,
			B8G8R8_SSCALED                             		= VK_FORMAT_B8G8R8_SSCALED,
			B8G8R8_UINT                                		= VK_FORMAT_B8G8R8_UINT,
			B8G8R8_SINT                                		= VK_FORMAT_B8G8R8_SINT,
			B8G8R8_SRGB                                		= VK_FORMAT_B8G8R8_SRGB,
			R8G8B8A8_UNORM                             		= VK_FORMAT_R8G8B8A8_UNORM,
			R8G8B8A8_SNORM                             		= VK_FORMAT_R8G8B8A8_SNORM,
			R8G8B8A8_USCALED                           		= VK_FORMAT_R8G8B8A8_USCALED,
			R8G8B8A8_SSCALED                           		= VK_FORMAT_R8G8B8A8_SSCALED,
			R8G8B8A8_UINT                              		= VK_FORMAT_R8G8B8A8_UINT,
			R8G8B8A8_SINT                              		= VK_FORMAT_R8G8B8A8_SINT,
			R8G8B8A8_SRGB                              		= VK_FORMAT_R8G8B8A8_SRGB,
			B8G8R8A8_UNORM                             		= VK_FORMAT_B8G8R8A8_UNORM,
			B8G8R8A8_SNORM                             		= VK_FORMAT_B8G8R8A8_SNORM,
			B8G8R8A8_USCALED                           		= VK_FORMAT_B8G8R8A8_USCALED,
			B8G8R8A8_SSCALED                           		= VK_FORMAT_B8G8R8A8_SSCALED,
			B8G8R8A8_UINT                              		= VK_FORMAT_B8G8R8A8_UINT,
			B8G8R8A8_SINT                              		= VK_FORMAT_B8G8R8A8_SINT,
			B8G8R8A8_SRGB                              		= VK_FORMAT_B8G8R8A8_SRGB,
			A8B8G8R8_UNORM_PACK32                      		= VK_FORMAT_A8B8G8R8_UNORM_PACK32,
			A8B8G8R8_SNORM_PACK32                      		= VK_FORMAT_A8B8G8R8_SNORM_PACK32,
			A8B8G8R8_USCALED_PACK32                    		= VK_FORMAT_A8B8G8R8_USCALED_PACK32,
			A8B8G8R8_SSCALED_PACK32                    		= VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
			A8B8G8R8_UINT_PACK32                       		= VK_FORMAT_A8B8G8R8_UINT_PACK32,
			A8B8G8R8_SINT_PACK32                       		= VK_FORMAT_A8B8G8R8_SINT_PACK32,
			A8B8G8R8_SRGB_PACK32                       		= VK_FORMAT_A8B8G8R8_SRGB_PACK32,
			A2R10G10B10_UNORM_PACK32                   		= VK_FORMAT_A2R10G10B10_UNORM_PACK32,
			A2R10G10B10_SNORM_PACK32                   		= VK_FORMAT_A2R10G10B10_SNORM_PACK32,
			A2R10G10B10_USCALED_PACK32                 		= VK_FORMAT_A2R10G10B10_USCALED_PACK32,
			A2R10G10B10_SSCALED_PACK32                 		= VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
			A2R10G10B10_UINT_PACK32                    		= VK_FORMAT_A2R10G10B10_UINT_PACK32,
			A2R10G10B10_SINT_PACK32                    		= VK_FORMAT_A2R10G10B10_SINT_PACK32,
			A2B10G10R10_UNORM_PACK32                   		= VK_FORMAT_A2B10G10R10_UNORM_PACK32,
			A2B10G10R10_SNORM_PACK32                   		= VK_FORMAT_A2B10G10R10_SNORM_PACK32,
			A2B10G10R10_USCALED_PACK32                 		= VK_FORMAT_A2B10G10R10_USCALED_PACK32,
			A2B10G10R10_SSCALED_PACK32                 		= VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
			A2B10G10R10_UINT_PACK32                    		= VK_FORMAT_A2B10G10R10_UINT_PACK32,
			A2B10G10R10_SINT_PACK32                    		= VK_FORMAT_A2B10G10R10_SINT_PACK32,
			R16_UNORM                                  		= VK_FORMAT_R16_UNORM,
			R16_SNORM                                  		= VK_FORMAT_R16_SNORM,
			R16_USCALED                                		= VK_FORMAT_R16_USCALED,
			R16_SSCALED                                		= VK_FORMAT_R16_SSCALED,
			R16_UINT                                   		= VK_FORMAT_R16_UINT,
			R16_SINT                                   		= VK_FORMAT_R16_SINT,
			R16_SFLOAT                                 		= VK_FORMAT_R16_SFLOAT,
			R16G16_UNORM                               		= VK_FORMAT_R16G16_UNORM,
			R16G16_SNORM                               		= VK_FORMAT_R16G16_SNORM,
			R16G16_USCALED                             		= VK_FORMAT_R16G16_USCALED,
			R16G16_SSCALED                             		= VK_FORMAT_R16G16_SSCALED,
			R16G16_UINT                                		= VK_FORMAT_R16G16_UINT,
			R16G16_SINT                                		= VK_FORMAT_R16G16_SINT,
			R16G16_SFLOAT                              		= VK_FORMAT_R16G16_SFLOAT,
			R16G16B16_UNORM                            		= VK_FORMAT_R16G16B16_UNORM,
			R16G16B16_SNORM                            		= VK_FORMAT_R16G16B16_SNORM,
			R16G16B16_USCALED                          		= VK_FORMAT_R16G16B16_USCALED,
			R16G16B16_SSCALED                          		= VK_FORMAT_R16G16B16_SSCALED,
			R16G16B16_UINT                             		= VK_FORMAT_R16G16B16_UINT,
			R16G16B16_SINT                             		= VK_FORMAT_R16G16B16_SINT,
			R16G16B16_SFLOAT                           		= VK_FORMAT_R16G16B16_SFLOAT,
			R16G16B16A16_UNORM                         		= VK_FORMAT_R16G16B16A16_UNORM,
			R16G16B16A16_SNORM                         		= VK_FORMAT_R16G16B16A16_SNORM,
			R16G16B16A16_USCALED                       		= VK_FORMAT_R16G16B16A16_USCALED,
			R16G16B16A16_SSCALED                       		= VK_FORMAT_R16G16B16A16_SSCALED,
			R16G16B16A16_UINT                          		= VK_FORMAT_R16G16B16A16_UINT,
			R16G16B16A16_SINT                          		= VK_FORMAT_R16G16B16A16_SINT,
			R16G16B16A16_SFLOAT                        		= VK_FORMAT_R16G16B16A16_SFLOAT,
			R32_UINT                                   		= VK_FORMAT_R32_UINT,
			R32_SINT                                   		= VK_FORMAT_R32_SINT,
			R32_SFLOAT                                 		= VK_FORMAT_R32_SFLOAT,
			R32G32_UINT                                		= VK_FORMAT_R32G32_UINT,
			R32G32_SINT                                		= VK_FORMAT_R32G32_SINT,
			R32G32_SFLOAT                              		= VK_FORMAT_R32G32_SFLOAT,
			R32G32B32_UINT                             		= VK_FORMAT_R32G32B32_UINT,
			R32G32B32_SINT                             		= VK_FORMAT_R32G32B32_SINT,
			R32G32B32_SFLOAT                           		= VK_FORMAT_R32G32B32_SFLOAT,
			R32G32B32A32_UINT                          		= VK_FORMAT_R32G32B32A32_UINT,
			R32G32B32A32_SINT                          		= VK_FORMAT_R32G32B32A32_SINT,
			R32G32B32A32_SFLOAT                        		= VK_FORMAT_R32G32B32A32_SFLOAT,
			R64_UINT                                   		= VK_FORMAT_R64_UINT,
			R64_SINT                                   		= VK_FORMAT_R64_SINT,
			R64_SFLOAT                                 		= VK_FORMAT_R64_SFLOAT,
			R64G64_UINT                                		= VK_FORMAT_R64G64_UINT,
			R64G64_SINT                                		= VK_FORMAT_R64G64_SINT,
			R64G64_SFLOAT                              		= VK_FORMAT_R64G64_SFLOAT,
			R64G64B64_UINT                             		= VK_FORMAT_R64G64B64_UINT,
			R64G64B64_SINT                             		= VK_FORMAT_R64G64B64_SINT,
			R64G64B64_SFLOAT                           		= VK_FORMAT_R64G64B64_SFLOAT,
			R64G64B64A64_UINT                          		= VK_FORMAT_R64G64B64A64_UINT,
			R64G64B64A64_SINT                          		= VK_FORMAT_R64G64B64A64_SINT,
			R64G64B64A64_SFLOAT                        		= VK_FORMAT_R64G64B64A64_SFLOAT,
			B10G11R11_UFLOAT_PACK32                    		= VK_FORMAT_B10G11R11_UFLOAT_PACK32,
			E5B9G9R9_UFLOAT_PACK32                     		= VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
			D16_UNORM                                  		= VK_FORMAT_D16_UNORM,
			X8_D24_UNORM_PACK32                        		= VK_FORMAT_X8_D24_UNORM_PACK32,
			D32_SFLOAT                                 		= VK_FORMAT_D32_SFLOAT,
			S8_UINT                                    		= VK_FORMAT_S8_UINT,
			D16_UNORM_S8_UINT                          		= VK_FORMAT_D16_UNORM_S8_UINT,
			D24_UNORM_S8_UINT                          		= VK_FORMAT_D24_UNORM_S8_UINT,
			D32_SFLOAT_S8_UINT                         		= VK_FORMAT_D32_SFLOAT_S8_UINT,
			BC1_RGB_UNORM_BLOCK                        		= VK_FORMAT_BC1_RGB_UNORM_BLOCK,
			BC1_RGB_SRGB_BLOCK                         		= VK_FORMAT_BC1_RGB_SRGB_BLOCK,
			BC1_RGBA_UNORM_BLOCK                       		= VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
			BC1_RGBA_SRGB_BLOCK                        		= VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
			BC2_UNORM_BLOCK                            		= VK_FORMAT_BC2_UNORM_BLOCK,
			BC2_SRGB_BLOCK                             		= VK_FORMAT_BC2_SRGB_BLOCK,
			BC3_UNORM_BLOCK                            		= VK_FORMAT_BC3_UNORM_BLOCK,
			BC3_SRGB_BLOCK                             		= VK_FORMAT_BC3_SRGB_BLOCK,
			BC4_UNORM_BLOCK                            		= VK_FORMAT_BC4_UNORM_BLOCK,
			BC4_SNORM_BLOCK                            		= VK_FORMAT_BC4_SNORM_BLOCK,
			BC5_UNORM_BLOCK                            		= VK_FORMAT_BC5_UNORM_BLOCK,
			BC5_SNORM_BLOCK                            		= VK_FORMAT_BC5_SNORM_BLOCK,
			BC6H_UFLOAT_BLOCK                          		= VK_FORMAT_BC6H_UFLOAT_BLOCK,
			BC6H_SFLOAT_BLOCK                          		= VK_FORMAT_BC6H_SFLOAT_BLOCK,
			BC7_UNORM_BLOCK                            		= VK_FORMAT_BC7_UNORM_BLOCK,
			BC7_SRGB_BLOCK                             		= VK_FORMAT_BC7_SRGB_BLOCK,
			ETC2_R8G8B8_UNORM_BLOCK                    		= VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
			ETC2_R8G8B8_SRGB_BLOCK                     		= VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
			ETC2_R8G8B8A1_UNORM_BLOCK                  		= VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
			ETC2_R8G8B8A1_SRGB_BLOCK                   		= VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
			ETC2_R8G8B8A8_UNORM_BLOCK                  		= VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
			ETC2_R8G8B8A8_SRGB_BLOCK                   		= VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
			EAC_R11_UNORM_BLOCK                        		= VK_FORMAT_EAC_R11_UNORM_BLOCK,
			EAC_R11_SNORM_BLOCK                        		= VK_FORMAT_EAC_R11_SNORM_BLOCK,
			EAC_R11G11_UNORM_BLOCK                     		= VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
			EAC_R11G11_SNORM_BLOCK                     		= VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
			ASTC_4x4_UNORM_BLOCK                       		= VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
			ASTC_4x4_SRGB_BLOCK                        		= VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
			ASTC_5x4_UNORM_BLOCK                       		= VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
			ASTC_5x4_SRGB_BLOCK                        		= VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
			ASTC_5x5_UNORM_BLOCK                       		= VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
			ASTC_5x5_SRGB_BLOCK                        		= VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
			ASTC_6x5_UNORM_BLOCK                       		= VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
			ASTC_6x5_SRGB_BLOCK                        		= VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
			ASTC_6x6_UNORM_BLOCK                       		= VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
			ASTC_6x6_SRGB_BLOCK                        		= VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
			ASTC_8x5_UNORM_BLOCK                       		= VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
			ASTC_8x5_SRGB_BLOCK                        		= VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
			ASTC_8x6_UNORM_BLOCK                       		= VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
			ASTC_8x6_SRGB_BLOCK                        		= VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
			ASTC_8x8_UNORM_BLOCK                       		= VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
			ASTC_8x8_SRGB_BLOCK                        		= VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
			ASTC_10x5_UNORM_BLOCK                      		= VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
			ASTC_10x5_SRGB_BLOCK                       		= VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
			ASTC_10x6_UNORM_BLOCK                      		= VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
			ASTC_10x6_SRGB_BLOCK                       		= VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
			ASTC_10x8_UNORM_BLOCK                      		= VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
			ASTC_10x8_SRGB_BLOCK                       		= VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
			ASTC_10x10_UNORM_BLOCK                     		= VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
			ASTC_10x10_SRGB_BLOCK                      		= VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
			ASTC_12x10_UNORM_BLOCK                     		= VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
			ASTC_12x10_SRGB_BLOCK                      		= VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
			ASTC_12x12_UNORM_BLOCK                     		= VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
			ASTC_12x12_SRGB_BLOCK                      		= VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
			G8B8G8R8_422_UNORM                         		= VK_FORMAT_G8B8G8R8_422_UNORM,
			B8G8R8G8_422_UNORM                         		= VK_FORMAT_B8G8R8G8_422_UNORM,
			G8_B8_R8_3PLANE_420_UNORM                  		= VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
			G8_B8R8_2PLANE_420_UNORM                   		= VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
			G8_B8_R8_3PLANE_422_UNORM                  		= VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
			G8_B8R8_2PLANE_422_UNORM                   		= VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
			G8_B8_R8_3PLANE_444_UNORM                  		= VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
			R10X6_UNORM_PACK16                         		= VK_FORMAT_R10X6_UNORM_PACK16,
			R10X6G10X6_UNORM_2PACK16                   		= VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
			R10X6G10X6B10X6A10X6_UNORM_4PACK16         		= VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
			G10X6B10X6G10X6R10X6_422_UNORM_4PACK16     		= VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
			B10X6G10X6R10X6G10X6_422_UNORM_4PACK16     		= VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
			G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 		= VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
			G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16  		= VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
			G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 		= VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
			G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16  		= VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
			G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 		= VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
			R12X4_UNORM_PACK16                         		= VK_FORMAT_R12X4_UNORM_PACK16,
			R12X4G12X4_UNORM_2PACK16                   		= VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
			R12X4G12X4B12X4A12X4_UNORM_4PACK16         		= VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
			G12X4B12X4G12X4R12X4_422_UNORM_4PACK16     		= VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
			B12X4G12X4R12X4G12X4_422_UNORM_4PACK16     		= VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
			G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 		= VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
			G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16  		= VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
			G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 		= VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
			G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16  		= VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
			G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 		= VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
			G16B16G16R16_422_UNORM                     		= VK_FORMAT_G16B16G16R16_422_UNORM,
			B16G16R16G16_422_UNORM                     		= VK_FORMAT_B16G16R16G16_422_UNORM,
			G16_B16_R16_3PLANE_420_UNORM               		= VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
			G16_B16R16_2PLANE_420_UNORM                		= VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
			G16_B16_R16_3PLANE_422_UNORM               		= VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
			G16_B16R16_2PLANE_422_UNORM                		= VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
			G16_B16_R16_3PLANE_444_UNORM               		= VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
			G8_B8R8_2PLANE_444_UNORM                   		= VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,
			G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16  		= VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
			G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16  		= VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
			G16_B16R16_2PLANE_444_UNORM                		= VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,
			A4R4G4B4_UNORM_PACK16                      		= VK_FORMAT_A4R4G4B4_UNORM_PACK16,
			A4B4G4R4_UNORM_PACK16                      		= VK_FORMAT_A4B4G4R4_UNORM_PACK16,
			ASTC_4x4_SFLOAT_BLOCK                      		= VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,
			ASTC_5x4_SFLOAT_BLOCK                      		= VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,
			ASTC_5x5_SFLOAT_BLOCK                      		= VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,
			ASTC_6x5_SFLOAT_BLOCK                      		= VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,
			ASTC_6x6_SFLOAT_BLOCK                      		= VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,
			ASTC_8x5_SFLOAT_BLOCK                      		= VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,
			ASTC_8x6_SFLOAT_BLOCK                      		= VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,
			ASTC_8x8_SFLOAT_BLOCK                      		= VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,
			ASTC_10x5_SFLOAT_BLOCK                     		= VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,
			ASTC_10x6_SFLOAT_BLOCK                     		= VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,
			ASTC_10x8_SFLOAT_BLOCK                     		= VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,
			ASTC_10x10_SFLOAT_BLOCK                    		= VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK,
			ASTC_12x10_SFLOAT_BLOCK                    		= VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK,
			ASTC_12x12_SFLOAT_BLOCK                    		= VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,
			PVRTC1_2BPP_UNORM_BLOCK_IMG                		= VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
			PVRTC1_4BPP_UNORM_BLOCK_IMG                		= VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
			PVRTC2_2BPP_UNORM_BLOCK_IMG                		= VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
			PVRTC2_4BPP_UNORM_BLOCK_IMG                		= VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
			PVRTC1_2BPP_SRGB_BLOCK_IMG                 		= VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
			PVRTC1_4BPP_SRGB_BLOCK_IMG                 		= VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
			PVRTC2_2BPP_SRGB_BLOCK_IMG                 		= VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
			PVRTC2_4BPP_SRGB_BLOCK_IMG                 		= VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
			R16G16_S10_5_NV                            		= VK_FORMAT_R16G16_S10_5_NV,
		};

		enum layout : uint {
			LAYOUT_UNDEFINED 											= VK_IMAGE_LAYOUT_UNDEFINED,
			GENERAL 													= VK_IMAGE_LAYOUT_GENERAL,
			COLOR_ATTACHMENT_OPTIMAL 									= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			DEPTH_STENCIL_ATTACHMENT_OPTIMAL 							= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			DEPTH_STENCIL_READ_ONLY_OPTIMAL 							= VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
			SHADER_READ_ONLY_OPTIMAL 									= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			TRANSFER_SRC_OPTIMAL 										= VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			TRANSFER_DST_OPTIMAL 										= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			PREINITIALIZED 												= VK_IMAGE_LAYOUT_PREINITIALIZED,
			DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL 					= VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
			DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL 					= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
			DEPTH_ATTACHMENT_OPTIMAL 									= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			DEPTH_READ_ONLY_OPTIMAL 									= VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
			STENCIL_ATTACHMENT_OPTIMAL 									= VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
			STENCIL_READ_ONLY_OPTIMAL 									= VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
			READ_ONLY_OPTIMAL 											= VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
			ATTACHMENT_OPTIMAL 											= VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			PRESENT_SRC_KHR 											= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VIDEO_DECODE_DST_KHR 										= VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR,
			VIDEO_DECODE_SRC_KHR 										= VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR,
			VIDEO_DECODE_DPB_KHR 										= VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR,
			SHARED_PRESENT_KHR 											= VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
			FRAGMENT_DENSITY_MAP_OPTIMAL_EXT 							= VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
			FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR 				= VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
#ifdef VK_ENABLE_BETA_EXTENSIONS
			VIDEO_ENCODE_DST_KHR 										= VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
			VIDEO_ENCODE_SRC_KHR 										= VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
			VIDEO_ENCODE_DPB_KHR 										= VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,
#endif
			ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT 						= VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT,
			DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR 				= VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR,
			DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR 				= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR,
			SHADING_RATE_OPTIMAL_NV 									= VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV,
			DEPTH_ATTACHMENT_OPTIMAL_KHR 								= VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR,
			DEPTH_READ_ONLY_OPTIMAL_KHR 								= VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL_KHR,
			STENCIL_ATTACHMENT_OPTIMAL_KHR 								= VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL_KHR,
			STENCIL_READ_ONLY_OPTIMAL_KHR 								= VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL_KHR,
			READ_ONLY_OPTIMAL_KHR 										= VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR,
			ATTACHMENT_OPTIMAL_KHR 										= VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
		};

		struct create_info {
			int Layout;
			int Sample;
			int Tiling;
			int Memory;
			int Usage;
			bool MipLevels;
			create_info();
			create_info(int aSample, int aTiling, int aMemory, int aUsage);
		};

		// Will yield the number of bits per pixel.
		static format t2f(util::type::id aID);
		static size_t bytes_per_pixel(int aFormat);
		static size_t bits_per_pixel(int aFormat);
		static VkImageAspectFlags aspect_flag(int aFormat);

		// Image Handle Info
		std::shared_ptr<context> 						Context;
		VkImageCreateInfo 								CreateInfo;
		VkImage											Handle;
		VkImageView 									View;

		// Memory Handle Info
		uint 											MemoryType;
		VkDeviceMemory 									MemoryHandle;

		// Host memory images.
		image();
		// Loads image into host memory.
		image(std::string aFilePath);
		// Creates host image with specified format and dimensions, and singular value provided.
		image(format aFormat, uint aX, uint aY = 1, uint aZ = 1, uint aT = 1, size_t aSourceSize = 0, void* aSourceData = NULL);
		// Loads image into specified device memory.
		image(std::shared_ptr<context> aContext, create_info aCreateInfo, std::string aFilePath);
		// Converts Host image to Device image
		image(std::shared_ptr<context> aContext, create_info aCreateInfo, std::shared_ptr<image> aHostImage);
		// All created images will be in SHADER_READ_ONLY state.
		image(std::shared_ptr<context> aContext, create_info aCreateInfo, format aFormat, uint aX, uint aY = 1, uint aZ = 1, uint aT = 1, void* aTextureData = NULL);
		// Copy Constructor.
		image(image& aInput);
		// Move Constructor.
		image(image&& aInput) noexcept;
		// Destructor
		~image();

		// Copy Assignment.
		image& operator=(image& aRhs);
		// Move Assignment.
		image& operator=(image&& aRhs) noexcept;

		// Scheduled Operations
		void copy(VkCommandBuffer aCommandBuffer, VkOffset3D aDestinationOffset, uint32_t aDestinationArrayLayer, buffer& aSourceData, size_t aSourceOffset, VkExtent3D aRegionExtent, uint32_t aArrayLayerCount = UINT32_MAX);
		void copy(VkCommandBuffer aCommandBuffer, buffer& aSourceData, std::vector<VkBufferImageCopy> aRegionList);
		void copy(VkCommandBuffer aCommandBuffer, VkOffset3D aDestinationOffset, uint32_t aDestinationArrayLayer, image& aSourceData, VkOffset3D aSourceOffset, uint32_t aSourceArrayLayer, VkExtent3D aRegionExtent, uint32_t aArrayLayerCount = UINT32_MAX);
		void copy(VkCommandBuffer aCommandBuffer, image& aSourceData, std::vector<VkImageCopy> aRegionList);
		void transition(
			VkCommandBuffer aCommandBuffer,
			layout aCurrentLayout, layout aFinalLayout,
			VkPipelineStageFlags aSrcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkPipelineStageFlags aDstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			uint32_t aMipLevel = 0, uint32_t aMipLevelCount = UINT32_MAX,
			uint32_t aArrayLayerStart = 0, uint32_t aArrayLayerCount = UINT32_MAX
		);

		// Immediate Operations
		VkResult copy(VkOffset3D aDestinationOffset, uint32_t aDestinationArrayLayer, buffer& aSourceData, size_t aSourceOffset, VkExtent3D aRegionExtent, uint32_t aArrayLayerCount = UINT32_MAX);
		VkResult copy(buffer& aSourceData, std::vector<VkBufferImageCopy> aRegionList);
		VkResult copy(VkOffset3D aDestinationOffset, uint32_t aDestinationArrayLayer, image& aSourceData, VkOffset3D aSourceOffset, uint32_t aSourceArrayLayer, VkExtent3D aRegionExtent, uint32_t aArrayLayerCount = UINT32_MAX);
		VkResult copy(image& aSourceData, std::vector<VkImageCopy> aRegionList);
		VkResult transition(
			layout aCurrentLayout, layout aFinalLayout,
			uint32_t aMipLevel = 0, uint32_t aMipLevelCount = UINT32_MAX,
			uint32_t aArrayLayerStart = 0, uint32_t aArrayLayerCount = UINT32_MAX
		);
		VkResult generate_mipmaps(layout aCurrentLayout, layout aFinalLayout, VkFilter aFilter);

		// Write to image data memory from host memory.
		VkResult write(VkOffset3D aDestinationOffset, uint32_t aDestinationArrayLayer, void* aSourceData, size_t aSourceOffset, VkExtent3D aDestinationExtent, uint32_t aDestinationArrayLayerCount = UINT32_MAX);
		VkResult write(void* aSourceData, std::vector<VkBufferImageCopy> aRegionList);

		// Read from image data memory to host memory.
		VkResult read(VkOffset3D aSourceOffset, uint32_t aSourceArrayLayer, void* aDestinationData, size_t aDestinationOffset, VkExtent3D aSourceExtent, uint32_t aSourceArrayLayerCount = UINT32_MAX);
		VkResult read(void* aDestinationData, std::vector<VkBufferImageCopy> aRegionList);

		VkImageView view(
			uint32_t aMipLevel = 0, uint32_t aMipLevelCount = UINT32_MAX,
			uint32_t aArrayLayerStart = 0, uint32_t aArrayLayerCount = UINT32_MAX
		) const;

		VkAttachmentDescription description(layout aStartingLayout, layout aEndingLayout) const;

		VkImageMemoryBarrier memory_barrier(
			uint aSrcAccess, uint aDstAccess,
			uint aOldLayout, uint aNewLayout,
			uint32_t aMipLevel = 0, uint32_t aMipLevelCount = UINT32_MAX,
			uint32_t aArrayLayerStart = 0, uint32_t aArrayLayerCount = UINT32_MAX
		) const;

		VkMemoryRequirements memory_requirements() const;

		void zero_out();
		void clear();

	};

}

#endif // !GEODESY_CORE_GCL_IMAGE_H
