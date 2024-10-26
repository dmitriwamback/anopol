//
//  uniform_buffer.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-26.
//

#ifndef uniform_buffer_h
#define uniform_buffer_h

namespace anopol::render {

struct anopolStandardUniform {
    glm::mat4 projection;
    glm::mat4 lookAt;
    glm::mat4 model;
};

class UniformBuffer {
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    
    
};

}

#endif /* uniform_buffer_h */
