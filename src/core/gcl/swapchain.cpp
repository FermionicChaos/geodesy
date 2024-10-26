#include <geodesy/core/gcl/swapchain.h>

#include <algorithm>

namespace geodesy::core::gcl {

    swapchain::property::property() {
		this->FrameCount		= 3;
		this->FrameRate			= 60.0f;
		this->PixelFormat		= image::format::B8G8R8A8_UNORM; // HDR is image::format::R16G16B16A16_SFLOAT
		this->ColorSpace		= swapchain::colorspace::SRGB_NONLINEAR;
		this->ImageUsage		= image::usage::COLOR_ATTACHMENT;
		this->CompositeAlpha	= swapchain::composite::ALPHA_OPAQUE;
		this->PresentMode		= swapchain::present_mode::FIFO;
		this->Clipped			= false;
    }

	swapchain::property::property(VkSwapchainCreateInfoKHR aCreateInfo, float aFrameRate) {
		this->FrameCount 		= aCreateInfo.minImageCount;
		this->FrameRate 		= aFrameRate;
		this->PixelFormat 		= (image::format)aCreateInfo.imageFormat;
		this->ColorSpace 		= (swapchain::colorspace)aCreateInfo.imageColorSpace;
		this->ImageUsage 		= (image::usage)aCreateInfo.imageUsage;
		this->CompositeAlpha 	= (swapchain::composite)aCreateInfo.compositeAlpha;
		this->PresentMode 		= (swapchain::present_mode)aCreateInfo.presentMode;
		this->Clipped 			= aCreateInfo.clipped;		
	}

	swapchain::swapchain(std::shared_ptr<context> aContext, VkSurfaceKHR aSurface, const property& aProperty, VkSwapchainKHR aOldSwapchain) : framechain(aContext, aProperty.FrameRate, aProperty.FrameCount) {
        VkResult Result = VK_SUCCESS;

        std::shared_ptr<gcl::device> aDevice                = aContext->Device;
		VkSurfaceCapabilitiesKHR SurfaceCapabilities		= aDevice->get_surface_capabilities(aSurface);
		std::vector<VkSurfaceFormatKHR> SurfaceFormat	    = aDevice->get_surface_format(aSurface);
		std::vector<VkPresentModeKHR> PresentMode		    = aDevice->get_surface_present_mode(aSurface);

        bool SurfaceFormatSupported = false;
		for (size_t i = 0; i < SurfaceFormat.size(); i++) {
			if ((SurfaceFormat[i].format == (VkFormat)aProperty.PixelFormat) && (SurfaceFormat[i].colorSpace == (VkColorSpaceKHR)aProperty.ColorSpace)) {
				SurfaceFormatSupported = true;
				break;
			}
		}

		bool PresentModeSupported = false;
		for (size_t i = 0; i < PresentMode.size(); i++) {
			if (PresentMode[i] == (VkPresentModeKHR)aProperty.PresentMode) {
				PresentModeSupported = true;
				break;
			}
		}

        bool TransformSupported = true;
		// TODO: Test for transform support.

        bool CompositeAlphaSupported = false;
		CompositeAlphaSupported = ((aProperty.CompositeAlpha & SurfaceCapabilities.supportedCompositeAlpha) == aProperty.CompositeAlpha);

		bool ImageUsageSupported = false;
		ImageUsageSupported = ((aProperty.ImageUsage & SurfaceCapabilities.supportedUsageFlags) == aProperty.ImageUsage);

		// Check is options are supported by device and surface.
		if ((!SurfaceFormatSupported) || (!PresentModeSupported) || (!TransformSupported) || (!CompositeAlphaSupported) || (!ImageUsageSupported))
            throw std::runtime_error("Swapchain Creation Failed: Unsupported Options.");
        
        Context                             = aContext;
        CreateInfo                          = {};
    	CreateInfo.sType					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		CreateInfo.pNext					= NULL;
		CreateInfo.flags					= 0;
		CreateInfo.surface					= aSurface;
		CreateInfo.minImageCount			= std::clamp(aProperty.FrameCount, SurfaceCapabilities.minImageCount, SurfaceCapabilities.maxImageCount);
		CreateInfo.imageFormat				= (VkFormat)aProperty.PixelFormat;
		CreateInfo.imageColorSpace			= (VkColorSpaceKHR)aProperty.ColorSpace;
		CreateInfo.imageExtent				= SurfaceCapabilities.currentExtent;
		CreateInfo.imageArrayLayers			= 1;
		CreateInfo.imageUsage				= aProperty.ImageUsage;
		CreateInfo.imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
		CreateInfo.queueFamilyIndexCount	= 0;
		CreateInfo.pQueueFamilyIndices		= NULL;
		CreateInfo.preTransform				= SurfaceCapabilities.currentTransform;
		CreateInfo.compositeAlpha			= (VkCompositeAlphaFlagBitsKHR)aProperty.CompositeAlpha;
		CreateInfo.presentMode				= (VkPresentModeKHR)aProperty.PresentMode;
		CreateInfo.clipped					= aProperty.Clipped;
		CreateInfo.oldSwapchain				= aOldSwapchain;

		this->Resolution = { CreateInfo.imageExtent.width, CreateInfo.imageExtent.height, 1u };

        Result = vkCreateSwapchainKHR(aContext->Handle, &CreateInfo, NULL, &this->Handle);

        if (Result == VK_SUCCESS) {
            // Get swapchain images.
			uint32_t ImageCount = 0;
			Result = vkGetSwapchainImagesKHR(aContext->Handle, this->Handle, &ImageCount, NULL);
			std::vector<VkImage> ImageList(ImageCount);
			// Initialize with default constructed frame objects.
			Result = vkGetSwapchainImagesKHR(aContext->Handle, this->Handle, &ImageCount, ImageList.data());
			for (std::size_t i = 0; i < ImageList.size(); i++) {
				this->Image[i]["Color"] = std::make_shared<image>();
				this->Image[i]["Color"]->Context = aContext;
				this->Image[i]["Color"]->CreateInfo = this->image_create_info();
				this->Image[i]["Color"]->Handle = ImageList[i];
				this->Image[i]["Color"]->transition(image::layout::LAYOUT_UNDEFINED, image::layout::PRESENT_SRC_KHR);
				this->Image[i]["Color"]->View = this->Image[i]["Color"]->view();
			}
        }

    }

    swapchain::~swapchain() {
		for (size_t i = 0; i < this->Image.size(); i++) {
			if (this->Image[i]["Color"]->View != VK_NULL_HANDLE) {
				vkDestroyImageView(this->Context->Handle, this->Image[i]["Color"]->View, NULL);
				this->Image[i]["Color"]->View = VK_NULL_HANDLE;
			}
			this->Image[i]["Color"]->Handle = VK_NULL_HANDLE;
		}
        vkDestroySwapchainKHR(this->Context->Handle, this->Handle, NULL);
    }

	VkImageCreateInfo swapchain::image_create_info() const {
		VkImageCreateInfo ImageCreateInfo{};
		ImageCreateInfo.sType					= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageCreateInfo.pNext					= NULL;
		ImageCreateInfo.flags					= 0;
		ImageCreateInfo.imageType				= VK_IMAGE_TYPE_2D;
		ImageCreateInfo.format					= CreateInfo.imageFormat;
		ImageCreateInfo.extent					= { CreateInfo.imageExtent.width, CreateInfo.imageExtent.height, 1u };
		ImageCreateInfo.mipLevels				= 1;
		ImageCreateInfo.arrayLayers				= CreateInfo.imageArrayLayers;
		ImageCreateInfo.samples					= (VkSampleCountFlagBits)image::sample::COUNT_1;
		ImageCreateInfo.tiling					= (VkImageTiling)image::tiling::OPTIMAL;
		ImageCreateInfo.usage					= CreateInfo.imageUsage;
		ImageCreateInfo.sharingMode				= CreateInfo.imageSharingMode;
		ImageCreateInfo.queueFamilyIndexCount	= 0;
		ImageCreateInfo.pQueueFamilyIndices		= NULL;
		ImageCreateInfo.initialLayout			= (VkImageLayout)image::LAYOUT_UNDEFINED;
		return ImageCreateInfo;
	}

}