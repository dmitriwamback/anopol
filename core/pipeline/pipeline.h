//
//  pipeline.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-14.
//

#ifndef pipeline_h
#define pipeline_h

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
        VkPipelineMultisampleStateCreateInfo        multisample{};
        
        uint32_t imageIndex;
    };
    
    struct descriptorSets {
        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;
    };
    
    //------------------------------------------------------------------------------------------//
    // Shadows
    //------------------------------------------------------------------------------------------//
    
    struct shadow {
        VkPipeline              shadow, shadowPCF, depthPipeline;
        VkRenderPass            shadowRenderPass;
        VkPipelineLayout        shadowPipelineLayout;
    };
    
    struct shadowImage {
        VkImage                 shadowImage;
        VkDeviceMemory          mem;
        VkImageView             shadowImageView;
        VkSampler               sampler;
    };
    
    struct shadowCascade {
        VkFramebuffer           framebuffer;
        VkImageView             cascadeImageView;
        
        glm::mat4               lookAtProjectionMatrix;
        glm::vec3               sunPosition;
        
        float depth;
    };
    
    struct shadowPushConstants {
        glm::vec4 position;
        uint32_t cascade;
    };
    
    shadow shadowPipelines;
    shadowImage shadowDepthImage;
    shadowPushConstants shadowPushConstantsBlock;
    
    std::array<shadowCascade, anopol_max_cascades> cascades;
    
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
    VkDescriptorSetLayout descriptor;
    
    std::map<std::string, VkPipelineShaderStageCreateInfo> shaderModules;
    
    std::vector<anopol::render::Renderable*> debugRenderables = std::vector<anopol::render::Renderable*>();
    std::vector<anopol::render::Asset*> assets = std::vector<anopol::render::Asset*>();
    
    //------------------------------------------------------------------------------------------//
    // Methods
    //------------------------------------------------------------------------------------------//
    
    static Pipeline CreatePipeline(std::string shaderFolder);
    
    static VkShaderModule CreateShaderModule(std::vector<char> shaderSource);
    static std::vector<char> LoadShaderContent(std::string path);
    
    void Bind(std::string name);
    void CleanUp();
    
private:
    
    VkShaderModule vert, frag;
    
    void InitializePipeline();
    void InitializeShadowDepthPass();
    void CreateSynchronizedObjects();
    void CreateCommandBuffers();
    void RenderScene(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t cascade);
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
    
    pipeline.vert = vert;
    pipeline.frag = frag;

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
    pipelineLayout->viewport.height = context->extent.height;
    
    pipelineLayout->scissor.offset = {0, 0};
    pipelineLayout->scissor.extent = context->extent;
    
    //------------------------------------------------------------------------------------------//
    // Preparing Cascaded Shadow Maps
    //------------------------------------------------------------------------------------------//
    
    InitializeShadowDepthPass();
    
    //------------------------------------------------------------------------------------------//
    // Debug
    //------------------------------------------------------------------------------------------//
    
    for (int i = 0; i < 1; i++) {
        for (int j = 0; j < 1; j++) {
            anopol::render::Renderable* renderable = anopol::render::Renderable::Create();
            renderable->position = glm::vec3((i), 0, (j));
            debugRenderables.push_back(renderable);
        }
    }
    assets.push_back(anopol::render::Asset::Create(""));
    
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
    p_pipeline->rasterizer.cullMode                         = VK_CULL_MODE_FRONT_BIT;
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
    
    p_pipeline->multisample.sType                           = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    p_pipeline->multisample.sampleShadingEnable             = VK_FALSE;
    p_pipeline->multisample.rasterizationSamples            = VK_SAMPLE_COUNT_1_BIT;
    p_pipeline->multisample.minSampleShading                = 1.0f;
    p_pipeline->multisample.alphaToCoverageEnable           = VK_FALSE;
    p_pipeline->multisample.alphaToOneEnable                = VK_FALSE;
    
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
    
    VkAttachmentDescription depth{};
    depth.format                    = anopol::ll::findDepthFormat();
    depth.samples                   = VK_SAMPLE_COUNT_1_BIT;
    depth.loadOp                    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth.stencilLoadOp             = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth.storeOp                   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth.stencilStoreOp            = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth.initialLayout             = VK_IMAGE_LAYOUT_UNDEFINED;
    depth.finalLayout               = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depthReference{};
    depthReference.attachment       = 1;
    depthReference.layout           = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthReference;
    
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depth};
    
    VkSubpassDependency dependency{};
    dependency.srcStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderpassInfo{};
    renderpassInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassInfo.attachmentCount  = attachments.size();
    renderpassInfo.pAttachments     = attachments.data();
    renderpassInfo.subpassCount     = 1;
    renderpassInfo.pSubpasses       = &subpass;
    renderpassInfo.dependencyCount  = 1;
    renderpassInfo.pDependencies    = &dependency;

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

        VkImageView attachments[] = {anopol::ll::swapchainImageViews[i], anopol::ll::depthImageView};

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType             = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass        = defaultRenderpass;
        framebufferCreateInfo.attachmentCount   = 2;
        framebufferCreateInfo.pAttachments      = attachments;
        framebufferCreateInfo.width             = context->extent.width;
        framebufferCreateInfo.height            = context->extent.height;
        framebufferCreateInfo.layers            = 1;
        
        if (vkCreateFramebuffer(context->device, &framebufferCreateInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) anopol_assert("Couldn't create framebuffers");
    }
    
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable            = VK_TRUE;
    depthStencilInfo.depthWriteEnable           = VK_TRUE;
    depthStencilInfo.depthCompareOp             = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable      = VK_TRUE;
    depthStencilInfo.stencilTestEnable          = VK_TRUE;
    depthStencilInfo.minDepthBounds             = 0.0f;
    depthStencilInfo.maxDepthBounds             = 1.0f;

    pipelineInfo.layout             = pipelineLayout->pipelineLayout;
    pipelineInfo.renderPass         = defaultRenderpass;
    pipelineInfo.subpass            = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex  = -1;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pMultisampleState  = &p_pipeline->multisample;

    VkPipelineShaderStageCreateInfo shaderStages[] = { shaderModules["vert"], shaderModules["frag"] };

    pipelineInfo.pStages = shaderStages;

    if (vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelineLayout->pipeline) != VK_SUCCESS) anopol_assert("Couldn't create VkPipeline");
    
    vkDestroyShaderModule(context->device, vert, nullptr);
    vkDestroyShaderModule(context->device, frag, nullptr);
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

    VkClearValue clearColor = {{{0.3f, 0.3f, 0.3f, 1.0f}}};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
    
    pipelineLayout->viewport.width = static_cast<uint32_t>(context->extent.width);
    pipelineLayout->viewport.height = static_cast<uint32_t>(context->extent.height);
    pipelineLayout->viewport.x = 0.0f;

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->pipeline);
    vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->pipelineLayout, 0, 1, &p_descriptorSets->descriptorSets[currentFrame], 0, nullptr);
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &pipelineLayout->viewport);
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &pipelineLayout->scissor);
    
    uniformBufferMemory.Update(currentFrame);
    
    //------------------------------------------------------------------------------------------//
    // Rendering objects
    //------------------------------------------------------------------------------------------//
    
    for (anopol::render::Renderable* r : debugRenderables) {
        
        uniformBufferMemory.Model(r->position, r->rotation, r->scale, currentFrame);
        uniformBufferMemory.Update(currentFrame);
        
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, &r->vertexBuffer.vertexBuffer, offsets);
        if (r->isIndexed) {
            vkCmdBindIndexBuffer(commandBuffers[currentFrame], r->indexBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(r->indices.size()), 1, 0, 0, 0);
        }
        else {
            vkCmdDraw(commandBuffers[currentFrame], static_cast<uint32_t>(r->vertices.size()), 1, 0, 0);
        }
    }
    
    for (anopol::render::Asset* a : assets) {
        
        uniformBufferMemory.Model(a->position, a->rotation, a->scale, currentFrame);
        uniformBufferMemory.Update(currentFrame);
        
        for (anopol::render::Asset::Mesh mesh : a->meshes) {
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, &mesh.vertexBuffer.vertexBuffer, offsets);
            vkCmdBindIndexBuffer(commandBuffers[currentFrame], mesh.indexBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
        }
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
    vkQueueWaitIdle(context->graphicsQueue);
}

void Pipeline::InitializeShadowDepthPass() {
    
    VkFormat depth = anopol::ll::findDepthFormat();
    
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format          = depth;
    depthAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    
    VkAttachmentReference depthAttachmentRenference{};
    depthAttachmentRenference.attachment    = 0;
    depthAttachmentRenference.layout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription depthSubpass{};
    depthSubpass.pipelineBindPoint          = VK_PIPELINE_BIND_POINT_GRAPHICS;
    depthSubpass.colorAttachmentCount       = 0;
    depthSubpass.pDepthStencilAttachment    = &depthAttachmentRenference;
    
    std::array<VkSubpassDependency, 2> subpassDependencies;
    
    subpassDependencies[0].srcSubpass       = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].dstSubpass       = 0;
    subpassDependencies[0].srcStageMask     = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    subpassDependencies[0].dstStageMask     = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependencies[0].srcAccessMask    = VK_ACCESS_SHADER_READ_BIT;
    subpassDependencies[0].dstAccessMask    = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpassDependencies[0].dependencyFlags  = VK_DEPENDENCY_BY_REGION_BIT;
    
    subpassDependencies[1].srcSubpass       = 0;
    subpassDependencies[1].dstSubpass       = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].srcStageMask     = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    subpassDependencies[1].dstStageMask     = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    subpassDependencies[1].srcAccessMask    = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpassDependencies[1].dstAccessMask    = VK_ACCESS_SHADER_READ_BIT;
    subpassDependencies[1].dependencyFlags  = VK_DEPENDENCY_BY_REGION_BIT;
    
    VkRenderPassCreateInfo renderpassCreateInfo{};
    renderpassCreateInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassCreateInfo.attachmentCount  = 1;
    renderpassCreateInfo.pAttachments     = &depthAttachment;
    renderpassCreateInfo.subpassCount     = 1;
    renderpassCreateInfo.pSubpasses       = &depthSubpass;
    renderpassCreateInfo.dependencyCount  = static_cast<uint32_t>(subpassDependencies.size());
    renderpassCreateInfo.pDependencies    = subpassDependencies.data();
    
    if (vkCreateRenderPass(context->device, &renderpassCreateInfo, nullptr, &shadowPipelines.shadowRenderPass) != VK_SUCCESS) {
        anopol_assert("Failed to create shadow depth renderpass");
    }
    
    
    anopol::ll::createImage(4096,
                            4096,
                            depth,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            shadowDepthImage.shadowImage, shadowDepthImage.mem);
    
    shadowDepthImage.shadowImageView = anopol::ll::createImageView(shadowDepthImage.shadowImage, depth, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D_ARRAY, anopol_max_cascades);
    
    for (uint32_t i = 0; i < anopol_max_cascades; i++) {
        cascades[i].cascadeImageView = anopol::ll::createImageView(shadowDepthImage.shadowImage, depth, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D_ARRAY);
        
        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType             = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass        = shadowPipelines.shadowRenderPass;
        framebufferCreateInfo.attachmentCount   = 1;
        framebufferCreateInfo.pAttachments      = &cascades[i].cascadeImageView;
        framebufferCreateInfo.width             = 4096;
        framebufferCreateInfo.height            = 4096;
        framebufferCreateInfo.layers            = 1;
        
        if (vkCreateFramebuffer(context->device, &framebufferCreateInfo, nullptr, &cascades[i].framebuffer) != VK_SUCCESS) {
            anopol_assert("Failed to create CSM framebuffer");
        }
    }
    
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter     = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter     = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeV  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeW  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.mipLodBias    = 0.0f;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    samplerCreateInfo.minLod        = 0.0f;
    samplerCreateInfo.maxLod        = 1.0f;
    samplerCreateInfo.borderColor   = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    if (vkCreateSampler(context->device, &samplerCreateInfo, nullptr, &shadowDepthImage.sampler) != VK_SUCCESS) {
        anopol_assert("Failed to create shadow sampler");
    }
}

void Pipeline::RenderScene(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t cascade = 0) {
    
}

void Pipeline::CleanUp() {
    
    for (VkFramebuffer framebuffer : framebuffers) {
        vkDestroyFramebuffer(context->device, framebuffer, nullptr);
    }
    
    for (shadowCascade cascade : cascades) {
        vkDestroyFramebuffer(context->device, cascade.framebuffer, nullptr);
        vkDestroyImageView(context->device, cascade.cascadeImageView, nullptr);
    }
    
    anopol::ll::freeSwapchain();
    
    vkDestroyPipeline(context->device, pipelineLayout->pipeline, nullptr);
    vkDestroyPipelineLayout(context->device, pipelineLayout->pipelineLayout, nullptr);
    vkDestroyRenderPass(context->device, defaultRenderpass, nullptr);
    
    vkFreeCommandBuffers(context->device, ll::commandPool, anopol_max_frames, commandBuffers.data());
    
    for (anopol::render::Renderable* renderable : debugRenderables) {
        renderable->vertexBuffer.dealloc();
        renderable->indexBuffer.dealloc();
    }
    
    for (anopol::render::Asset* asset : assets) {
        for (anopol::render::Asset::Mesh mesh : asset->meshes) {
            mesh.vertexBuffer.dealloc();
            mesh.indexBuffer.dealloc();
        }
    }
    
    uniformBufferMemory.dealloc();
    
    vkDestroyDescriptorPool(context->device, p_descriptorSets->descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(context->device, descriptor, nullptr);
    
    for (size_t i = 0; i < anopol_max_frames; i++) {
        vkDestroySemaphore(context->device, renderSemaphores[i], nullptr);
        vkDestroySemaphore(context->device, imageSemaphores[i], nullptr);
        vkDestroyFence(context->device, inFlightFences[i], nullptr);
    }
    
    vkDestroyImageView(context->device, shadowDepthImage.shadowImageView, nullptr);
    vkDestroyImage(context->device, shadowDepthImage.shadowImage, nullptr);
    vkFreeMemory(context->device, shadowDepthImage.mem, nullptr);
    vkDestroySampler(context->device, shadowDepthImage.sampler, nullptr);
    vkDestroyRenderPass(context->device, shadowPipelines.shadowRenderPass, nullptr);
    
    free(pipelineLayout);
    free(p_pipeline);
    free(p_descriptorSets);
}

}

#endif /* pipeline_h */
