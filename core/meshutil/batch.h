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
    
    anopol::render::VertexBuffer vertexBuffer;
    anopol::render::IndexBuffer indexBuffer;
    MeshCombineGroup meshCombineGroup;
    
    static Batch Create();
    void Append(anopol::render::Renderable* renderable);
    void Append(anopol::render::Asset* asset);
    void Append(std::vector<anopol::render::Renderable*> renderables);
    void Append(std::vector<anopol::render::Asset*> assets);
    
private:
    void Combine();
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
    Combine();
}

void Batch::Append(anopol::render::Asset* asset) {
    meshCombineGroup.Append(asset);
    Combine();
}

void Batch::Append(std::vector<anopol::render::Renderable*> renderables) {
    meshCombineGroup.Append(renderables);
    Combine();
}

void Batch::Append(std::vector<anopol::render::Asset*> assets) {
    meshCombineGroup.Append(assets);
    Combine();
}

void Batch::Combine() {
    
    drawInformation.clear();
    batchVertices.clear();
    batchIndices.clear();
    
    uint32_t vertexOffset = 0, indexOffset = 0, object = 0;
    
    for (anopol::render::Renderable* renderable : meshCombineGroup.renderables) {
        
        batchDrawInformation drawInfo;
        
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
    
    vertexBuffer.alloc(batchVertices);
    if (batchIndices.size() > 0) indexBuffer.alloc(batchIndices);
}

}

#endif /* batch_h */
