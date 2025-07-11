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
    bool isIndexed, collisionEnabled = true;
    
    glm::vec3 position, rotation, scale, color;
    
    static Renderable* Create();
    std::vector<float> GetColliderVertices(bool withNormals);
    float ComputeBoundingSphereRadius();
    float boundingSphereRadius;
private:
    glm::vec3 lastPosition, lastRotation, lastScale;
    glm::mat4 currentModel;
    bool computedBoundingSphere, computedModelMatrix;
};

Renderable* Renderable::Create() {
    
    Renderable* renderable = new Renderable();
    
    renderable->scale    = glm::vec3(1.0f, 1.0f, 1.0f);
    renderable->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    renderable->position = glm::vec3(0.0f);
    
    renderable->lastScale    = renderable->scale;
    renderable->lastRotation = renderable->rotation;
    renderable->lastPosition = renderable->position;
    
    renderable->currentModel = glm::mat4(0.0f);
    
    renderable->computedBoundingSphere = false;
    
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
    
    /*
    renderable->indices = {
        0, 2, 1, 0, 3, 2,
        5, 7, 4, 5, 6, 7,
        4, 3, 0, 4, 7, 3,
        1, 6, 5, 1, 2, 6,
        4, 1, 5, 4, 0, 1,
        3, 6, 2, 3, 7, 6
    };
    */
    
    renderable->isIndexed = false;
    //renderable->vertexBuffer = VertexBuffer();
    //renderable->vertexBuffer.alloc(renderable->vertices);
    
    return renderable;
}

std::vector<float> Renderable::GetColliderVertices(bool withNormals = false) {
    
    if (lastPosition != position || lastScale != scale || lastRotation != rotation || !computedModelMatrix) {
        currentModel = anopol::modelMatrix(position, scale, rotation);
        computedModelMatrix = true;
    }
    
    std::vector<float> projectedVertices = std::vector<float>();
    
    for (int i = 0; i < vertices.size(); i++) {
        glm::vec3 vertex = vertices[i].vertex;
        glm::vec3 projected = glm::vec3(currentModel * glm::vec4(vertex, 1.0));
        
        projectedVertices.push_back(projected.x);
        projectedVertices.push_back(projected.y);
        projectedVertices.push_back(projected.z);
        if (withNormals) {
            projectedVertices.push_back(0);
            projectedVertices.push_back(0);
            projectedVertices.push_back(0);
        }
    }
    lastPosition = position;
    lastScale    = scale;
    lastRotation = rotation;
    
    return projectedVertices;
}

float Renderable::ComputeBoundingSphereRadius() {
    
    float radius = 0;
    
    if (lastPosition != position || lastScale != scale || lastRotation != rotation || !computedBoundingSphere) {
        
        std::vector<float> colliderVertices = GetColliderVertices();
        glm::vec3 max = glm::vec3(colliderVertices[0], colliderVertices[1], colliderVertices[2]);
        glm::vec3 min = glm::vec3(colliderVertices[0], colliderVertices[1], colliderVertices[2]);
        
        for (int i = 0; i < colliderVertices.size()/3; i++) {
            glm::vec3 vertex = glm::vec3(colliderVertices[i*3], colliderVertices[i*3+1], colliderVertices[i*3+2]);
            min = glm::min(min, vertex);
            max = glm::max(max, vertex);
        }
        
        glm::vec3 center = (min + max) / 2.0f;
        radius = glm::distance(center, max);
        
        boundingSphereRadius = radius;
        computedBoundingSphere = true;
        
        lastPosition = position;
        lastScale    = scale;
        lastRotation = rotation;
    }
    
    return boundingSphereRadius;
}


}

#endif /* renderable_h */
