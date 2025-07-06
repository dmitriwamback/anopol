//
//  batch.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-04-13.
//

#ifndef batch_h
#define batch_h

#define max_batch_draw_indirect_size static_cast<int>(2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2)      // 2^18 = 262144
#define max_batch_indirect_transform_size static_cast<int>(2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2) // 2^18 = 262144

namespace anopol::batch {

class Batch {
public:
    struct batchFrame {
        VkBuffer        drawCommandBuffer;
        VkDeviceMemory  drawCommandBufferMemory;
        VkDeviceSize    drawCommandBufferSize = 0;

        VkBuffer        transformBuffer;
        VkDeviceMemory  transformBufferMemory;
        VkDeviceSize    transformBufferSize = 0;

        bool allocatedTransformations = false;
        bool allocatedDrawCommands = false;
        bool empty = true;
        uint32_t idx;
    };
    
    std::vector<anopol::render::Vertex> batchVertices;
    std::vector<uint32_t> batchIndices;
    std::vector<batchDrawInformation> drawInformation;
    std::vector<batchIndirectTransformation> transformations;
    
    anopol::render::VertexBuffer vertexBuffer;
    anopol::render::IndexBuffer indexBuffer;
    MeshCombineGroup meshCombineGroup;

    static Batch Create();
    void Append(anopol::render::Renderable* renderable);
    void Append(anopol::render::Asset* asset);
    void Append(std::vector<anopol::render::Renderable*> renderables);
    void Append(std::vector<anopol::render::Asset*> assets);
    void Dealloc();
    void Combine(int currentFrame);
    void UpdateTransforms(batchFrame& frame, uint32_t idx);
    void Cull();
    batchFrame GetBatchFrame(int frame);
private:
    
    batchFrame frames[anopol_max_frames];
    bool    vertexBufferAllocated, indexBufferAllocated, framesAllocated;
    bool    everyObjectCulled = false;
    int     processed;
    size_t  uploadedTransformCount = 0;
    void allocateFrame(int frameidx);
};

Batch Batch::Create() {
    
    Batch batch = Batch();
    
    batch.vertexBuffer = anopol::render::VertexBuffer();
    batch.indexBuffer = anopol::render::IndexBuffer();
    
    batch.meshCombineGroup = MeshCombineGroup();
    batch.processed = 0;
    
    batch.batchVertices.reserve(4294967296);
    batch.batchIndices.reserve(4294967296);
    batch.drawInformation.reserve(4294967296);
    batch.transformations.reserve(4294967296);

    
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

void Batch::Combine(int currentFrame = -1) {
    
    VkFence fence = VK_NULL_HANDLE;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (vkCreateFence(context->device, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS) {
        anopol_assert("Failed to create fence");
    }
        
    uint32_t vertexOffset = 0, indexOffset = 0;
    size_t previousProcessed = processed;
    size_t currentCount = meshCombineGroup.renderables.size();
    
    bool isInFrustum = true;
    int culledAmount = 0;
    
    anopol::camera::Frustum frustum = anopol::camera::CreateFrustumPlanes(anopol::camera::camera);
    
    for (size_t i = previousProcessed; i < currentCount; i++) {
        
        anopol::render::Renderable* renderable = meshCombineGroup.renderables[i];
        
        float radius = renderable->ComputeBoundingSphereRadius();
        
        glm::mat4 model = anopol::modelMatrix(renderable->position, renderable->scale, renderable->rotation);
        
        if (!anopol::camera::isSphereInFrustum(renderable->position, renderable->scale, model, radius, frustum)) {
            //continue;
        }
        
        batchDrawInformation drawInfo;
        
        transformations.push_back({
            modelMatrix(renderable->position, renderable->scale, renderable->rotation),
            glm::vec4(renderable->color, 1.0f)
        });
        
        if (renderable->isIndexed == false || renderable->indexBuffer.bufferSize == VkDeviceSize(0)) {
            drawInfo.drawType       = nonIndexed;
            drawInfo.firstVertex    = vertexOffset;
            drawInfo.vertexCount    = static_cast<uint32_t>(renderable->vertices.size());
            drawInfo.object         = static_cast<uint32_t>(i);
        }
        else {
            drawInfo.drawType = indexed;
            for (uint32_t index : renderable->indices) {
                batchIndices.push_back(index + vertexOffset);
            }
            drawInfo.firstIndex     = indexOffset;
            drawInfo.indexCount     = static_cast<uint32_t>(renderable->indices.size());
            drawInfo.vertexOffset   = vertexOffset;
            drawInfo.object         = static_cast<uint32_t>(i);
            
            indexOffset += renderable->indices.size();
        }
        vertexOffset += renderable->vertices.size();
        
        drawInformation.push_back(drawInfo);
        
#if defined(__APPLE__) && defined(APPLE_USE_METAL_GPU_HELPERS)
        // to implement metal shader
        batchVertices.insert(batchVertices.end(), renderable->vertices.begin(), renderable->vertices.end());
#elif defined(__APPLE__) && defined(APPLE_USE_OPENCL_GPU_HELPERS)
        
#else
        batchVertices.insert(batchVertices.end(), renderable->vertices.begin(), renderable->vertices.end());
#endif
    }
    
    if (!isInFrustum) return;
    
    processed = meshCombineGroup.renderables.size();
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
    
    if (culledAmount == meshCombineGroup.renderables.size()) return;
    vertexBuffer.alloc(batchVertices);
    
    if (batchIndices.size() > 0 && !indexBufferAllocated) {
        indexBuffer.alloc(batchIndices);
        indexBufferAllocated = true;
    }
    
    if (currentFrame == -1) {
        for (int i = 0; i < anopol_max_frames; i++) {
            allocateFrame(i);
        }
    }
    else {
        allocateFrame(currentFrame);
    }
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, fence);
    
    vkWaitForFences(context->device, 1, &fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(context->device, fence, nullptr);

}

void Batch::allocateFrame(int frameidx) {
    batchFrame& frame = frames[frameidx];

    std::vector<VkDrawIndirectCommand> drawCommands;

    if (drawInformation.empty() || transformations.empty()) {
        frame.empty = true;
        return;
    }
    frame.empty = false;

    for (anopol::batch::batchDrawInformation drawInfo : drawInformation) {
        VkDrawIndirectCommand command{};
        command.vertexCount = drawInfo.vertexCount;
        command.instanceCount = 1;
        command.firstVertex = drawInfo.firstVertex;
        command.firstInstance = drawInfo.object;
        drawCommands.push_back(command);
    }

    VkDeviceSize bufferSize = sizeof(VkDrawIndirectCommand) * drawCommands.size();
    if (bufferSize == 0) {
        frame.empty = true;
        return;
    }

    if (!frame.allocatedDrawCommands || bufferSize > frame.drawCommandBufferSize) {
        // Clean up old buffer
        if (frame.allocatedDrawCommands) {
            vkDestroyBuffer(context->device, frame.drawCommandBuffer, nullptr);
            vkFreeMemory(context->device, frame.drawCommandBufferMemory, nullptr);
        }

        // Create new buffer
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        vkCreateBuffer(context->device, &bufferInfo, nullptr, &frame.drawCommandBuffer);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(context->device, frame.drawCommandBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = anopol::ll::findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkAllocateMemory(context->device, &allocInfo, nullptr, &frame.drawCommandBufferMemory);
        vkBindBufferMemory(context->device, frame.drawCommandBuffer, frame.drawCommandBufferMemory, 0);

        frame.drawCommandBufferSize = bufferSize;
        frame.allocatedDrawCommands = true;
    }

    VkBuffer staging;
    VkDeviceMemory stagingMemory;

    anopol::ll::createBuffer(bufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             staging, stagingMemory);

    void* data;
    vkMapMemory(context->device, stagingMemory, 0, bufferSize, 0, &data);
    memcpy(data, drawCommands.data(), bufferSize);
    vkUnmapMemory(context->device, stagingMemory);

    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(context->device, &fenceCreateInfo, nullptr, &fence);

    VkCommandBuffer commandBuffer = anopol::ll::beginSingleCommandBuffer();

    VkBufferCopy copyRegion{};
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(commandBuffer, staging, frame.drawCommandBuffer, 1, &copyRegion);

    anopol::ll::endSingleCommandBuffer(commandBuffer, fence);
    vkWaitForFences(context->device, 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyBuffer(context->device, staging, nullptr);
    vkFreeMemory(context->device, stagingMemory, nullptr);
    vkDestroyFence(context->device, fence, nullptr);

    UpdateTransforms(frame, frameidx);
}


void Batch::UpdateTransforms(batchFrame& frame, uint32_t idx) {
    
    VkDeviceSize bufferSize = sizeof(batchIndirectTransformation) * max_batch_indirect_transform_size;
        
    if (!frame.allocatedTransformations) {
        anopol::ll::createBuffer(bufferSize,
                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 frame.transformBuffer,
                                 frame.transformBufferMemory);
        frame.allocatedTransformations = true;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    anopol::ll::createBuffer(bufferSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             stagingBuffer,
                             stagingBufferMemory);
    
    void* data;
    vkMapMemory(context->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, transformations.data(), bufferSize);
    vkUnmapMemory(context->device, stagingBufferMemory);
    
    VkCommandBuffer commandBuffer = anopol::ll::beginSingleCommandBuffer();
    
    VkBufferCopy copyRegion{};
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(commandBuffer, stagingBuffer, frame.transformBuffer, 1, &copyRegion);
    
    anopol::ll::endSingleCommandBuffer(commandBuffer);
    
    vkDestroyBuffer(context->device, stagingBuffer, nullptr);
    vkFreeMemory(context->device, stagingBufferMemory, nullptr);
}

Batch::batchFrame Batch::GetBatchFrame(int frame) {
    return frames[frame];
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
    
    for (int i = 0; i < anopol_max_frames; i++) {
        vkDestroyBuffer(context->device, frames[i].drawCommandBuffer, nullptr);
        vkFreeMemory(context->device, frames[i].drawCommandBufferMemory, nullptr);
        
        vkDestroyBuffer(context->device, frames[i].transformBuffer, nullptr);
        vkFreeMemory(context->device, frames[i].transformBufferMemory, nullptr);
    }
}

void Batch::Cull() {
    drawInformation.clear();
    transformations.clear();

    uint32_t vertexOffset = 0, indexOffset = 0;

    anopol::camera::Frustum frustum = anopol::camera::CreateFrustumPlanes(anopol::camera::camera);

    for (size_t i = 0; i < meshCombineGroup.renderables.size(); i++) {
        anopol::render::Renderable* renderable = meshCombineGroup.renderables[i];

        float radius = renderable->ComputeBoundingSphereRadius();
        glm::mat4 model = anopol::modelMatrix(renderable->position, renderable->scale, renderable->rotation);

        if (!anopol::camera::isSphereInFrustum(renderable->position, renderable->scale, model, radius, frustum)) {
            continue;
        }

        batchDrawInformation drawInfo;
        transformations.push_back({
            model,
            glm::vec4(renderable->color, 1.0f)
        });

        if (!renderable->isIndexed || renderable->indexBuffer.bufferSize == 0) {
            drawInfo.drawType = nonIndexed;
            drawInfo.firstVertex = vertexOffset;
            drawInfo.vertexCount = static_cast<uint32_t>(renderable->vertices.size());
            drawInfo.object = static_cast<uint32_t>(i);
        }
        else {
            drawInfo.drawType = indexed;
            for (uint32_t index : renderable->indices) {
                batchIndices.push_back(index + vertexOffset);
            }
            drawInfo.firstIndex = indexOffset;
            drawInfo.indexCount = static_cast<uint32_t>(renderable->indices.size());
            drawInfo.vertexOffset = vertexOffset;
            drawInfo.object = static_cast<uint32_t>(i);
            indexOffset += renderable->indices.size();
        }

        drawInformation.push_back(drawInfo);
        vertexOffset += renderable->vertices.size();
    }

    for (int i = 0; i < anopol_max_frames; ++i) {
        allocateFrame(i);
    }
}


}

#endif /* batch_h */
