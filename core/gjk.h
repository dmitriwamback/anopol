//
//  gjk.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-12-29.
//
#ifndef gjk_h
#define gjk_h
// GJK Gilbert-Johnson-Keerthi Algorithm
#include <algorithm>
namespace anopol::collision {
struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};
glm::vec3 transformVertex(glm::vec3 vertex, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
    
    glm::mat4 translationMatrix = glm::mat4(1.0f);
    translationMatrix = glm::translate(translationMatrix, position);
    
    glm::mat4 scaleMatrix = glm::mat4(1.0f);
    scaleMatrix = glm::scale(scaleMatrix, scale);
    
    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 rotationMatrix = rotationZ * rotationY * rotationX;
    glm::mat4 transformation = scaleMatrix * rotationMatrix * translationMatrix;
    glm::vec4 newVertex = transformation * glm::vec4(vertex, 1.0f);
    
    return glm::vec3(newVertex);
}
glm::vec3 support(const std::vector<anopol::render::Vertex>& vertices, const std::vector<uint32_t>& indices, const glm::vec3& direction, std::array<glm::vec3, 3> transformation) {
    
    float maxDot = -FLT_MAX;
    glm::vec3 bestVertex;
    
    for (uint32_t index : indices) {
        
        const glm::vec3& vertex = transformVertex(vertices[index].vertex, transformation[0], transformation[1], transformation[2]);
        float dot = glm::dot(vertex, direction);
        
        if (dot > maxDot) {
            maxDot = dot;
            bestVertex = vertex;
        }
    }
    
    return bestVertex;
}
bool GJKcollision(const std::vector<anopol::render::Vertex>& vertices, const std::vector<uint32_t>& indices, std::array<glm::vec3, 3> transformation) {
    
    glm::vec3 direction = glm::normalize(anopol::camera::camera.cameraPosition);
    std::vector<glm::vec3> simplex;
        
    glm::vec3 supportVertex = support(vertices, indices, direction, transformation) - anopol::camera::camera.cameraPosition;
    simplex.push_back(supportVertex);
    direction = -supportVertex;
    
    const int MAX_ITERATIONS = 50;
    int iterations = 0;
    
    while (iterations < MAX_ITERATIONS) {
        glm::vec3 newPoint = support(vertices, indices, direction, transformation) - anopol::camera::camera.cameraPosition;
        
        if (glm::dot(newPoint, direction) <= 0) return false;
        
        simplex.push_back(newPoint);
        
        glm::vec3 ao = -simplex.back();
        
        if (simplex.size() != 3) {
            iterations++;
            continue;
        }
        glm::vec3 ab = simplex[1] - simplex[2], ac = simplex[0] - simplex[2];
        glm::vec3 abc = glm::cross(ab, ac);
        
        if (glm::dot(glm::cross(abc, ac), ao) > 0) {
            simplex.erase(simplex.begin());
            direction = glm::cross(ac, ao);
        }
        else if (glm::dot(glm::cross(ab, abc), ao) > 0) {
            simplex.erase(simplex.begin() + 1);
            direction = glm::cross(ab, ao);
        }
        else {
            return true;
        }
        direction = ao;
        iterations++;
    }
    
    return false;
}
bool raycast(const Ray& ray, const anopol::render::Asset::Mesh* mesh, float& hitDistance) {
    
    bool hit = false;
    
    hitDistance = FLT_MAX;
    
    for (size_t i = 0; i < mesh->indices.size(); i += 3) {
        
        glm::vec3 v0 = mesh->vertices[mesh->indices[i]].vertex;
        glm::vec3 v1 = mesh->vertices[mesh->indices[i + 1]].vertex;
        glm::vec3 v2 = mesh->vertices[mesh->indices[i + 2]].vertex;
        
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 pvec = glm::cross(ray.direction, edge2);
        float det = glm::dot(edge1, pvec);
        
        if (fabs(det) < 1e-8) continue;
        float inv = 1.0f/det;
        
        glm::vec3 tvec = ray.origin - v0;
        float u = glm::dot(tvec, pvec) * inv;
        if (u < 0 || u > 1) continue;
        
        glm::vec3 qvec = glm::cross(tvec, edge1);
        float v = glm::dot(ray.direction, qvec) * inv;
        if (v < 0 || u + v > 1) continue;
        
        float t = glm::dot(edge2, qvec) * inv;
        if (t > 0.0f && t < hitDistance) {
            hit = true;
            hitDistance = t;
        }
    }
    
    return hit;
}
bool raycast(const Ray& ray, const anopol::render::Renderable* renderable, float& hitDistance) {
    
    bool hit = false;
    
    hitDistance = FLT_MAX;
    
    for (size_t i = 0; i < renderable->indices.size(); i += 3) {
        
        glm::vec3 v0 = renderable->vertices[renderable->indices[i]].vertex;
        glm::vec3 v1 = renderable->vertices[renderable->indices[i + 1]].vertex;
        glm::vec3 v2 = renderable->vertices[renderable->indices[i + 2]].vertex;
        
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 pvec = glm::cross(ray.direction, edge2);
        float det = glm::dot(edge1, pvec);
        
        if (fabs(det) < 1e-8) continue;
        float inv = 1.0f/det;
        
        glm::vec3 tvec = ray.origin - v0;
        float u = glm::dot(tvec, pvec) * inv;
        if (u < 0 || u > 1) continue;
        
        glm::vec3 qvec = glm::cross(tvec, edge1);
        float v = glm::dot(ray.direction, qvec) * inv;
        if (v < 0 || u + v > 1) continue;
        
        float t = glm::dot(edge2, qvec) * inv;
        if (t > 0.0f && t < hitDistance) {
            hit = true;
            hitDistance = t;
        }
    }
    
    return hit;
}
bool GJK(const anopol::render::Asset::Mesh* mesh) {
    
    std::array<glm::vec3, 3> transformation = {
        mesh->parent->position,
        mesh->parent->rotation,
        mesh->parent->scale
    };
    
    return GJKcollision(mesh->vertices, mesh->indices, transformation);
}
bool GJK(const anopol::render::Renderable* renderable) {
    
    std::array<glm::vec3, 3> transformation = {
        renderable->position,
        renderable->rotation,
        renderable->scale
    };
    
    return GJKcollision(renderable->vertices, renderable->indices, transformation);
}
void resolveCameraPosition() {
    
}
}
#endif /* gjk_h */
