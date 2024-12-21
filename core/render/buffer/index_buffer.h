//
//  index_buffer.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-29.
//

#ifndef index_buffer_h
#define index_buffer_h

namespace anopol::render {

class IndexBuffer {
public:
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    
    void alloc(std::vector<uint32_t> indices);
    void dealloc();
};

void IndexBuffer::alloc(std::vector<uint32_t> indices) {
    VkDeviceSize bufferSize = indices.size() * sizeof(indices[0]);
    
    VkBuffer staging;
    VkDeviceMemory stagingMemory;
    
    anopol::ll::createBuffer(bufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             staging, stagingMemory);
    
    void* data;
    vkMapMemory(context->device, stagingMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), bufferSize);
    vkUnmapMemory(context->device, stagingMemory);
    
    anopol::ll::createBuffer(bufferSize,
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             indexBuffer, indexBufferMemory);
    
    anopol::ll::memCopyBuffer(staging, indexBuffer, bufferSize);
    
    vkDestroyBuffer(context->device, staging, nullptr);
    vkFreeMemory(context->device, stagingMemory, nullptr);
}

void IndexBuffer::dealloc() {
    vkDestroyBuffer(context->device, indexBuffer, nullptr);
    vkFreeMemory(context->device, indexBufferMemory, nullptr);
}

}

#endif /* index_buffer_h */
