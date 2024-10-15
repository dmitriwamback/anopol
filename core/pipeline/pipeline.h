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
    struct layout {
        VkPipelineLayout    pipelineLayout;
        VkRenderPass        renderpass;
        VkRect2D            scissor{};
        VkViewport          viewport;
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
    
    std::vector<VkCommandBuffer> commandBuffers     = std::vector<VkCommandBuffer>();
    std::vector<VkSemaphore>     imageSemaphores    = std::vector<VkSemaphore>(),
                                 renderSemphores    = std::vector<VkSemaphore>();
    
    std::vector<VkFence> inFlightFences             = std::vector<VkFence>();
    VkCommandPool commandPool;
    
    layout* pipelineLayout;
    pipeline* pipeline;
    
    std::map<std::string, Scene*> scenes;
    std::map<std::string, VkPipelineShaderStageCreateInfo> shaderModules;
    
    std::vector<Vertex> testVertices;
    
        
    static Pipeline CreatePipeline(std::string shaderFolder);
    
    static VkShaderModule CreateShaderModule(std::vector<char> shaderSource);
    static std::vector<char> LoadShaderContent(std::string path);
    
    void Bind(std::string name);
    void AddScene(Scene* scene, std::string name);
    
private:
    void InitializePipeline();
    void CreateSynchronizedObjects();
    void CreateCommandBiffers();
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
    pipeline.testVertices = {
        {{ 0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.2f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    };
    
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

}

#endif /* pipeline_h */
