//
//  vertex.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-14.
//

#ifndef vertex_h
#define vertex_h

namespace anopol::render {
struct Vertex {
    
    glm::vec3 vertex;
    glm::vec3 normal;
    glm::vec2 uv;
    
    static VkVertexInputBindingDescription getBindingDescription() {
        
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        return binding;
    }
    
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription() {
        
        std::array<VkVertexInputAttributeDescription, 3> attributes{};
        
        attributes[0].binding   = 0;
        attributes[0].location  = 0;
        attributes[0].format    = VK_FORMAT_R32G32B32_SFLOAT;
        attributes[0].offset    = offsetof(Vertex, vertex);
        
        attributes[1].binding   = 0;
        attributes[1].location  = 1;
        attributes[1].format    = VK_FORMAT_R32G32B32_SFLOAT;
        attributes[1].offset    = offsetof(Vertex, normal);
        
        attributes[2].binding   = 0;
        attributes[2].location  = 2;
        attributes[2].format    = VK_FORMAT_R32G32_SFLOAT;
        attributes[2].offset    = offsetof(Vertex, uv);
        
        return attributes;
    }
};
}

#endif /* vertex_h */
