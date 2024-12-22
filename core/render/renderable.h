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
        { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f) }, // bottom-left
        { glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f) }, // top-right
        { glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 0.0f) }, // bottom-right
        { glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f) }, // top-right
        { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f) }, // bottom-left
        { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 1.0f) }, // top-left
        // front face
        { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 0.0f) }, // bottom-left
        { glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 0.0f) }, // bottom-right
        { glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 1.0f) }, // top-right
        { glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 1.0f) }, // top-right
        { glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 1.0f) }, // top-left
        { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 0.0f) }, // bottom-left
        // left face
        { glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) }, // top-right
        { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f) }, // top-left
        { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) }, // bottom-left
        { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) }, // bottom-left
        { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f) }, // bottom-right
        { glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) }, // top-right
        // right face
        { glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) }, // top-left
        { glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) }, // bottom-right
        { glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f) }, // top-right
        { glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) }, // bottom-right
        { glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) }, // top-left
        { glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f) }, // bottom-left
        // bottom face
        { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 1.0f) }, // top-right
        { glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 1.0f) }, // top-left
        { glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 0.0f) }, // bottom-left
        { glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 0.0f) }, // bottom-left
        { glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 0.0f) }, // bottom-right
        { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 1.0f) }, // top-right
        // top face
        { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 1.0f) }, // top-left
        { glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 0.0f) }, // bottom-right
        { glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 1.0f) }, // top-right
        { glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 0.0f) }, // bottom-right
        { glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 1.0f) }, // top-left
        { glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 0.0f) }  // bottom-left
    };
    
    renderable->isIndexed = false;
    renderable->vertexBuffer = VertexBuffer();
    renderable->vertexBuffer.alloc(renderable->vertices);
    
    return renderable;
}
}

#endif /* renderable_h */
