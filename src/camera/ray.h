//
//  ray.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-04-18.
//

#ifndef ray_h
#define ray_h
#include <optional>

namespace anopol::camera {

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

struct Intersection {
    glm::vec3 intersectionPoint;
    glm::vec3 normal;
    float distance;
    anopol::render::Renderable* target;
};

std::optional<Intersection> RayIntersectTriangle(const Ray& ray, const glm::vec3& pointA, const glm::vec3& pointB, const glm::vec3& pointC, anopol::render::Renderable* renderable) {
    
    glm::vec3 edge1 = pointB - pointA;
    glm::vec3 edge2 = pointC - pointA;
    glm::vec3 h = glm::cross(ray.direction, edge2);
    float a = glm::dot(edge1, h);
    
    if (fabs(a) < 0.0000001f) return std::nullopt;
    float f = 1.0f / a;
    
    glm::vec3 s = ray.origin - pointA;
    float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) return std::nullopt;
    
    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(ray.direction, q);
    if (v < 0.0f || u + v > 1.0f) return std::nullopt;
    
    float t = f * glm::dot(edge2, q);
    if (t > 0.0000001f) {
        return Intersection{ray.origin + ray.direction * t, glm::vec3(0.0f), t, renderable};
    }
    
    return std::nullopt;
}

std::optional<Intersection> Raycast(const Ray& ray, anopol::render::Renderable* renderable) {
    
    std::optional<Intersection> closest;
    
    std::vector<anopol::render::Vertex> vertices = renderable->GetColliderVertices();
    
    if (renderable->indexBuffer.bufferSize != VkDeviceSize(0)) {
        for (size_t i = 0; i < renderable->indexBuffer.indices.size(); i += 3) {
            glm::vec3 pointA = vertices[renderable->indexBuffer.indices[i]].vertex;
            glm::vec3 pointB = vertices[renderable->indexBuffer.indices[i + 1]].vertex;
            glm::vec3 pointC = vertices[renderable->indexBuffer.indices[i + 2]].vertex;
            
            auto intersection = RayIntersectTriangle(ray, pointA, pointB, pointC, renderable);
            if (intersection) {
                if (!closest || intersection->distance < closest->distance) {
                    closest = intersection;
                    closest->normal = glm::normalize(glm::cross(pointB - pointA, pointC - pointA));
                }
            }
        }
    }
    else {
        for (int i = 0; i < vertices.size(); i += 3) {
            const glm::vec3& pointA = vertices[i + 0].vertex;
            const glm::vec3& pointB = vertices[i + 1].vertex;
            const glm::vec3& pointC = vertices[i + 2].vertex;

            auto intersection = RayIntersectTriangle(ray, pointA, pointB, pointC, renderable);
            if (intersection) {
                if (!closest || intersection->distance < closest->distance) {
                    closest = intersection;
                    closest->normal = glm::normalize(glm::cross(pointB - pointA, pointC - pointA));
                }
            }
        }
    }
    return closest;
}

}

#endif /* ray_h */
