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
    
    void alloc(std::vector<Vertex> vertices) {
        
        VkBufferCreateInfo vertexBufferCreateInfo{};
        vertexBufferCreateInfo.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vertexBufferCreateInfo.size         = sizeof(vertices[0]) * vertices.size();
        vertexBufferCreateInfo.usage        = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vertexBufferCreateInfo.sharingMode  = VK_SHARING_MODE_EXCLUSIVE;
        
        if (vkCreateBuffer(context->device, &vertexBufferCreateInfo, nullptr, &vertexBuffer) != VK_SUCCESS) anopol_assert("Failed to create vertex buffer");
        
        VkMemoryRequirements mem;
        VkMemoryAllocateInfo memoryAllocationInfo{};
        
        vkGetBufferMemoryRequirements(context->device, vertexBuffer, &mem);
        
        memoryAllocationInfo.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocationInfo.allocationSize     = mem.size;
        memoryAllocationInfo.memoryTypeIndex    = anopol::ll::findMemoryType(mem.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        if (vkAllocateMemory(context->device, &memoryAllocationInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) anopol_assert("Failed to allocate memory");
        
        vkBindBufferMemory(context->device, vertexBuffer, vertexBufferMemory, 0);
        
        void* data;
        vkMapMemory(context->device, vertexBufferMemory, 0, vertexBufferCreateInfo.size, 0, &data);
        
        memcpy(data, vertices.data(), (size_t)vertexBufferCreateInfo.size);
        vkUnmapMemory(context->device, vertexBufferMemory);
    }
};

}

#endif /* vertex_buffer_h */
