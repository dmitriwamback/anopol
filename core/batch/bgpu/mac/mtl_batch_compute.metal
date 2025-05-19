//
//  mtl_batch_compute.metal
//  anopol
//
//  Created by Dmitri Wamback on 2025-05-02.
//

#include <metal_stdlib>
using namespace metal;

struct batchDrawInformation {
    
    int drawType;
    
    uint firstIndex;
    uint indexCount;
    uint vertexOffset;
    uint object;
    
    uint firstVertex;
    uint vertexCount;
    uint texture;
};

struct batchIndirectTransformation {
    float4x4 model;
    float4 color;
};
struct RenderableInformation {
    uint vertexOffset;
    uint indexOffset;
    uint vertexCount;
    uint indexCount;
    uint isIndexed;
};
struct Vertex {
    float3 p_vertex;
    float3 p_normal;
    int2   p_uv;
};


kernel void batchMergeVertices(
    device const RenderableInformation* renderables [[ buffer(0) ]],
    device const Vertex* vsrc [[ buffer(1) ]],
    device const uint* isrc [[ buffer(2) ]],
    
    device Vertex* vdst [[ buffer(3) ]],
    device uint* idst [[ buffer(4) ]],
                               
    device atomic_uint* vOffset [[ buffer(5) ]],
    device atomic_uint* iOffset [[ buffer(6) ]],
    
    device batchDrawInformation* drawInformations [[ buffer(7) ]],
    uint3 threadPosition [[ thread_position_in_threadgroup ]],
    uint3 groupID [[ threadgroup_position_in_grid ]],
    uint3 threadgroupSize [[ threads_per_threadgroup ]]
) {
    const uint threadID = threadPosition.x;
    const uint groupIndex = groupID.x;
    
    const RenderableInformation renderable = renderables[groupIndex];
    
    threadgroup Vertex lVertices[256];
    threadgroup uint lIndices[512];
    
    if (threadID < renderable.vertexCount) {
        lVertices[threadID] = vsrc[renderable.vertexOffset + threadID];
    }
    
    if (threadID < renderable.indexCount) {
        lIndices[threadID] = isrc[renderable.indexOffset + threadID];
    }
    
    threadgroup_barrier(mem_flags::mem_threadgroup);
    
    threadgroup uint vertexBase;
    threadgroup uint indexBase;
    if (threadID == 0) {
        vertexBase = atomic_fetch_add_explicit(vOffset, renderable.vertexCount, memory_order_relaxed);
        indexBase = atomic_fetch_add_explicit(iOffset, renderable.indexCount, memory_order_relaxed);
    }

    threadgroup_barrier(mem_flags::mem_threadgroup);

    if (threadID < renderable.vertexCount) {
        vdst[vertexBase + threadID] = lVertices[threadID];
    }

    if (threadID < renderable.indexCount) {
        idst[indexBase + threadID] = lIndices[threadID] + vertexBase;
    }

    if (threadID == 0) {
        drawInformations[groupIndex] = batchDrawInformation {
            .drawType       = (renderable.isIndexed && renderable.indexCount > 0) ? 1 : 0,
            .firstVertex    = vertexBase,
            .vertexCount    = renderable.vertexCount,
            .firstIndex     = indexBase,
            .indexCount     = renderable.indexCount,
            .vertexOffset   = 0,
            .object         = groupIndex
        };
    }
}
