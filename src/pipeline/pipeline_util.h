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

pipelineDefinitions GeneratePipelineDefinitions(PipelineType type, const VkViewport* viewport, const VkRect2D* scissor) {
    
    pipelineDefinitions definitions{};
    
    
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    definitions.dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};;
    
    VkVertexInputBindingDescription vertexBindingDescription = anopol::render::Vertex::getBindingDescription();
    VkVertexInputBindingDescription instanceBindingDescription = anopol::render::InstanceBuffer::GetBindingDescription();
    
    
    if (type == InstanceAndBatching) {
        definitions.attributes = {
            VkVertexInputAttributeDescription {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(anopol::render::Vertex, vertex)},
            VkVertexInputAttributeDescription {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(anopol::render::Vertex, normal)},
            VkVertexInputAttributeDescription {2, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(anopol::render::Vertex, uv)},
            VkVertexInputAttributeDescription {3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow0)},
            VkVertexInputAttributeDescription {4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow1)},
            VkVertexInputAttributeDescription {5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow2)},
            VkVertexInputAttributeDescription {6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow3)},
        };
        
        definitions.bindings = {
            vertexBindingDescription,
            instanceBindingDescription
        };
    }
    else if (type == Instance) {
        definitions.attributes = {
            VkVertexInputAttributeDescription {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow0)},
            VkVertexInputAttributeDescription {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow1)},
            VkVertexInputAttributeDescription {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow2)},
            VkVertexInputAttributeDescription {3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(anopol::render::instanceProperties, modelRow3)}
        };
        definitions.bindings = {
            instanceBindingDescription
        };
    }
    else if (type == Batching) {
        definitions.attributes = {
            VkVertexInputAttributeDescription {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(anopol::render::Vertex, vertex)},
            VkVertexInputAttributeDescription {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(anopol::render::Vertex, normal)},
            VkVertexInputAttributeDescription {2, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(anopol::render::Vertex, uv)},
        };
        definitions.bindings = {
            vertexBindingDescription,
        };
    }
    
    definitions.viewport = *viewport;
    definitions.scissor = *scissor;
    
    definitions.dynamicState.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    definitions.dynamicState.dynamicStateCount  = (uint32_t)definitions.dynamicStates.size();
    definitions.dynamicState.pDynamicStates     = definitions.dynamicStates.data();
    definitions.dynamicState.flags              = 0;
    definitions.dynamicState.pNext              = nullptr;

    definitions.vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    definitions.vertexInput.vertexBindingDescriptionCount   = (uint32_t)definitions.bindings.size();
    definitions.vertexInput.pVertexBindingDescriptions      = definitions.bindings.data();
    definitions.vertexInput.vertexAttributeDescriptionCount = (uint32_t)definitions.attributes.size();
    definitions.vertexInput.pVertexAttributeDescriptions    = definitions.attributes.data();
    definitions.vertexInput.flags                           = 0;
    definitions.vertexInput.pNext                           = nullptr;
    
    definitions.inputAssembly.sType                         = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    definitions.inputAssembly.topology                      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    definitions.inputAssembly.primitiveRestartEnable        = VK_FALSE;
    definitions.inputAssembly.flags                         = 0;
    definitions.inputAssembly.pNext                         = nullptr;
    
    definitions.viewportState.sType                         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    definitions.viewportState.viewportCount                 = 1;
    definitions.viewportState.scissorCount                  = 1;
    definitions.viewportState.pViewports                    = viewport;
    definitions.viewportState.pScissors                     = scissor;
    definitions.viewportState.flags                         = 0;
    definitions.viewportState.pNext                         = nullptr;
    
    definitions.rasterizer.sType                            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    definitions.rasterizer.depthClampEnable                 = VK_FALSE;
    definitions.rasterizer.rasterizerDiscardEnable          = VK_FALSE;
    definitions.rasterizer.polygonMode                      = VK_POLYGON_MODE_FILL;
    definitions.rasterizer.lineWidth                        = 1;
    definitions.rasterizer.cullMode                         = VK_CULL_MODE_FRONT_BIT;
    definitions.rasterizer.frontFace                        = VK_FRONT_FACE_CLOCKWISE;
    definitions.rasterizer.depthBiasEnable                  = VK_FALSE;
    definitions.rasterizer.depthBiasConstantFactor          = 0;
    definitions.rasterizer.depthBiasClamp                   = 0;
    definitions.rasterizer.depthBiasSlopeFactor             = 0;
    definitions.rasterizer.flags                            = 0;
    definitions.rasterizer.pNext                            = NULL;
    
    definitions.color.colorWriteMask                        = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    definitions.color.blendEnable                           = VK_FALSE;
    
    definitions.colorBlending.sType                         = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    definitions.colorBlending.logicOpEnable                 = VK_FALSE;
    definitions.colorBlending.attachmentCount               = 1;
    definitions.colorBlending.pAttachments                  = &definitions.color;
    definitions.colorBlending.flags                         = 0;
    definitions.colorBlending.pNext                         = NULL;
    
    definitions.multisample.sType                           = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    definitions.multisample.sampleShadingEnable             = VK_FALSE;
    definitions.multisample.rasterizationSamples            = VK_SAMPLE_COUNT_1_BIT;
    definitions.multisample.minSampleShading                = 1.0f;
    definitions.multisample.alphaToCoverageEnable           = VK_FALSE;
    definitions.multisample.alphaToOneEnable                = VK_FALSE;
    
    return definitions;
}
    
}

#endif /* pipeline_util_h */
