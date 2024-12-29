//
//  renderable.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-26.
//

#ifndef renderable_h
#define renderable_h

namespace anopol::render {
    
class Renderable {
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VertexBuffer vertexBuffer;
    IndexBuffer  indexBuffer;
    bool isIndexed;
    
    glm::vec3 position, rotation, scale;
    
    static Renderable* Create();
};

Renderable* Renderable::Create() {
    
    Renderable* renderable = new Renderable();
    
    renderable->scale    = glm::vec3(1.0f, 1.0f, 1.0f);
    renderable->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    renderable->position = glm::vec3(0.0f);
    
    renderable->vertices = {
        { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f) },  // 0
        { glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 0.0f) },  // 1
        { glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f) },  // 2
        { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 1.0f) },  // 3
        { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 0.0f) },  // 4
        { glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 0.0f) },  // 5
        { glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 1.0f) },  // 6
        { glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 1.0f) }   // 7
    };

    renderable->indices = {
        // Back face
            0, 3, 2, 0, 2, 1,
            // Front face
            4, 7, 6, 4, 6, 5,
            // Left face
            4, 0, 3, 4, 3, 7,
            // Right face
            1, 2, 6, 1, 6, 5,
            // Bottom face
            4, 5, 1, 4, 1, 0,
            // Top face
            3, 6, 2, 3, 2, 7
    };
    
    renderable->isIndexed = true;
    renderable->vertexBuffer = VertexBuffer();
    renderable->vertexBuffer.alloc(renderable->vertices);
    
    renderable->indexBuffer = IndexBuffer();
    renderable->indexBuffer.alloc(renderable->indices);
    
    return renderable;
}
}

#endif /* renderable_h */
