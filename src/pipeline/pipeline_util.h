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

struct pipelineConfigurations {
    
    VkPipelineDynamicStateCreateInfo            dynamicState{};
    VkPipelineVertexInputStateCreateInfo        vertexInput{};
    VkPipelineInputAssemblyStateCreateInfo      inputAssembly{};
    VkPipelineRasterizationStateCreateInfo      rasterizer{};
    VkPipelineViewportStateCreateInfo           viewportState{};
    VkPipelineColorBlendStateCreateInfo         colorBlending{};
    VkPipelineMultisampleStateCreateInfo        multisample{};
    VkPipelineColorBlendAttachmentState         color{};
    
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;
    std::vector<VkDynamicState> dynamicStates;
    
    VkViewport viewport;
    VkRect2D scissor;
    
    uint32_t imageIndex;
};

enum PipelineType {
    Instance = 0, Batching = 1, InstanceAndBatching = 2
};

pipelineConfigurations CreatePipelineConfigurations(PipelineType type, const VkViewport* viewport, const VkRect2D* scissor) {
    
    pipelineConfigurations config{};
    
    
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    config.dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};;
    
    VkVertexInputBindingDescription vertexBindingDescription = anopol::render::Vertex::getBindingDescription();
    VkVertexInputBindingDescription instanceBindingDescription = anopol::render::InstanceBuffer::GetBindingDescription();
    
    
    if (type == InstanceAndBatching) {
        config.attributes = {
            VkVertexInputAttributeDescription {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(anopol::render::Vertex, vertex)},
            VkVertexInputAttributeDescription {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(anopol::render::Vertex, normal)},
            VkVertexInputAttributeDescription {2, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(anopol::render::Vertex, uv)},
            VkVertexInputAttributeDescription {3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow0)},
            VkVertexInputAttributeDescription {4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow1)},
            VkVertexInputAttributeDescription {5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow2)},
            VkVertexInputAttributeDescription {6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow3)},
        };
        
        config.bindings = {
            vertexBindingDescription,
            instanceBindingDescription
        };
    }
    else if (type == Instance) {
        config.attributes = {
            VkVertexInputAttributeDescription {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow0)},
            VkVertexInputAttributeDescription {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow1)},
            VkVertexInputAttributeDescription {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow2)},
            VkVertexInputAttributeDescription {3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow3)}
        };
        config.bindings = {
            instanceBindingDescription
        };
    }
    else if (type == Batching) {
        config.attributes = {
            VkVertexInputAttributeDescription {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(anopol::render::Vertex, vertex)},
            VkVertexInputAttributeDescription {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(anopol::render::Vertex, normal)},
            VkVertexInputAttributeDescription {2, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(anopol::render::Vertex, uv)},
        };
        config.bindings = {
            vertexBindingDescription,
        };
    }
    
    config.viewport = *viewport;
    config.scissor = *scissor;
    
    config.dynamicState.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    config.dynamicState.dynamicStateCount  = (uint32_t)config.dynamicStates.size();
    config.dynamicState.pDynamicStates     = config.dynamicStates.data();
    config.dynamicState.flags              = 0;
    config.dynamicState.pNext              = nullptr;

    config.vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    config.vertexInput.vertexBindingDescriptionCount   = (uint32_t)config.bindings.size();
    config.vertexInput.pVertexBindingDescriptions      = config.bindings.data();
    config.vertexInput.vertexAttributeDescriptionCount = (uint32_t)config.attributes.size();
    config.vertexInput.pVertexAttributeDescriptions    = config.attributes.data();
    config.vertexInput.flags                           = 0;
    config.vertexInput.pNext                           = nullptr;
    
    config.inputAssembly.sType                         = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config.inputAssembly.topology                      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.inputAssembly.primitiveRestartEnable        = VK_FALSE;
    config.inputAssembly.flags                         = 0;
    config.inputAssembly.pNext                         = nullptr;
    
    config.viewportState.sType                         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    config.viewportState.viewportCount                 = 1;
    config.viewportState.scissorCount                  = 1;
    config.viewportState.pViewports                    = viewport;
    config.viewportState.pScissors                     = scissor;
    config.viewportState.flags                         = 0;
    config.viewportState.pNext                         = nullptr;
    
    config.rasterizer.sType                            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config.rasterizer.depthClampEnable                 = VK_FALSE;
    config.rasterizer.rasterizerDiscardEnable          = VK_FALSE;
    config.rasterizer.polygonMode                      = VK_POLYGON_MODE_FILL;
    config.rasterizer.lineWidth                        = 1;
    config.rasterizer.cullMode                         = VK_CULL_MODE_FRONT_BIT;
    config.rasterizer.frontFace                        = VK_FRONT_FACE_CLOCKWISE;
    config.rasterizer.depthBiasEnable                  = VK_FALSE;
    config.rasterizer.depthBiasConstantFactor          = 0;
    config.rasterizer.depthBiasClamp                   = 0;
    config.rasterizer.depthBiasSlopeFactor             = 0;
    config.rasterizer.flags                            = 0;
    config.rasterizer.pNext                            = NULL;
    
    config.color.colorWriteMask                        = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    config.color.blendEnable                           = VK_FALSE;
    
    config.colorBlending.sType                         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    config.colorBlending.logicOpEnable                 = VK_FALSE;
    config.colorBlending.attachmentCount               = 1;
    config.colorBlending.pAttachments                  = &config.color;
    config.colorBlending.flags                         = 0;
    config.colorBlending.pNext                         = NULL;
    
    config.multisample.sType                           = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config.multisample.sampleShadingEnable             = VK_FALSE;
    config.multisample.rasterizationSamples            = VK_SAMPLE_COUNT_1_BIT;
    config.multisample.minSampleShading                = 1.0f;
    config.multisample.alphaToCoverageEnable           = VK_FALSE;
    config.multisample.alphaToOneEnable                = VK_FALSE;
    
    return config;
}
    
}

#endif /* pipeline_util_h */
