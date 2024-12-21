//
//  shadow.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-12-21.
//

#ifndef shadow_h
#define shadow_h

namespace anopol::structs {

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

}

#endif /* shadow_h */
