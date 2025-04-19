//
//  batch.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-04-13.
//

#ifndef batch_h
#define batch_h

namespace anopol::batch {

class Batch {
public:
    std::vector<anopol::render::Vertex> batchVertices;
    std::vector<uint32_t> batchIndices;
    std::vector<batchDrawInformation> drawInformation;
    std::vector<glm::mat4> transformations;
    
    anopol::render::VertexBuffer vertexBuffer;
    anopol::render::IndexBuffer indexBuffer;
    MeshCombineGroup meshCombineGroup;
    
    VkBuffer drawCommandBuffer, transformBuffer;
    VkDeviceMemory drawCommandBufferMemory, transformBufferMemory;
    
    static Batch Create();
    void Append(anopol::render::Renderable* renderable);
    void Append(anopol::render::Asset* asset);
    void Append(std::vector<anopol::render::Renderable*> renderables);
    void Append(std::vector<anopol::render::Asset*> assets);
    void Dealloc();
    void Combine();
    
private:
};

Batch Batch::Create() {
    
    Batch batch = Batch();
    
    batch.vertexBuffer = anopol::render::VertexBuffer();
    batch.indexBuffer = anopol::render::IndexBuffer();
    
    batch.meshCombineGroup = MeshCombineGroup();
    
    return batch;
}

void Batch::Append(anopol::render::Renderable* renderable) {
    meshCombineGroup.Append(renderable);
}

void Batch::Append(anopol::render::Asset* asset) {
    meshCombineGroup.Append(asset);
}

void Batch::Append(std::vector<anopol::render::Renderable*> renderables) {
    meshCombineGroup.Append(renderables);
}

void Batch::Append(std::vector<anopol::render::Asset*> assets) {
    meshCombineGroup.Append(assets);
}

void Batch::Combine() {
    
    drawInformation.clear();
    batchVertices.clear();
    batchIndices.clear();
    transformations.clear();
    
    uint32_t vertexOffset = 0, indexOffset = 0, object = 0;
    
    for (anopol::render::Renderable* renderable : meshCombineGroup.renderables) {
        
        batchDrawInformation drawInfo;
        
        transformations.push_back(modelMatrix(renderable->position, renderable->scale, renderable->rotation));
        
        if (renderable->isIndexed == false) {
            drawInfo.drawType = nonIndexed;
            drawInfo.firstVertex = vertexOffset;
            drawInfo.vertexCount = static_cast<uint32_t>(renderable->vertices.size());
            drawInfo.object = object++;
        }
        else {
            drawInfo.drawType = indexed;
            for (uint32_t index : renderable->indices) {
                batchIndices.push_back(index + vertexOffset);
            }
            drawInfo.firstIndex = indexOffset;
            drawInfo.indexCount = static_cast<uint32_t>(renderable->indices.size());
            drawInfo.vertexOffset = vertexOffset;
            drawInfo.object = object++;
            
            indexOffset += renderable->indices.size();
        }
        vertexOffset += renderable->vertices.size();
        
        drawInformation.push_back(drawInfo);
        for (anopol::render::Vertex vertex : renderable->vertices) {
            batchVertices.push_back(vertex);
        }
    }
    /*
    for (anopol::render::Asset* asset : meshCombineGroup.assets) {
        for (anopol::render::Asset::Mesh mesh : asset->meshes) {
            batchDrawInformation drawInfo;
            drawInfo.drawType = indexed;
            
            drawInformation.push_back(drawInfo);
            for (anopol::render::Vertex vertex : mesh.vertices) {
                batchVertices.push_back(vertex);
            }
        }
    }
     */
    
    //------------------------------------------------------------------------------------------//
    // Allocating Vertex Buffer
    //------------------------------------------------------------------------------------------//
    
    {
        vertexBuffer.alloc(batchVertices);
        if (batchIndices.size() > 0) indexBuffer.alloc(batchIndices);
        
        std::vector<VkDrawIndirectCommand> drawCommands;
        
        for (anopol::batch::batchDrawInformation drawInfo : drawInformation) {
            
            VkDrawIndirectCommand command{};
            command.vertexCount = drawInfo.vertexCount;
            command.instanceCount = 1;
            command.firstVertex = drawInfo.firstVertex;
            command.firstInstance = drawInfo.object;
            
            drawCommands.push_back(command);
        }
        
        VkDeviceSize bufferSize = sizeof(VkDrawIndirectCommand) * drawCommands.size();
        
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        vkCreateBuffer(context->device, &bufferInfo, nullptr, &drawCommandBuffer);
        
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(context->device, drawCommandBuffer, &memRequirements);
        
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = anopol::ll::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        vkAllocateMemory(context->device, &allocInfo, nullptr, &drawCommandBufferMemory);
        vkBindBufferMemory(context->device, drawCommandBuffer, drawCommandBufferMemory, 0);
        
        VkBuffer staging;
        VkDeviceMemory stagingMemory;
        
        anopol::ll::createBuffer(bufferSize,
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 staging, stagingMemory);
        
        void* data;
        vkMapMemory(context->device, stagingMemory, 0, bufferSize, 0, &data);
        memcpy(data, drawCommands.data(), (size_t) bufferSize);
        vkUnmapMemory(context->device, stagingMemory);
        
        VkCommandBuffer commandBuffer = anopol::ll::beginSingleCommandBuffer();
        
        VkBufferCopy copyRegion{};
        copyRegion.size = bufferSize;
        vkCmdCopyBuffer(commandBuffer, staging, drawCommandBuffer, 1, &copyRegion);
        
        anopol::ll::endSingleCommandBuffer(commandBuffer);
        
        vkDestroyBuffer(context->device, staging, nullptr);
        vkFreeMemory(context->device, stagingMemory, nullptr);
    }
    
    //------------------------------------------------------------------------------------------//
    // Allocating transformations
    //------------------------------------------------------------------------------------------//
    {
        VkDeviceSize bufferSize = sizeof(glm::mat4) * transformations.size();
        
        anopol::ll::createBuffer(bufferSize,
                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 transformBuffer,
                                 transformBufferMemory);
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        anopol::ll::createBuffer(bufferSize,
                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 stagingBuffer,
                                 stagingBufferMemory);
        
        void* data;
        vkMapMemory(context->device, stagingBufferMemory, 0, sizeof(glm::mat4) * transformations.size(), 0, &data);
        memcpy(data, transformations.data(), sizeof(glm::mat4) * transformations.size());
        vkUnmapMemory(context->device, stagingBufferMemory);
        
        VkCommandBuffer commandBuffer = anopol::ll::beginSingleCommandBuffer();

        VkBufferCopy copyRegion{};
        copyRegion.size = sizeof(glm::mat4) * transformations.size();
        vkCmdCopyBuffer(commandBuffer, stagingBuffer, transformBuffer, 1, &copyRegion);

        anopol::ll::endSingleCommandBuffer(commandBuffer);

        vkDestroyBuffer(context->device, stagingBuffer, nullptr);
        vkFreeMemory(context->device, stagingBufferMemory, nullptr);
    }
}

void Batch::Dealloc() {
    for (anopol::render::Renderable* renderable : meshCombineGroup.renderables) {
        renderable->vertexBuffer.dealloc();
        renderable->indexBuffer.dealloc();
    }
    for (anopol::render::Asset* asset : meshCombineGroup.assets) {
        for (anopol::render::Asset::Mesh mesh : asset->meshes) {
            mesh.vertexBuffer.dealloc();
            mesh.indexBuffer.dealloc();
        }
    }
    vertexBuffer.dealloc();
    indexBuffer.dealloc();
    
    vkDestroyBuffer(context->device, drawCommandBuffer, nullptr);
    vkFreeMemory(context->device, drawCommandBufferMemory, nullptr);
    
    vkDestroyBuffer(context->device, transformBuffer, nullptr);
    vkFreeMemory(context->device, transformBufferMemory, nullptr);
}

}

#endif /* batch_h */
