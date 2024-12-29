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
    glm::vec3 cameraPosition;
    float t;
};
anopolStandardUniform asu{};

class UniformBuffer {
public:
    std::vector<VkBuffer>       uniformBuffer = std::vector<VkBuffer>();
    std::vector<VkDeviceMemory> uniformBufferMemory;
    std::vector<void*>          uniformBufferMapped;
    
    static UniformBuffer Create();
    void Update(int currentFrame);
    void Model(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, int currentFrame);
    void dealloc();
};

UniformBuffer UniformBuffer::Create() {
    
    VkDeviceSize bufferSize = sizeof(anopolStandardUniform);
    UniformBuffer buffer = UniformBuffer();
    
    buffer.uniformBuffer.resize(anopol_max_frames);
    buffer.uniformBufferMemory.resize(anopol_max_frames);
    buffer.uniformBufferMapped.resize(anopol_max_frames);
    
    for (size_t i = 0; i < anopol_max_frames; i++) {
        anopol::ll::createBuffer(bufferSize,
                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 buffer.uniformBuffer[i], buffer.uniformBufferMemory[i]);
        
        vkMapMemory(context->device, buffer.uniformBufferMemory[i], 0, bufferSize, 0, &buffer.uniformBufferMapped[i]);
    }
    
    return buffer;
}

void UniformBuffer::Update(int currentFrame) {
        
    asu.t = debugTime;
    asu.projection = anopol::camera::camera.cameraProjection;
    asu.lookAt = anopol::camera::camera.cameraLookAt;
    asu.cameraPosition = anopol::camera::camera.cameraPosition;

    memcpy(uniformBufferMapped[currentFrame], &asu, sizeof(asu));
    
}

void UniformBuffer::dealloc() {
    
    for (VkBuffer buffer : uniformBuffer) {
        vkDestroyBuffer(context->device, buffer, nullptr);
    }
    for (VkDeviceMemory deviceMemory : uniformBufferMemory) {
        vkFreeMemory(context->device, deviceMemory, nullptr);
    }
}

}

#endif /* uniform_buffer_h */
