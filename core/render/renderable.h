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
    std::vector<float> GetColliderVertices(bool withNormals);
};

Renderable* Renderable::Create() {
    
    Renderable* renderable = new Renderable();
    
    renderable->scale    = glm::vec3(1.0f, 1.0f, 1.0f);
    renderable->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    renderable->position = glm::vec3(0.0f);
    
    renderable->vertices = {
        // Front face
            {{-0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {0, 0}},
            {{ 0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {1, 0}},
            {{ 0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {1, 1}},
            {{ 0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {1, 1}},
            {{-0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {0, 1}},
            {{-0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {0, 0}},

            // Back face
            {{ 0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {0, 0}},
            {{-0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {1, 0}},
            {{-0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {1, 1}},
            {{-0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {1, 1}},
            {{ 0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {0, 1}},
            {{ 0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {0, 0}},

            // Right face
            {{ 0.5f, -0.5f,  0.5f}, { 1,  0,  0}, {0, 0}},
            {{ 0.5f, -0.5f, -0.5f}, { 1,  0,  0}, {1, 0}},
            {{ 0.5f,  0.5f, -0.5f}, { 1,  0,  0}, {1, 1}},
            {{ 0.5f,  0.5f, -0.5f}, { 1,  0,  0}, {1, 1}},
            {{ 0.5f,  0.5f,  0.5f}, { 1,  0,  0}, {0, 1}},
            {{ 0.5f, -0.5f,  0.5f}, { 1,  0,  0}, {0, 0}},

            // Left face
            {{-0.5f, -0.5f, -0.5f}, {-1,  0,  0}, {0, 0}},
            {{-0.5f, -0.5f,  0.5f}, {-1,  0,  0}, {1, 0}},
            {{-0.5f,  0.5f,  0.5f}, {-1,  0,  0}, {1, 1}},
            {{-0.5f,  0.5f,  0.5f}, {-1,  0,  0}, {1, 1}},
            {{-0.5f,  0.5f, -0.5f}, {-1,  0,  0}, {0, 1}},
            {{-0.5f, -0.5f, -0.5f}, {-1,  0,  0}, {0, 0}},

            // Top face
            {{-0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {0, 0}},
            {{ 0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {1, 0}},
            {{ 0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {1, 1}},
            {{ 0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {1, 1}},
            {{-0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {0, 1}},
            {{-0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {0, 0}},

            // Bottom face
            {{-0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {0, 0}},
            {{ 0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {1, 0}},
            {{ 0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {1, 1}},
            {{ 0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {1, 1}},
            {{-0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {0, 1}},
            {{-0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {0, 0}},
    };

    renderable->indices = {
        // Front face
                0, 2, 1, 0, 3, 2,  // Reversed (CW)

                // Back face
                5, 7, 4, 5, 6, 7,  // Reversed (CW)

                // Left face
                4, 3, 0, 4, 7, 3,  // Reversed (CW)

                // Right face
                1, 6, 5, 1, 2, 6,  // Reversed (CW)

                // Top face
                4, 1, 5, 4, 0, 1,  // Reversed (CW)

                // Bottom face
                3, 6, 2, 3, 7, 6   // Reversed (CW)
    };
    
    renderable->isIndexed = false;
    renderable->vertexBuffer = VertexBuffer();
    renderable->vertexBuffer.alloc(renderable->vertices);
    
    return renderable;
}

std::vector<float> Renderable::GetColliderVertices(bool withNormals = false) {
    
    glm::mat4 model = anopol::modelMatrix(position, scale, rotation);
    
    std::vector<float> projectedVertices = std::vector<float>();
    
    for (int i = 0; i < vertices.size(); i++) {
        glm::vec3 vertex = vertices[i].vertex;
        glm::vec3 projected = glm::vec3(model * glm::vec4(vertex, 1.0));
        
        projectedVertices.push_back(projected.x);
        projectedVertices.push_back(projected.y);
        projectedVertices.push_back(projected.z);
        if (withNormals) {
            projectedVertices.push_back(0);
            projectedVertices.push_back(0);
            projectedVertices.push_back(0);
        }
    }
    return projectedVertices;
}

}

#endif /* renderable_h */
