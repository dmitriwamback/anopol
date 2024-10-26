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
    VertexBuffer vertexBuffer;
    
    static Renderable Create();
};

Renderable Renderable::Create() {
    
    Renderable renderable = Renderable();
    
    renderable.vertices = {
        {{ 0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.2f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
    };
    renderable.vertexBuffer = VertexBuffer();
    renderable.vertexBuffer.alloc(renderable.vertices);
    
    return renderable;
}
}

#endif /* renderable_h */
