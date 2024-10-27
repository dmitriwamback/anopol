//
//  pipeline.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-14.
//

#ifndef pipeline_h
#define pipeline_h

#include "scene.h"

namespace anopol::pipeline {

class Pipeline {
public:
    
    //------------------------------------------------------------------------------------------//
    // Structs
    //------------------------------------------------------------------------------------------//
    
    struct layout {
        VkPipelineLayout    pipelineLayout;
        VkRect2D            scissor{};
        VkViewport          viewport{};
        VkRenderPass        renderPass;
        VkPipeline          pipeline;
    };
    
    struct pipeline {
        
        VkPipelineDynamicStateCreateInfo            dynamicState{};
        VkPipelineVertexInputStateCreateInfo        vertexInput{};
        VkPipelineInputAssemblyStateCreateInfo      inputAssembly{};
        VkPipelineRasterizationStateCreateInfo      rasterizer{};
        VkPipelineViewportStateCreateInfo           viewportState{};
        VkPipelineColorBlendStateCreateInfo         colorBlending{};
        
        uint32_t imageIndex;
    };
    
    struct descriptorSets {
        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;
    };
    
    //------------------------------------------------------------------------------------------//
    // Variables
    //------------------------------------------------------------------------------------------//
    
    int currentFrame = 0;
    
    VkRenderPass defaultRenderpass;
    
    anopol::render::UniformBuffer uniformBufferMemory;
    
    std::vector<VkCommandBuffer> commandBuffers     = std::vector<VkCommandBuffer>();
    
    std::vector<VkFramebuffer>   framebuffers       = std::vector<VkFramebuffer>();
    std::vector<VkSemaphore>     imageSemaphores    = std::vector<VkSemaphore>(),
                                 renderSemaphores   = std::vector<VkSemaphore>();
    
    std::vector<VkFence> inFlightFences             = std::vector<VkFence>();
    
    layout* pipelineLayout;
    pipeline* p_pipeline;
    descriptorSets* p_descriptorSets;
    
    std::map<std::string, Scene*> scenes;
    std::map<std::string, VkPipelineShaderStageCreateInfo> shaderModules;
    
    std::vector<anopol::render::Renderable> debugRenderables = std::vector<anopol::render::Renderable>();
    
    //------------------------------------------------------------------------------------------//
    // Methods
    //------------------------------------------------------------------------------------------//
    
    static Pipeline CreatePipeline(std::string shaderFolder);
    
    static VkShaderModule CreateShaderModule(std::vector<char> shaderSource);
    static std::vector<char> LoadShaderContent(std::string path);
    
    void Bind(std::string name);
    void AddScene(Scene* scene, std::string name);
    
private:
    void InitializePipeline();
    void CreateSynchronizedObjects();
    void CreateCommandBuffers();
    VkGraphicsPipelineCreateInfo InitializePipelineInfo();
};


Pipeline Pipeline::CreatePipeline(std::string shaderFolder) {
    Pipeline pipeline = Pipeline();
    
    VkShaderModule vert = CreateShaderModule(LoadShaderContent(shaderFolder+"/spirv/vert.spv")),
                   frag = CreateShaderModule(LoadShaderContent(shaderFolder+"/spirv/frag.spv"));
    
    VkPipelineShaderStageCreateInfo vertex{}, fragment{};
    
    vertex.sType    = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex.stage    = VK_SHADER_STAGE_VERTEX_BIT;
    vertex.module   = vert;
    vertex.pName    = "main";
    
    fragment.sType    = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment.stage    = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment.module   = frag;
    fragment.pName    = "main";
    
    std::map<std::string, VkPipelineShaderStageCreateInfo> programs;
    programs["vert"] = vertex;
    programs["frag"] = fragment;
    
    pipeline.shaderModules = programs;

    pipeline.pipelineLayout = static_cast<Pipeline::layout*>(malloc(1 * sizeof(Pipeline::layout)));
    pipeline.p_pipeline = static_cast<Pipeline::pipeline*>(malloc(1 * sizeof(Pipeline::pipeline)));
    pipeline.p_descriptorSets = static_cast<Pipeline::descriptorSets*>(malloc(1 * sizeof(Pipeline::descriptorSets)));
    
    pipeline.InitializePipeline();
    pipeline.CreateCommandBuffers();
    
    return pipeline;
}





std::vector<char> Pipeline::LoadShaderContent(std::string path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) anopol_assert("Failed to open shader file");
    
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    
    std::cout << fileSize << '\n';
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
        
    return buffer;
}
VkShaderModule Pipeline::CreateShaderModule(std::vector<char> shaderSource) {
    
    VkShaderModule shaderModule;
    
    VkShaderModuleCreateInfo shaderModuleInfo{};
    shaderModuleInfo.sType      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleInfo.codeSize   = shaderSource.size();
    shaderModuleInfo.pCode      = reinterpret_cast<const uint32_t*>(shaderSource.data());
    
    if (vkCreateShaderModule(context->device, &shaderModuleInfo, nullptr, &shaderModule) != VK_SUCCESS) anopol_assert("Failed to compile shader");
    
    return shaderModule;
}
void Pipeline::InitializePipeline() {
    
    //------------------------------------------------------------------------------------------//
    // Preparing
    //------------------------------------------------------------------------------------------//
    
    framebuffers.resize(anopol::ll::swapchainImageViews.size());
    
    commandBuffers.resize(anopol_max_frames);
    renderSemaphores.resize(anopol_max_frames);
    imageSemaphores.resize(anopol_max_frames);
    inFlightFences.resize(anopol_max_frames);
    
    pipelineLayout->viewport.x = 0.0f;
    pipelineLayout->viewport.y = 0.0f;
    pipelineLayout->viewport.minDepth = 0.0f;
    pipelineLayout->viewport.maxDepth = 1.0f;
    pipelineLayout->viewport.width  = context->extent.width;
    pipelineLayout->viewport.height = context->extent.width;
    
    pipelineLayout->scissor.offset = {0, 0};
    pipelineLayout->scissor.extent = context->extent;
    
    //------------------------------------------------------------------------------------------//
    // Debug
    //------------------------------------------------------------------------------------------//
    
    debugRenderables.push_back(anopol::render::Renderable::Create(true));
    debugRenderables.push_back(anopol::render::Renderable::Create());
    
    //------------------------------------------------------------------------------------------//
    // Creating Uniform Buffers
    //------------------------------------------------------------------------------------------//
    
    uniformBufferMemory = anopol::render::UniformBuffer::Create();
    
    VkDescriptorPoolSize poolSize{};
    poolSize.type                   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount        = (uint32_t)anopol_max_frames;
    
    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount    = 1;
    poolCreateInfo.pPoolSizes       = &poolSize;
    poolCreateInfo.maxSets          = (uint32_t)anopol_max_frames;
    
    if (vkCreateDescriptorPool(context->device, &poolCreateInfo, nullptr, &p_descriptorSets->descriptorPool) != VK_SUCCESS) anopol_assert("Failed to create descriptor pool");
    
    //------------------------------------------------------------------------------------------//
    // Creating Necessary Pipeline Inputs
    //------------------------------------------------------------------------------------------//
    
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    p_pipeline->dynamicState.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    p_pipeline->dynamicState.dynamicStateCount  = (uint32_t)dynamicStates.size();
    p_pipeline->dynamicState.pDynamicStates     = dynamicStates.data();
    p_pipeline->dynamicState.flags              = 0;
    p_pipeline->dynamicState.pNext              = nullptr;
    
    
    VkVertexInputBindingDescription description = anopol::render::Vertex::getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> attribute = anopol::render::Vertex::getAttributeDescription();
    
    p_pipeline->vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    p_pipeline->vertexInput.vertexBindingDescriptionCount   = 1;
    p_pipeline->vertexInput.pVertexBindingDescriptions      = &description;
    p_pipeline->vertexInput.vertexAttributeDescriptionCount = attribute.size();
    p_pipeline->vertexInput.pVertexAttributeDescriptions    = attribute.data();
    p_pipeline->vertexInput.flags                           = 0;
    p_pipeline->vertexInput.pNext                           = nullptr;
    
    p_pipeline->inputAssembly.sType                         = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    p_pipeline->inputAssembly.topology                      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    p_pipeline->inputAssembly.primitiveRestartEnable        = VK_FALSE;
    p_pipeline->inputAssembly.flags                         = 0;
    p_pipeline->inputAssembly.pNext                         = nullptr;
    
    p_pipeline->viewportState.sType                         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    p_pipeline->viewportState.viewportCount                 = 1;
    p_pipeline->viewportState.scissorCount                  = 1;
    p_pipeline->viewportState.pViewports                    = &pipelineLayout->viewport;
    p_pipeline->viewportState.pScissors                     = &pipelineLayout->scissor;
    p_pipeline->viewportState.flags                         = 0;
    p_pipeline->viewportState.pNext                         = nullptr;
    
    p_pipeline->rasterizer.sType                            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    p_pipeline->rasterizer.depthClampEnable                 = VK_FALSE;
    p_pipeline->rasterizer.rasterizerDiscardEnable          = VK_FALSE;
    p_pipeline->rasterizer.polygonMode                      = VK_POLYGON_MODE_FILL;
    p_pipeline->rasterizer.lineWidth                        = 1;
    p_pipeline->rasterizer.cullMode                         = VK_CULL_MODE_BACK_BIT;
    p_pipeline->rasterizer.frontFace                        = VK_FRONT_FACE_CLOCKWISE;
    p_pipeline->rasterizer.depthBiasEnable                  = VK_FALSE;
    p_pipeline->rasterizer.depthBiasConstantFactor          = 0;
    p_pipeline->rasterizer.depthBiasClamp                   = 0;
    p_pipeline->rasterizer.depthBiasSlopeFactor             = 0;
    p_pipeline->rasterizer.flags                            = 0;
    p_pipeline->rasterizer.pNext                            = NULL;
    
    VkPipelineColorBlendAttachmentState color{};
    color.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color.blendEnable = VK_FALSE;

    p_pipeline->colorBlending.sType                         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    p_pipeline->colorBlending.logicOpEnable                 = VK_FALSE;
    p_pipeline->colorBlending.attachmentCount               = 1;
    p_pipeline->colorBlending.pAttachments                  = &color;
    p_pipeline->colorBlending.flags                         = 0;
    p_pipeline->colorBlending.pNext                         = NULL;
    
    //------------------------------------------------------------------------------------------//
    // Uniform Descriptor Sets
    //------------------------------------------------------------------------------------------//
    
    VkDescriptorSetLayoutBinding uniformBufferLayoutBinding{};
    uniformBufferLayoutBinding.binding          = 0;
    uniformBufferLayoutBinding.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferLayoutBinding.descriptorCount  = 1;
    uniformBufferLayoutBinding.stageFlags       = VK_SHADER_STAGE_VERTEX_BIT;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings    = &uniformBufferLayoutBinding;
    
    VkDescriptorSetLayout descriptor;
    
    if (vkCreateDescriptorSetLayout(context->device, &layoutInfo, nullptr, &descriptor) != VK_SUCCESS) anopol_assert("Failed to create descriptor");
    
    std::vector<VkDescriptorSetLayout> descriptorLayouts(anopol_max_frames, descriptor);
    
    VkDescriptorSetAllocateInfo descriptorAllocationInfo{};
    descriptorAllocationInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorAllocationInfo.descriptorPool     = p_descriptorSets->descriptorPool;
    descriptorAllocationInfo.descriptorSetCount = (uint32_t)anopol_max_frames;
    descriptorAllocationInfo.pSetLayouts        = descriptorLayouts.data();
    
    p_descriptorSets->descriptorSets.resize(anopol_max_frames);
    if (vkAllocateDescriptorSets(context->device, &descriptorAllocationInfo, p_descriptorSets->descriptorSets.data()) != VK_SUCCESS) anopol_assert("Failed to allocate descriptor sets");
    
    for (size_t i = 0; i < anopol_max_frames; i++) {
        VkDescriptorBufferInfo descriptorBufferInfo{};
        descriptorBufferInfo.buffer = uniformBufferMemory.uniformBuffer[i];
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range  = sizeof(anopol::render::anopolStandardUniform);
        
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet          = p_descriptorSets->descriptorSets[i];
        descriptorWrite.dstBinding      = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo     = &descriptorBufferInfo;
        
        vkUpdateDescriptorSets(context->device, 1, &descriptorWrite, 0, nullptr);
    }
    
    //------------------------------------------------------------------------------------------//
    // Creating Graphics Pipeline
    //------------------------------------------------------------------------------------------//

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount   = 1;
    pipelineLayoutInfo.pSetLayouts      = &descriptor;
    
    if (vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, nullptr, &pipelineLayout->pipelineLayout) != VK_SUCCESS) anopol_assert("Failed to create pipeline");
    
    VkGraphicsPipelineCreateInfo pipelineInfo{};

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format          = context->format;
    colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment   = 0;
    colorAttachmentRef.layout       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;

    VkRenderPassCreateInfo renderpassInfo{};
    renderpassInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassInfo.attachmentCount  = 1;
    renderpassInfo.subpassCount     = 1;
    renderpassInfo.pAttachments     = &colorAttachment;
    renderpassInfo.pSubpasses       = &subpass;

    pipelineInfo.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount             = 2;
    pipelineInfo.pVertexInputState      = &p_pipeline->vertexInput;
    pipelineInfo.pInputAssemblyState    = &p_pipeline->inputAssembly;
    pipelineInfo.pViewportState         = &p_pipeline->viewportState;
    pipelineInfo.pRasterizationState    = &p_pipeline->rasterizer;
    pipelineInfo.pColorBlendState       = &p_pipeline->colorBlending;
    pipelineInfo.pDynamicState          = &p_pipeline->dynamicState;
    
    if (vkCreateRenderPass(context->device, &renderpassInfo, nullptr, &defaultRenderpass) != VK_SUCCESS) anopol_assert("Failed to create RenderPass!");
    
    for (size_t i = 0; i < anopol::ll::swapchainImageViews.size(); i++) {

        VkImageView attachments[] = {anopol::ll::swapchainImageViews[i]};

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType             = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass        = defaultRenderpass;
        framebufferCreateInfo.attachmentCount   = 1;
        framebufferCreateInfo.pAttachments      = attachments;
        framebufferCreateInfo.width             = context->extent.width;
        framebufferCreateInfo.height            = context->extent.height;
        framebufferCreateInfo.layers            = 1;
        
        if (vkCreateFramebuffer(context->device, &framebufferCreateInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) anopol_assert("Couldn't create framebuffers");
    }

    pipelineInfo.layout             = pipelineLayout->pipelineLayout;
    pipelineInfo.renderPass         = defaultRenderpass;
    pipelineInfo.subpass            = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex  = -1;

    VkPipelineShaderStageCreateInfo shaderStages[] = { shaderModules["vert"], shaderModules["frag"] };

    pipelineInfo.pStages = shaderStages;

    if (vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelineLayout->pipeline) != VK_SUCCESS) anopol_assert("Couldn't create VkPipeline");
}

void Pipeline::CreateCommandBuffers() {
    
    //------------------------------------------------------------------------------------------//
    // Creating Command Buffer
    //------------------------------------------------------------------------------------------//

    VkCommandBufferAllocateInfo commandBufferAllocationInfo{};
    commandBufferAllocationInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocationInfo.commandBufferCount  = anopol_max_frames;
    commandBufferAllocationInfo.level               = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocationInfo.commandPool         = ll::commandPool;

    if (vkAllocateCommandBuffers(context->device,
                                &commandBufferAllocationInfo,
                                commandBuffers.data()) != VK_SUCCESS) anopol_assert("Couldn't create command buffer");
    
    //------------------------------------------------------------------------------------------//
    // Creating Semaphores and Fences
    //------------------------------------------------------------------------------------------//

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < anopol_max_frames; i++) {
        if (vkCreateSemaphore(context->device, &semaphoreCreateInfo, nullptr, &imageSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->device, &semaphoreCreateInfo, nullptr, &renderSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            anopol_assert("Couldn't create semaphores");
        }
    }
}

void Pipeline::Bind(std::string name) {
    
    //------------------------------------------------------------------------------------------//
    // Binding
    //------------------------------------------------------------------------------------------//
    
    vkWaitForFences(context->device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    
    vkAcquireNextImageKHR(context->device, context->swapchain, UINT64_MAX, imageSemaphores[currentFrame], VK_NULL_HANDLE, &p_pipeline->imageIndex);
    vkResetFences(context->device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) anopol_assert("Couldn't begin command buffer");
    
    //------------------------------------------------------------------------------------------//
    // Preparing Render Pass
    //------------------------------------------------------------------------------------------//
    
    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType               = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass          = defaultRenderpass;
    renderPassBeginInfo.framebuffer         = framebuffers[p_pipeline->imageIndex];
    renderPassBeginInfo.renderArea.offset   = {0, 0};
    renderPassBeginInfo.renderArea.extent   = context->extent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.5f, 1.0f}}};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
    
    pipelineLayout->viewport.width = static_cast<uint32_t>(context->extent.width);
    pipelineLayout->viewport.height = static_cast<uint32_t>(context->extent.height);

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->pipeline);
    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->pipelineLayout, 0, 1, &p_descriptorSets->descriptorSets[currentFrame], 0, nullptr);
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &pipelineLayout->viewport);
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &pipelineLayout->scissor);
    
    uniformBufferMemory.Update(currentFrame);
    
    //------------------------------------------------------------------------------------------//
    // Rendering objects
    //------------------------------------------------------------------------------------------//
    
    for (anopol::render::Renderable r : debugRenderables) {
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, &r.vertexBuffer.vertexBuffer, offsets);
        vkCmdDraw(commandBuffers[currentFrame], r.vertices.size(), 1, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) anopol_assert("Failed to record command buffer");
    
    //------------------------------------------------------------------------------------------//
    // Submitting
    //------------------------------------------------------------------------------------------//
    
    VkSemaphore waitSemaphores[] = {imageSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signal[] = {renderSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal;
        
    if (vkQueueSubmit(context->presentQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        anopol_assert("Failed to submit the draw command");
    }
    
    //------------------------------------------------------------------------------------------//
    // Presenting
    //------------------------------------------------------------------------------------------//

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signal;

    VkSwapchainKHR swapChains[] = {context->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &p_pipeline->imageIndex;

    vkQueuePresentKHR(context->presentQueue, &presentInfo);
}

}

#endif /* pipeline_h */
