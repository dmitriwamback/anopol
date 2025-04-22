//
//  pipeline_util.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-04-21.
//

#ifndef pipeline_util_h
#define pipeline_util_h

namespace anopol::pipeline {

struct pipeline {
    VkPipelineLayout    pipelineLayout;
    VkRect2D            scissor{};
    VkViewport          viewport{};
    VkRenderPass        renderPass;
    VkPipeline          pipeline;
};

struct pipelineDefinitions {
    
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

}

#endif /* pipeline_util_h */
