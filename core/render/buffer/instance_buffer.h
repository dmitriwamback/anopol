//
//  instance_buffer.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-12-22.
//

#ifndef instance_buffer_h
#define instance_buffer_h

namespace anopol::render {

struct instanceProperties {
    glm::mat4 model;
    glm::vec4 color;
};

class InstanceBuffer {
public:
    VkBuffer instanceBuffer;
    VkDeviceMemory instanceBufferMemory;
    void* instanceBufferMemoryMapped;
    std::vector<instanceProperties> instances{};
    
    void alloc(size_t initialSize);
    void dealloc();
    
    void appendInstance(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, glm::vec3 color);
    void allocInstances();

private:
    size_t maxInstances;
    void resize();
    void flush();
};

void InstanceBuffer::alloc(size_t initialSize) {
    
    maxInstances = initialSize;
    VkDeviceSize bufferSize = sizeof(instanceProperties) * maxInstances;
    
    anopol::ll::createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, instanceBuffer, instanceBufferMemory);
    
    vkMapMemory(context->device, instanceBufferMemory, 0, VK_WHOLE_SIZE, 0, &instanceBufferMemoryMapped);
}

void InstanceBuffer::dealloc() {
    vkDestroyBuffer(context->device, instanceBuffer, nullptr);
    vkFreeMemory(context->device, instanceBufferMemory, nullptr);
}

void InstanceBuffer::appendInstance(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, glm::vec3 color) {
    
    instanceProperties instance{};
    instance.model = modelMatrix(position, scale, rotation);
    instance.color = glm::vec4(color, 1.0f);
    
    instances.push_back(instance);
    
    if (instances.size() > maxInstances) {
        resize();
    }
}

void InstanceBuffer::allocInstances() {
    memcpy(instanceBufferMemoryMapped, instances.data(), sizeof(instanceProperties) * instances.size());
    flush();
}

void InstanceBuffer::resize() {
    
    maxInstances *= 2;
    VkDeviceSize bufferSize = sizeof(instanceProperties) * maxInstances;
    
    VkBuffer newBuffer;
    VkDeviceMemory newMemory;
    
    anopol::ll::createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             newBuffer, newMemory);
    
    void* newMemoryMapped;
    vkMapMemory(context->device, newMemory, 0, VK_WHOLE_SIZE, 0, &newMemoryMapped);
    
    memcpy(newMemoryMapped, instances.data(), sizeof(instanceProperties) * instances.size());
    
    dealloc();
    
    instanceBuffer              = newBuffer;
    instanceBufferMemory        = newMemory;
    instanceBufferMemoryMapped  = newMemoryMapped;
}

void InstanceBuffer::flush() {
    
    VkMappedMemoryRange range{};
    range.sType     = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.memory    = instanceBufferMemory;
    range.offset    = 0;
    range.size      = VK_WHOLE_SIZE;
    
    vkFlushMappedMemoryRanges(context->device, 1, &range);
}

}


#endif /* instance_buffer_h */
