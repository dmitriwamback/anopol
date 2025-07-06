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
    
    struct CameraAdjustment {
        glm::vec3 normal;
        float depth;
    };
    
    //------------------------------------------------------------------------------------------//
    // Variables
    //------------------------------------------------------------------------------------------//
    
    int currentFrame = 0;
    
    VkRenderPass defaultRenderpass;
    
    anopol::render::UniformBuffer   uniformBufferMemory;
    anopol::render::InstanceBuffer* instanceBuffer;
    
    std::vector<VkCommandBuffer>    commandBuffers     = std::vector<VkCommandBuffer>();
    std::vector<VkFramebuffer>      framebuffers       = std::vector<VkFramebuffer>();
    std::vector<VkSemaphore>        imageSemaphores    = std::vector<VkSemaphore>(),
                                    renderSemaphores   = std::vector<VkSemaphore>();
    std::vector<VkFence>            inFlightFences     = std::vector<VkFence>();
    
    pipeline*                       anopolMainPipeline;
    pipelineDefinitions*            anopolPipelineDefinitions;
    VkDescriptorSetLayout           samplerDescriptorSetLayout;
    
    VkDescriptorSet                 samplerDescriptorSet;
    
    std::map<std::string, VkPipelineShaderStageCreateInfo> shaderModules;
    
    std::vector<anopol::render::Renderable*>    debugRenderables = std::vector<anopol::render::Renderable*>();
    std::vector<anopol::render::Asset*>         assets = std::vector<anopol::render::Asset*>();
    
    anopol::batch::Batch testBatch;
    anopol::render::texture::Texture texture, texture2;
    
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
    bool isLeftMouseButtonDown = false;
    
    anopol::render::OffscreenRendering offscreen;
    
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

    pipeline.anopolMainPipeline         = static_cast<struct pipeline*>(malloc(1 * sizeof(struct pipeline)));
    pipeline.anopolPipelineDefinitions  = static_cast<pipelineDefinitions*>(malloc(1 * sizeof(pipelineDefinitions)));
    
    pipeline.InitializePipeline();
    pipeline.CreateCommandBuffers();
    
    return pipeline;
}





std::vector<char> Pipeline::LoadShaderContent(std::string path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) anopol_assert("Failed to open shader file");
    
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
        
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
    
    anopolMainPipeline->viewport.x = 0.0f;
    anopolMainPipeline->viewport.y = 0.0f;
    anopolMainPipeline->viewport.minDepth = 0.0f;
    anopolMainPipeline->viewport.maxDepth = 1.0f;
    anopolMainPipeline->viewport.width  = context->extent.width;
    anopolMainPipeline->viewport.height = context->extent.height;
    
    anopolMainPipeline->scissor.offset = {0, 0};
    anopolMainPipeline->scissor.extent = context->extent;
    
    //------------------------------------------------------------------------------------------//
    // Preparing Cascaded Shadow Maps
    //------------------------------------------------------------------------------------------//
    
    InitializeShadowDepthPass();
    
    //------------------------------------------------------------------------------------------//
    // Debug
    //------------------------------------------------------------------------------------------//
    
    testBatch = anopol::batch::Batch::Create();
    
    offscreen = anopol::render::OffscreenRendering::Create();
    
    int length = 200;
    int idx = 0;
    
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < length; j++) {
            anopol::render::Renderable* renderable = anopol::render::Renderable::Create();
            renderable->position = glm::vec3((i - length/2) * 15.f, 0, (j - length/2) * 15.f);
            renderable->scale    = glm::vec3(10.0f, 10.f, 10.0f);
            renderable->rotation = glm::vec3(rand()%360);
            renderable->color    = glm::vec3(rand()%255/255.0f, rand()%255/255.0f, rand()%255/255.0f);
                        
            testBatch.Append(renderable);
            idx++;
        }
    }
    anopol::render::Asset* testAsset = anopol::render::Asset::Create("");
    testBatch.Combine();
    
    int instance_size = 100;
    
    for (int i = 0; i < instance_size; i++) {
        for (int j = 0; j < instance_size; j++) {
            
            float x = (i - instance_size / 2) * 2.0f;
            float z = (j - instance_size / 2) * 2.0f;
            
            float y = floor(math::overlapNoise((x + 0.01f) / 32.25f, (z + 0.01f) / 32.25f, 0.4, 1.8, 10, 1039.3f * 10.0f));
                        
            testAsset->PushInstance(glm::vec3(x, -100.0f + y, z), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.f, 0.f, 0.f) , glm::vec3(1.0f, 0.8f, 0.7f));
        }
    }
    testAsset->AllocInstances();
    
    texture = anopol::render::texture::Texture::LoadTexture("/Users/dmitriwamback/Documents/Projects/anopol/anopol/textures/wall.jpg");
    texture2 = anopol::render::texture::Texture::LoadTexture("/Users/dmitriwamback/Documents/Projects/anopol/anopol/textures/diamondplate.jpg");
    
    assets.push_back(testAsset);
    
    //------------------------------------------------------------------------------------------//
    // Creating Uniform Buffers and Instance Buffers
    //------------------------------------------------------------------------------------------//
    
    uniformBufferMemory = anopol::render::UniformBuffer::Create();
    instanceBuffer      = new anopol::render::InstanceBuffer();
    instanceBuffer->alloc(1000000);
    instanceBuffer->appendInstance(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
    
    //------------------------------------------------------------------------------------------//
    // Creating Necessary Pipeline Inputs
    //------------------------------------------------------------------------------------------//
    
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    anopolPipelineDefinitions->dynamicState.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    anopolPipelineDefinitions->dynamicState.dynamicStateCount  = (uint32_t)dynamicStates.size();
    anopolPipelineDefinitions->dynamicState.pDynamicStates     = dynamicStates.data();
    anopolPipelineDefinitions->dynamicState.flags              = 0;
    anopolPipelineDefinitions->dynamicState.pNext              = nullptr;
    
    
    VkVertexInputBindingDescription description = anopol::render::Vertex::getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> attribute = anopol::render::Vertex::getAttributeDescription();
    
    anopolPipelineDefinitions->vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    anopolPipelineDefinitions->vertexInput.vertexBindingDescriptionCount   = 1;
    anopolPipelineDefinitions->vertexInput.pVertexBindingDescriptions      = &description;
    anopolPipelineDefinitions->vertexInput.vertexAttributeDescriptionCount = attribute.size();
    anopolPipelineDefinitions->vertexInput.pVertexAttributeDescriptions    = attribute.data();
    anopolPipelineDefinitions->vertexInput.flags                           = 0;
    anopolPipelineDefinitions->vertexInput.pNext                           = nullptr;
    
    anopolPipelineDefinitions->inputAssembly.sType                         = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    anopolPipelineDefinitions->inputAssembly.topology                      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    anopolPipelineDefinitions->inputAssembly.primitiveRestartEnable        = VK_FALSE;
    anopolPipelineDefinitions->inputAssembly.flags                         = 0;
    anopolPipelineDefinitions->inputAssembly.pNext                         = nullptr;
    
    anopolPipelineDefinitions->viewportState.sType                         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    anopolPipelineDefinitions->viewportState.viewportCount                 = 1;
    anopolPipelineDefinitions->viewportState.scissorCount                  = 1;
    anopolPipelineDefinitions->viewportState.pViewports                    = &anopolMainPipeline->viewport;
    anopolPipelineDefinitions->viewportState.pScissors                     = &anopolMainPipeline->scissor;
    anopolPipelineDefinitions->viewportState.flags                         = 0;
    anopolPipelineDefinitions->viewportState.pNext                         = nullptr;
    
    anopolPipelineDefinitions->rasterizer.sType                            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    anopolPipelineDefinitions->rasterizer.depthClampEnable                 = VK_FALSE;
    anopolPipelineDefinitions->rasterizer.rasterizerDiscardEnable          = VK_FALSE;
    anopolPipelineDefinitions->rasterizer.polygonMode                      = VK_POLYGON_MODE_FILL;
    anopolPipelineDefinitions->rasterizer.lineWidth                        = 1;
    anopolPipelineDefinitions->rasterizer.cullMode                         = VK_CULL_MODE_FRONT_BIT;
    anopolPipelineDefinitions->rasterizer.frontFace                        = VK_FRONT_FACE_CLOCKWISE;
    anopolPipelineDefinitions->rasterizer.depthBiasEnable                  = VK_FALSE;
    anopolPipelineDefinitions->rasterizer.depthBiasConstantFactor          = 0;
    anopolPipelineDefinitions->rasterizer.depthBiasClamp                   = 0;
    anopolPipelineDefinitions->rasterizer.depthBiasSlopeFactor             = 0;
    anopolPipelineDefinitions->rasterizer.flags                            = 0;
    anopolPipelineDefinitions->rasterizer.pNext                            = NULL;
    
    VkPipelineColorBlendAttachmentState color{};
    color.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color.blendEnable = VK_FALSE;
    
    anopolPipelineDefinitions->colorBlending.sType                         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    anopolPipelineDefinitions->colorBlending.logicOpEnable                 = VK_FALSE;
    anopolPipelineDefinitions->colorBlending.attachmentCount               = 1;
    anopolPipelineDefinitions->colorBlending.pAttachments                  = &color;
    anopolPipelineDefinitions->colorBlending.flags                         = 0;
    anopolPipelineDefinitions->colorBlending.pNext                         = NULL;
    
    anopolPipelineDefinitions->multisample.sType                           = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    anopolPipelineDefinitions->multisample.sampleShadingEnable             = VK_FALSE;
    anopolPipelineDefinitions->multisample.rasterizationSamples            = VK_SAMPLE_COUNT_1_BIT;
    anopolPipelineDefinitions->multisample.minSampleShading                = 1.0f;
    anopolPipelineDefinitions->multisample.alphaToCoverageEnable           = VK_FALSE;
    anopolPipelineDefinitions->multisample.alphaToOneEnable                = VK_FALSE;
    
    //------------------------------------------------------------------------------------------//
    // Uniform, Instance, Batching, Texture Descriptor Sets
    //------------------------------------------------------------------------------------------//
    
    for (size_t i = 0; i < anopol_max_frames; i++) {
        
        VkDescriptorBufferInfo uniformDescriptorBufferInfo{};
        uniformDescriptorBufferInfo.buffer = uniformBufferMemory.uniformBuffer[i];
        uniformDescriptorBufferInfo.offset = 0;
        uniformDescriptorBufferInfo.range  = sizeof(anopol::render::anopolStandardUniform);
        
        VkDescriptorBufferInfo instanceDescriptorBufferInfo{};
        instanceDescriptorBufferInfo.buffer = testAsset->GetInstances()->instanceBuffer;
        instanceDescriptorBufferInfo.offset = 0;
        instanceDescriptorBufferInfo.range  = sizeof(anopol::render::InstanceBuffer);
        
        VkDescriptorBufferInfo transformDescriptorBufferInfo{};
        transformDescriptorBufferInfo.buffer = testBatch.GetBatchFrame(i).transformBuffer;
        transformDescriptorBufferInfo.offset = 0;
        transformDescriptorBufferInfo.range  = sizeof(anopol::batch::batchIndirectTransformation) * testBatch.drawInformation.size();
        
        VkDescriptorImageInfo textureInfo{};
        textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        textureInfo.imageView   = texture.textureImageView;
        textureInfo.sampler     = texture.sampler;
                
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[0].dstSet          = ANOPOL_DESCRIPTOR_SETS->descriptorSets[i];
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[0].dstBinding      = 2;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[0].descriptorCount = 1;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[0].pBufferInfo     = &uniformDescriptorBufferInfo;
        
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[1].dstSet          = ANOPOL_DESCRIPTOR_SETS->descriptorSets[i];
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[1].dstBinding      = 1;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[1].descriptorCount = 1;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[1].pBufferInfo     = &instanceDescriptorBufferInfo;
        
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[2].dstSet          = ANOPOL_DESCRIPTOR_SETS->descriptorSets[i];
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[2].dstBinding      = 3;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[2].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[2].descriptorCount = 1;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[2].pBufferInfo     = &transformDescriptorBufferInfo;
        
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[3].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[3].dstSet          = ANOPOL_DESCRIPTOR_SETS->descriptorSets[i];
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[3].dstBinding      = 4;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[3].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[3].descriptorCount = 1;
        GLOBAL_PIPELINE_DESCRIPTOR_SETS[3].pImageInfo      = &textureInfo;
        
        vkUpdateDescriptorSets(context->device, 4, GLOBAL_PIPELINE_DESCRIPTOR_SETS.data(), 0, nullptr);
    }
    
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = anopol_max_textures;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
        
    VkDescriptorSetLayoutCreateInfo samplerLayoutInfo{};
    samplerLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    samplerLayoutInfo.bindingCount = 1;
    samplerLayoutInfo.pBindings = &samplerLayoutBinding;

    vkCreateDescriptorSetLayout(context->device, &samplerLayoutInfo, nullptr, &samplerDescriptorSetLayout);
    
    
    VkDescriptorSetAllocateInfo samplerAllocInfo{};
    samplerAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    samplerAllocInfo.descriptorPool     = ANOPOL_DESCRIPTOR_SETS->descriptorPool;
    samplerAllocInfo.descriptorSetCount = 1;
    samplerAllocInfo.pSetLayouts        = &samplerDescriptorSetLayout;

    if (vkAllocateDescriptorSets(context->device, &samplerAllocInfo, &samplerDescriptorSet) != VK_SUCCESS)
        anopol_assert("Failed to allocate sampler descriptor set!");

    // Update sampler descriptor set
    VkDescriptorImageInfo samplerImageInfo{};
    samplerImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    samplerImageInfo.imageView   = texture.textureImageView;
    samplerImageInfo.sampler     = texture.sampler;
    
    std::vector<VkDescriptorImageInfo> imageInfos(anopol_max_textures);
    for (uint32_t i = 0; i < anopol_max_textures; ++i) {
        imageInfos[i].sampler = texture.sampler;
        imageInfos[i].imageView = texture.textureImageView;
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    imageInfos[1].sampler = texture2.sampler;
    imageInfos[1].imageView = texture2.textureImageView;

    VkWriteDescriptorSet samplerWrite{};
    samplerWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    samplerWrite.dstSet          = samplerDescriptorSet;
    samplerWrite.dstBinding      = 0;
    samplerWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerWrite.descriptorCount = anopol_max_textures;
    samplerWrite.pImageInfo      = imageInfos.data();

    vkUpdateDescriptorSets(context->device, 1, &samplerWrite, 0, nullptr);
    
    
    //------------------------------------------------------------------------------------------//
    // Creating Graphics Pipeline
    //------------------------------------------------------------------------------------------//
    
    std::vector<VkDescriptorSetLayout> setLayouts = {
        GLOBAL_ANOPOL_DESCRIPTOR_SET_LAYOUT,
        samplerDescriptorSetLayout
    };
    
    
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags    = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset        = 0;
    pushConstantRange.size          = sizeof(anopol::render::anopolStandardPushConstants);
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount           = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts              = setLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount   = 1;
    pipelineLayoutInfo.pPushConstantRanges      = &pushConstantRange;
    
    if (vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, nullptr, &anopolMainPipeline->pipelineLayout) != VK_SUCCESS) anopol_assert("Failed to create pipeline");
    
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
    dependency.srcSubpass           = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass           = 0;
    dependency.srcStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask        = 0;
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
    pipelineInfo.pVertexInputState      = &anopolPipelineDefinitions->vertexInput;
    pipelineInfo.pInputAssemblyState    = &anopolPipelineDefinitions->inputAssembly;
    pipelineInfo.pViewportState         = &anopolPipelineDefinitions->viewportState;
    pipelineInfo.pRasterizationState    = &anopolPipelineDefinitions->rasterizer;
    pipelineInfo.pColorBlendState       = &anopolPipelineDefinitions->colorBlending;
    pipelineInfo.pDynamicState          = &anopolPipelineDefinitions->dynamicState;
    
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
    depthStencilInfo.depthCompareOp             = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilInfo.depthBoundsTestEnable      = VK_FALSE;
    depthStencilInfo.stencilTestEnable          = VK_FALSE;
    depthStencilInfo.minDepthBounds             = 0.0f;
    depthStencilInfo.maxDepthBounds             = 1.0f;

    pipelineInfo.layout             = anopolMainPipeline->pipelineLayout;
    pipelineInfo.renderPass         = defaultRenderpass;
    pipelineInfo.subpass            = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex  = -1;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pMultisampleState  = &anopolPipelineDefinitions->multisample;

    VkPipelineShaderStageCreateInfo shaderStages[] = { shaderModules["vert"], shaderModules["frag"] };

    pipelineInfo.pStages = shaderStages;

    if (vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &anopolMainPipeline->pipeline) != VK_SUCCESS) anopol_assert("Couldn't create VkPipeline");
    
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
    
    //anopol::lighting::fogDst = 150 * (sin(debugTime/10.0f) + 1) + 20;
    
    vkWaitForFences(context->device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    
    vkAcquireNextImageKHR(context->device, context->swapchain, UINT64_MAX, imageSemaphores[currentFrame], VK_NULL_HANDLE, &anopolPipelineDefinitions->imageIndex);
    vkResetFences(context->device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) anopol_assert("Couldn't begin command buffer");
    
    //------------------------------------------------------------------------------------------//
    // Preparing Render Pass
    //------------------------------------------------------------------------------------------//
    
    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType               = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass          = defaultRenderpass;
    renderPassBeginInfo.framebuffer         = framebuffers[anopolPipelineDefinitions->imageIndex];
    renderPassBeginInfo.renderArea.offset   = {0, 0};
    renderPassBeginInfo.renderArea.extent   = context->extent;

    VkClearValue clearColor = {{{0.4f, 0.7f, 1.0f, 1.0f}}};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
    
    anopolMainPipeline->viewport.width = static_cast<uint32_t>(context->extent.width);
    anopolMainPipeline->viewport.height = static_cast<uint32_t>(context->extent.height);
    anopolMainPipeline->viewport.x = 0.0f;

    vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, anopolMainPipeline->pipeline);
    
    VkDescriptorSet descriptorSets[] = {
        ANOPOL_DESCRIPTOR_SETS->descriptorSets[currentFrame],
        samplerDescriptorSet
    };
    
    vkCmdBindDescriptorSets(commandBuffers[currentFrame],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            anopolMainPipeline->pipelineLayout,
                            0,
                            2,
                            descriptorSets,
                            0,
                            nullptr);
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &anopolMainPipeline->viewport);
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &anopolMainPipeline->scissor);
    
    //testBatch.Cull();
        
    //------------------------------------------------------------------------------------------//
    // Camera-Renderable Collision
    //------------------------------------------------------------------------------------------//
    
    int hardwareThreads = std::thread::hardware_concurrency();
    auto& renderables = testBatch.meshCombineGroup.renderables;
    int total = renderables.size();
    uint32_t maxThreads = std::min(hardwareThreads, total);
    int chunkSize = (total + maxThreads - 1) / maxThreads;

    glm::vec3 totalPush(0.0f);
    const int maxIterations = 5;

    for (int iter = 0; iter < maxIterations; ++iter) {
        glm::vec3 accumulatedMTV(0.0f);
        float totalDepth = 0.0f;

        std::vector<std::future<std::vector<CameraAdjustment>>> futures;

        for (int i = 0; i < maxThreads; ++i) {
            int start = i * chunkSize;
            int end = std::min(start + chunkSize, total);

            futures.push_back(std::async(std::launch::async, [start, end, &renderables]() {
                std::vector<CameraAdjustment> adjustments;

                for (int j = start; j < end; ++j) {
                    auto* r = renderables[j];
                    
                    //anopol::camera::Ray ray = {anopol::camera::camera.cameraPosition, anopol::camera::camera.mouseRay};
                    //std::optional<anopol::camera::Intersection> intersection = anopol::camera::Raycast(ray, r);
                    
                    //if (intersection != std::nullopt) {
                    //    intersection->target->color = glm::vec3(1.0f, 0.0f, 0.0f);
                    //}
                    
                    if (!r->collisionEnabled) continue;
                    if (glm::distance(r->position, anopol::camera::camera.cameraPosition) > r->ComputeBoundingSphereRadius() + 2.0f) continue;
                    
                    auto col = anopol::collision::GJKCollisionWithCamera(r);
                    
                    if (col.collided && col.depth > 0.02f) {
                        glm::vec3 correctionDir = glm::normalize(col.A - col.B);
                        if (glm::dot(col.normal, correctionDir) < 0.0f)
                            col.normal = -col.normal;

                        float depth = std::max(col.depth, 0.005f);
                        adjustments.push_back({col.normal, depth});
                    }
                }

                return adjustments;
            }));
        }

        for (auto& fut : futures) {
            for (const auto& adj : fut.get()) {
                accumulatedMTV += adj.normal * adj.depth;
                totalDepth += adj.depth;
            }
        }
        if (totalDepth == 0.0f) break;

        glm::vec3 stableCorrection = glm::normalize(accumulatedMTV) * totalDepth;
        anopol::camera::camera.cameraPosition += stableCorrection;
        totalPush += stableCorrection;

        if (glm::length(totalPush) > 2.0f) break;
    }
    anopol::camera::camera.updateLookAt();
        
    if (glfwGetMouseButton(context->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isLeftMouseButtonDown) {
        anopol::render::Renderable* renderable = anopol::render::Renderable::Create();
        renderable->position = anopol::camera::camera.cameraPosition + anopol::camera::camera.mouseRay*14.0f;
        renderable->scale    = glm::vec3(5.0f, 5.0f, 5.0f);
        renderable->rotation = glm::vec3(rand()%360);
        renderable->color    = glm::vec3(rand()%255/255.0f);
                    
        testBatch.Append(renderable);
        testBatch.Combine();
        isLeftMouseButtonDown = true;
    }
    if (glfwGetMouseButton(context->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && isLeftMouseButtonDown) {
        isLeftMouseButtonDown = false;
    }
        
    uniformBufferMemory.Update(currentFrame);
    
    //------------------------------------------------------------------------------------------//
    // Rendering Batch
    //------------------------------------------------------------------------------------------//
    
    testBatch.Render(anopolMainPipeline->pipelineLayout, commandBuffers[currentFrame], defaultRenderpass, framebuffers[anopolPipelineDefinitions->imageIndex], currentFrame);
    
    //------------------------------------------------------------------------------------------//
    // Rendering Models / Instancing
    //------------------------------------------------------------------------------------------//
    for (anopol::render::Asset* a : assets) {
        
        //------------------------------------------------------------------------------------------//
        // Push Constants
        //------------------------------------------------------------------------------------------//
        
        anopol::render::anopolStandardPushConstants standardPushConstants{};
        standardPushConstants.scale             = glm::vec4(glm::vec3(0.75f), 1.0f);
        standardPushConstants.position          = glm::vec4(glm::vec3(10.0f), 1.0f);
        standardPushConstants.rotation          = glm::vec4(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
        
        glm::mat4 model = modelMatrix(standardPushConstants.position,
                                      standardPushConstants.scale,
                                      standardPushConstants.rotation);
        
        standardPushConstants.model = model;
        
        std::vector<VkBuffer> vertexBuffers = std::vector<VkBuffer>();
        
        if (a->IsInstanced()) {
            standardPushConstants.instanced = true;
        }
        standardPushConstants.physicallyBasedRendering = true;
        
        anopol::render::Asset::Mesh mesh = a->meshes[0];
        
        vertexBuffers.push_back(mesh.vertexBuffer.vertexBuffer);
        if (a->IsInstanced()) {
            vertexBuffers.push_back(a->GetInstances()->instanceBuffer);
        }
        
        std::vector<VkDeviceSize> offsets(vertexBuffers.size(), 0);
        
        vkCmdPushConstants(commandBuffers[currentFrame],
                           anopolMainPipeline->pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0,
                           sizeof(anopol::render::anopolStandardPushConstants),
                           &standardPushConstants);
        
        //------------------------------------------------------------------------------------------//
        // Rendering
        //------------------------------------------------------------------------------------------//
        
        vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, static_cast<uint32_t>(vertexBuffers.size()), vertexBuffers.data(), offsets.data());
        vkCmdBindIndexBuffer(commandBuffers[currentFrame], mesh.indexBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffers[currentFrame], static_cast<uint32_t>(mesh.indices.size()), static_cast<uint32_t>(a->IsInstanced() ? a->GetInstances()->instances.size() : 1), 0, 0, 0);
    }
    
    vkCmdEndRenderPass(commandBuffers[currentFrame]);
    
    
    //offscreen.Render(testBatch, commandBuffers[currentFrame], anopolMainPipeline->pipelineLayout, currentFrame);
    
    
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
    presentInfo.pImageIndices = &anopolPipelineDefinitions->imageIndex;

    vkQueuePresentKHR(context->presentQueue, &presentInfo);
}

void Pipeline::InitializeShadowDepthPass() {
    
}

void Pipeline::RenderScene(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t cascade = 0) {
    
}

void Pipeline::CleanUp() {
    
    //------------------------------------------------------------------------------------------//
    // Clean up and deallocating
    //------------------------------------------------------------------------------------------//
    
    for (VkFramebuffer framebuffer : framebuffers) {
        vkDestroyFramebuffer(context->device, framebuffer, nullptr);
    }
    
    anopol::ll::freeSwapchain();
    instanceBuffer->dealloc();
    
    vkDestroyPipeline(context->device, anopolMainPipeline->pipeline, nullptr);
    vkDestroyPipelineLayout(context->device, anopolMainPipeline->pipelineLayout, nullptr);
    vkDestroyRenderPass(context->device, defaultRenderpass, nullptr);
    
    vkFreeCommandBuffers(context->device, ll::commandPool, anopol_max_frames, commandBuffers.data());
    
    testBatch.Dealloc();
    offscreen.Free();
    
    for (anopol::render::Renderable* renderable : debugRenderables) {
        renderable->vertexBuffer.dealloc();
        renderable->indexBuffer.dealloc();
    }
    
    for (anopol::render::Asset* asset : assets) {
        for (anopol::render::Asset::Mesh mesh : asset->meshes) {
            mesh.vertexBuffer.dealloc();
            mesh.indexBuffer.dealloc();
        }
        if(asset->IsInstanced()) asset->GetInstances()->dealloc();
    }
    uniformBufferMemory.dealloc();
    texture.Dealloc();
    texture2.Dealloc();
    
    vkDestroyDescriptorSetLayout(context->device, samplerDescriptorSetLayout, nullptr);
    
    for (size_t i = 0; i < anopol_max_frames; i++) {
        vkDestroySemaphore(context->device, renderSemaphores[i], nullptr);
        vkDestroySemaphore(context->device, imageSemaphores[i], nullptr);
        vkDestroyFence(context->device, inFlightFences[i], nullptr);
    }
    
    free(anopolPipelineDefinitions);
    free(anopolMainPipeline);
}

}

#endif /* pipeline_h */
