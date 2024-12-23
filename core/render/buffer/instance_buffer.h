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
    
    void alloc(std::vector<glm::vec3> position, std::vector<glm::vec3> scale, std::vector<glm::vec3> rotation, std::vector<glm::vec3> color);
    void dealloc();
    
    void appendInstance(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation, glm::vec3 color);

private:
    void copyMemory();
};

void InstanceBuffer::alloc(std::vector<glm::vec3> position, std::vector<glm::vec3> scale, std::vector<glm::vec3> rotation, std::vector<glm::vec3> color) {
    
    for (int i = 0; i < position.size(); i++) {
        
        instanceProperties instance{};
        instance.model = modelMatrix(position[i], scale[i], rotation[i]);
        instance.color = glm::vec4(color[i], 1.0f);
        
        instances.push_back(instance);
    }
    dealloc();
    copyMemory();
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
    
    dealloc();
    copyMemory();
}


void InstanceBuffer::copyMemory() {
    
    VkDeviceSize bufferSize = sizeof(instanceProperties) * instances.size();
    
    anopol::ll::createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                             instanceBuffer, instanceBufferMemory);
    
    vkMapMemory(context->device, instanceBufferMemory, 0, bufferSize, 0, &instanceBufferMemoryMapped);
    memcpy(instanceBufferMemoryMapped, instances.data(), sizeof(instanceProperties) * instances.size());
    vkUnmapMemory(context->device, instanceBufferMemory);
}

}


#endif /* instance_buffer_h */
