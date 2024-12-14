#pragma once
#ifndef GEODESY_CORE_GCL_SWAPCHAIN_H
#define GEODESY_CORE_GCL_SWAPCHAIN_H

#include "config.h"
#include "image.h"
#include "pipeline.h"
#include "framechain.h"

namespace geodesy::core::gcl {

    class swapchain : public framechain {
    public:

    	enum colorspace {
			SRGB_NONLINEAR          	= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
			DISPLAY_P3_NONLINEAR    	= VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT,
			EXTENDED_SRGB_LINEAR    	= VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT,
			DISPLAY_P3_LINEAR       	= VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT,
			DCI_P3_NONLINEAR        	= VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT,
			BT709_LINEAR            	= VK_COLOR_SPACE_BT709_LINEAR_EXT,
			BT709_NONLINEAR         	= VK_COLOR_SPACE_BT709_NONLINEAR_EXT,
			BT2020_LINEAR           	= VK_COLOR_SPACE_BT2020_LINEAR_EXT,
			HDR10_ST2084            	= VK_COLOR_SPACE_HDR10_ST2084_EXT,
			DOLBYVISION             	= VK_COLOR_SPACE_DOLBYVISION_EXT,
			HDR10_HLG               	= VK_COLOR_SPACE_HDR10_HLG_EXT,
			ADOBERGB_LINEAR         	= VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT,
			ADOBERGB_NONLINEAR      	= VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT,
			PASS_THROUGH            	= VK_COLOR_SPACE_PASS_THROUGH_EXT,
			EXTENDED_SRGB_NONLINEAR 	= VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT,
			DISPLAY_NATIVE_AMD      	= VK_COLOR_SPACE_DISPLAY_NATIVE_AMD,
		};

		enum composite {
			ALPHA_OPAQUE          		= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			ALPHA_PRE_MULTIPLIED  		= VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			ALPHA_POST_MULTIPLIED 		= VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			ALPHA_INHERIT         		= VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};

		enum present_mode {
			IMMEDIATE                 	= VK_PRESENT_MODE_IMMEDIATE_KHR,
			MAILBOX                   	= VK_PRESENT_MODE_MAILBOX_KHR,
			FIFO                      	= VK_PRESENT_MODE_FIFO_KHR,
			FIFO_RELAXED              	= VK_PRESENT_MODE_FIFO_RELAXED_KHR,
			SHARED_DEMAND_REFRESH     	= VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
			SHARED_CONTINUOUS_REFRESH 	= VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR,
		};

        struct property {
			uint32_t					    FrameCount;
			float 							FrameRate;
			image::format 					PixelFormat;
			colorspace						ColorSpace;
            int 							ImageUsage;
			composite 						CompositeAlpha;
			present_mode 					PresentMode;
			bool 							Clipped;
            property();
			property(VkSwapchainCreateInfoKHR aCreateInfo, float aFrameRate);
		};

		VkSurfaceKHR 							Surface;
		VkSwapchainCreateInfoKHR				CreateInfo;
		VkSwapchainKHR							Handle;

        swapchain(std::shared_ptr<context> aContext, VkSurfaceKHR aSurface, const property& aProperty);
        ~swapchain();

		VkImageCreateInfo image_create_info() const;

		VkResult next_frame(VkSemaphore aSemaphore = VK_NULL_HANDLE, VkFence aFence = VK_NULL_HANDLE);
		command_batch next_frame() override;
		std::vector<command_batch> present_frame() override;

	private:

		VkResult create_swapchain(std::shared_ptr<context> aContext, VkSurfaceKHR aSurface, const property& aProperty, VkSwapchainKHR aOldSwapchain = VK_NULL_HANDLE);
		void clear();

    };

}

#endif // !GEODESY_CORE_GCL_SWAPCHAIN_H