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

glm::vec3 support(const std::vector<anopol::render::Vertex>& vertices, const std::vector<uint32_t>& indices, const glm::vec3& direction, glm::mat4 model) {
    
    float maxDot = -FLT_MAX;
    glm::vec3 bestVertex;
    
    for (uint32_t index : indices) {
        
        const glm::vec3& vertex = glm::vec3(model * glm::vec4(vertices[index].vertex, 1.0f));
        float dot = glm::dot(vertex, direction);
        
        if (dot > maxDot) {
            maxDot = dot;
            bestVertex = vertex;
        }
    }
    
    return bestVertex;
}
bool GJKcollision(const std::vector<anopol::render::Vertex>& vertices, const std::vector<uint32_t>& indices, glm::mat4 model) {
    
    glm::vec3 direction = glm::normalize(anopol::camera::camera.cameraPosition);
    std::vector<glm::vec3> simplex;
        
    glm::vec3 supportVertex = support(vertices, indices, direction, model) - anopol::camera::camera.cameraPosition;
    simplex.push_back(supportVertex);
    direction = -supportVertex;
    
    const int MAX_ITERATIONS = 50;
    int iterations = 0;
    
    while (iterations < MAX_ITERATIONS) {
        glm::vec3 newPoint = support(vertices, indices, direction, model) - anopol::camera::camera.cameraPosition;
        
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
    
    //return GJKcollision(mesh->vertices, mesh->indices, model);
    return false;
}
bool GJK(const anopol::render::Renderable* renderable, glm::mat4 model) {
    
    return GJKcollision(renderable->vertices, renderable->indices, model);
}
void resolveCameraPosition() {
    
}
}
#endif /* gjk_h */
