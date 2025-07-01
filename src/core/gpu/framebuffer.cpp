#include <geodesy/core/gpu/framebuffer.h>

#include <geodesy/core/gpu/context.h>

namespace geodesy::core::gpu {

	framebuffer::framebuffer(std::shared_ptr<context> aContext, std::shared_ptr<pipeline> aPipeline, std::vector<std::shared_ptr<image>> aImageAttachements, math::vec<uint, 3> aResolution) {
		this->Context = aContext;
		this->ClearValue = std::vector<VkClearValue>(aImageAttachements.size());
		for (size_t i = 0; i < aImageAttachements.size(); i++) {
			this->ClearValue[i].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		}
		std::vector<VkImageView> Attachment(aImageAttachements.size());
		for (size_t i = 0; i < aImageAttachements.size(); i++) {
			Attachment[i] = aImageAttachements[i]->View;
		}
		VkResult Result = VK_SUCCESS;
		VkFramebufferCreateInfo FBCI{};
		FBCI.sType				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FBCI.pNext				= NULL;
		FBCI.flags				= 0;
		FBCI.renderPass			= aPipeline->RenderPass;
		FBCI.attachmentCount	= Attachment.size();
		FBCI.pAttachments		= Attachment.data();
		FBCI.width				= aResolution[0];
		FBCI.height				= aResolution[1];
		FBCI.layers				= 1;
		Result = vkCreateFramebuffer(aContext->Handle, &FBCI, NULL, &this->Handle);
	}

	framebuffer::framebuffer(std::shared_ptr<context> aContext, std::shared_ptr<pipeline> aPipeline, std::map<std::string, std::shared_ptr<image>> aImage, std::vector<std::string> aAttachmentSelection, math::vec<uint, 3> aResolution) {
		this->Context = aContext;
		this->ClearValue = std::vector<VkClearValue>(aAttachmentSelection.size());
		for (size_t i = 0; i < aAttachmentSelection.size(); i++) {
			this->ClearValue[i].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		}
		std::vector<VkImageView> Attachment(aAttachmentSelection.size());
		for (size_t i = 0; i < aAttachmentSelection.size(); i++) {
			Attachment[i] = aImage[aAttachmentSelection[i]]->View;
		}
		VkResult Result = VK_SUCCESS;
		VkFramebufferCreateInfo FBCI{};
		FBCI.sType				= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		FBCI.pNext				= NULL;
		FBCI.flags				= 0;
		FBCI.renderPass			= aPipeline->RenderPass;
		FBCI.attachmentCount	= Attachment.size();
		FBCI.pAttachments		= Attachment.data();
		FBCI.width				= aResolution[0];
		FBCI.height				= aResolution[1];
		FBCI.layers				= 1;
		Result = vkCreateFramebuffer(aContext->Handle, &FBCI, NULL, &this->Handle);
	}

	framebuffer::~framebuffer() {
		if (this->Handle != VK_NULL_HANDLE) {
			vkDestroyFramebuffer(this->Context->Handle, this->Handle, NULL);
		}
	}

}
