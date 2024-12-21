//
//  vertex_buffer.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-20.
//

#ifndef vertex_buffer_h
#define vertex_buffer_h

namespace anopol::render {

class VertexBuffer {
public:
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    
    void alloc(std::vector<Vertex> vertices);
    void dealloc();
};

void VertexBuffer::alloc(std::vector<Vertex> vertices) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    
    VkBuffer staging;
    VkDeviceMemory stagingMemory;
    
    anopol::ll::createBuffer(bufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             staging, stagingMemory);
    
    void* data;
    vkMapMemory(context->device, stagingMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(context->device, stagingMemory);
    
    anopol::ll::createBuffer(bufferSize,
                             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             vertexBuffer,
                             vertexBufferMemory);
    
    anopol::ll::memCopyBuffer(staging, vertexBuffer, bufferSize);
    
    vkDestroyBuffer(context->device, staging, nullptr);
    vkFreeMemory(context->device, stagingMemory, nullptr);
}

void VertexBuffer::dealloc() {
    
    vkDestroyBuffer(context->device, vertexBuffer, nullptr);
    vkFreeMemory(context->device, vertexBufferMemory, nullptr);
}

}

#endif /* vertex_buffer_h */
