//
//  offscreen.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-05-19.
//

#ifndef offscreen_h
#define offscreen_h

namespace anopol::render {

class OffscreenRendering {
public:
    static OffscreenRendering Create();
    void Render(anopol::batch::Batch batch, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame);
    void End(VkCommandBuffer commandBuffer, uint32_t currentFrame);
    void Free();
private:
    VkRenderPass renderPass;
    std::vector<VkImage>          colorImages;
    std::vector<VkDeviceMemory>   colorImageMemory;
    std::vector<VkImageView>      colorImageViews;
    std::vector<VkFramebuffer>    framebuffers;
};

OffscreenRendering OffscreenRendering::Create() {
    OffscreenRendering offscreen = OffscreenRendering();
    
    offscreen.colorImages.resize(anopol_max_frames);
    offscreen.colorImageMemory.resize(anopol_max_frames);
    offscreen.colorImageViews.resize(anopol_max_frames);
    offscreen.framebuffers.resize(anopol_max_frames);
    
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format         = context->format;
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // --- DEPTH ATTACHMENT ---
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format         = anopol::ll::findDepthFormat();
    depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // --- COLOR REFERENCE ---
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // --- DEPTH REFERENCE ---
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // --- SUBPASS ---
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // --- SUBPASS DEPENDENCY (to external) ---
    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // --- RENDERPASS CREATE INFO ---
    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;

    if (vkCreateRenderPass(context->device, &renderPassInfo, nullptr, &offscreen.renderPass) != VK_SUCCESS) {
        anopol_assert("Failed to create render pass");
    }
    
    for (uint32_t i = 0; i < anopol_max_frames; i++) {
        
        anopol::ll::createImage(context->extent.width,
                                context->extent.height,
                                context->format,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                offscreen.colorImages[i],
                                offscreen.colorImageMemory[i]);
        
        VkImageViewCreateInfo colorViewInfo{};
        colorViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorViewInfo.image = offscreen.colorImages[i];
        colorViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorViewInfo.format = context->format;
        colorViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorViewInfo.subresourceRange.levelCount = 1;
        colorViewInfo.subresourceRange.layerCount = 1;
        
        if (vkCreateImageView(context->device, &colorViewInfo, nullptr, &offscreen.colorImageViews[i]) != VK_SUCCESS) {
            anopol_assert("Failed to create color image view");
        }
        
        std::array<VkImageView, 2> attachments = {
            offscreen.colorImageViews[i],
            anopol::ll::depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = offscreen.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = context->extent.width;
        framebufferInfo.height = context->extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(context->device, &framebufferInfo, nullptr, &offscreen.framebuffers[i]) != VK_SUCCESS) {
            anopol_assert("Failed to create framebuffer");
        }
    }
    
    return offscreen;
}


void OffscreenRendering::Render(anopol::batch::Batch batch, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t currentFrame) {
    
    VkClearValue clearValues[2];
    clearValues[0].color        = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = renderPass;
    renderPassInfo.framebuffer       = framebuffers[currentFrame];
    renderPassInfo.renderArea.extent = context->extent;
    renderPassInfo.clearValueCount   = 2;
    renderPassInfo.pClearValues      = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &batch.vertexBuffer.vertexBuffer, offsets);
        
    anopol::render::anopolStandardPushConstants standardPushConstants{};
    standardPushConstants.scale             = glm::vec4(glm::vec3(0), 1.0f);
    standardPushConstants.position          = glm::vec4(glm::vec3(0), 1.0f);
    standardPushConstants.rotation          = glm::vec4(glm::vec3(0), 1.0f);
    standardPushConstants.color             = glm::vec4(glm::vec3(0), 1.0f);
    
    standardPushConstants.instanced = false;
    standardPushConstants.batched = true;
    
    glm::mat4 model = modelMatrix(standardPushConstants.position,
                                  standardPushConstants.scale,
                                  standardPushConstants.rotation);
    
    standardPushConstants.model = model;
    
    vkCmdPushConstants(commandBuffer,
                       pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0,
                       sizeof(anopol::render::anopolStandardPushConstants),
                       &standardPushConstants);
    vkCmdDrawIndirect(commandBuffer, batch.GetBatchFrame(currentFrame).drawCommandBuffer, 0, static_cast<uint32_t>(batch.transformations.size()), sizeof(VkDrawIndirectCommand));
    
    End(commandBuffer, currentFrame);
}

void OffscreenRendering::End(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
    
    vkCmdEndRenderPass(commandBuffer);

    anopol::ll::imageLayoutTransition(
        colorImages[currentFrame],
        context->format,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
}


void OffscreenRendering::Free() {
    for (VkFramebuffer framebuffer : framebuffers) {
        if (framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(context->device, framebuffer, nullptr);
        }
    }
    framebuffers.clear();

    for (VkImageView view : colorImageViews) {
        if (view != VK_NULL_HANDLE) {
            vkDestroyImageView(context->device, view, nullptr);
        }
    }
    colorImageViews.clear();

    for (size_t i = 0; i < colorImages.size(); ++i) {
        if (colorImages[i] != VK_NULL_HANDLE) {
            vkDestroyImage(context->device, colorImages[i], nullptr);
        }
        if (colorImageMemory[i] != VK_NULL_HANDLE) {
            vkFreeMemory(context->device, colorImageMemory[i], nullptr);
        }
    }
    colorImages.clear();
    colorImageMemory.clear();

    if (renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(context->device, renderPass, nullptr);
        renderPass = VK_NULL_HANDLE;
    }
}

}

#endif /* offscreen_h */
