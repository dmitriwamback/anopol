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
    
    float t = 0;
};

class UniformBuffer {
public:
    std::vector<VkBuffer>       uniformBuffer = std::vector<VkBuffer>();
    std::vector<VkDeviceMemory> uniformBufferMemory;
    std::vector<void*>          uniformBufferMapped;
    
    float debugTime = 0;
    
    static UniformBuffer Create();
    void Update(int currentFrame);
};
UniformBuffer UniformBuffer::Create() {
    
    VkDeviceSize bufferSize = sizeof(anopolStandardUniform);
    UniformBuffer buffer = UniformBuffer();
    
    buffer.uniformBuffer.resize(anopol_max_frames);
    buffer.uniformBufferMemory.resize(anopol_max_frames);
    buffer.uniformBufferMapped.resize(anopol_max_frames);
    
    for (size_t i = 0; i < anopol_max_frames; i++) {
        anopol::ll::createBuffer(bufferSize,
                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 buffer.uniformBuffer[i], buffer.uniformBufferMemory[i]);
        
        vkMapMemory(context->device, buffer.uniformBufferMemory[i], 0, bufferSize, 0, &buffer.uniformBufferMapped[i]);
    }
    
    return buffer;
}
void UniformBuffer::Update(int currentFrame) {
    anopolStandardUniform asu{};
    debugTime += 0.1f;
    asu.t = debugTime;
        
    memcpy(uniformBufferMapped[currentFrame], &asu, sizeof(asu));
}

}

#endif /* uniform_buffer_h */
