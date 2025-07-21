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
    VkBuffer indexBuffer                = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory    = VK_NULL_HANDLE;
    VkDeviceSize bufferSize             = 0;
    std::vector<uint32_t> indices;
    
    void alloc(std::vector<uint32_t> indices);
    void dealloc();
};

void IndexBuffer::alloc(std::vector<uint32_t> indices) {
    
    this->indices = indices;
    
    VkBuffer oldBuffer = indexBuffer;
    VkDeviceMemory oldMemory = indexBufferMemory;

    VkFence copyFence;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;
    vkCreateFence(context->device, &fenceInfo, nullptr, &copyFence);

    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer staging;
    VkDeviceMemory stagingMemory;
    anopol::ll::createBuffer(bufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             staging, stagingMemory);

    void* data;
    vkMapMemory(context->device, stagingMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(context->device, stagingMemory);

    anopol::ll::createBuffer(bufferSize,
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             indexBuffer,
                             indexBufferMemory);

    anopol::ll::memCopyBuffer(staging, indexBuffer, bufferSize, copyFence);

    vkWaitForFences(context->device, 1, &copyFence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(context->device, copyFence, nullptr);

    vkDestroyBuffer(context->device, staging, nullptr);
    vkFreeMemory(context->device, stagingMemory, nullptr);

    if (oldBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(context->device, oldBuffer, nullptr);
    }
    if (oldMemory != VK_NULL_HANDLE) {
        vkFreeMemory(context->device, oldMemory, nullptr);
    }
}

void IndexBuffer::dealloc() {
    vkDestroyBuffer(context->device, indexBuffer, nullptr);
    vkFreeMemory(context->device, indexBufferMemory, nullptr);
}

}

#endif /* index_buffer_h */
