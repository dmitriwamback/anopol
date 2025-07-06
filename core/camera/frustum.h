//
//  frustum.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-26.
//

#ifndef frustum_h
#define frustum_h

namespace anopol::camera {

struct Plane {
    glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
    float distance = 0.0f;
    
    float getSignedDistanceToPlane(const glm::vec3& point) const {
        return glm::dot(normal, point) - distance;
    }
    
    Plane() = default;
    
    Plane(const glm::vec3& p1, const glm::vec3& norm) : normal(glm::normalize(norm)), distance(glm::dot(normal, p1)) {}
};

struct Frustum {
    Plane top, bottom,
          left, right,
          far, near;
};
struct Sphere {
    glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
    float radius = 0.0f;
    
    Sphere(const glm::vec3& inCenter, float inRadius) : position{ inCenter }, radius{ inRadius } {}
    
    bool isOnOrForwardPlane(const Plane& plane) const {
        return plane.getSignedDistanceToPlane(position) >= -radius;
    }
};

Frustum CreateFrustumPlanes(Camera camera) {
    Frustum frustum;
    
    float zNear = camera.near;
    float zFar = camera.far;

    const float halfVSide = zFar * tanf(camera.fov * 0.5f);
    const float halfHSide = halfVSide * camera.aspect;
    const glm::vec3 front = zFar * camera.lookDirection;
    
    glm::vec3 up = glm::normalize(glm::cross(camera.right, camera.lookDirection));

    frustum.near    = { camera.cameraPosition + zNear * camera.lookDirection, camera.lookDirection };
    frustum.far     = { camera.cameraPosition + front, -camera.lookDirection };
    frustum.right   = { camera.cameraPosition, glm::cross(front - camera.right * halfHSide, up) };
    frustum.left    = { camera.cameraPosition, glm::cross(up, front + camera.right * halfHSide) };
    frustum.top     = { camera.cameraPosition, glm::cross(camera.right, front - up * halfVSide) };
    frustum.bottom  = { camera.cameraPosition, glm::cross(front + up * halfVSide, camera.right) };

    return frustum;
}

bool isSphereInFrustum(glm::vec3 position, glm::vec3 scale, glm::mat4 model, float radius, const Frustum& frustum) {
    
    const glm::vec3 globalCenter = glm::vec3(model[3]);
    Sphere globalSphere(globalCenter, radius);
    
    return (globalSphere.isOnOrForwardPlane(frustum.left) &&
            globalSphere.isOnOrForwardPlane(frustum.right) &&
            globalSphere.isOnOrForwardPlane(frustum.far) &&
            globalSphere.isOnOrForwardPlane(frustum.near) &&
            globalSphere.isOnOrForwardPlane(frustum.top) &&
            globalSphere.isOnOrForwardPlane(frustum.bottom));
}

}

#endif /* frustum_h */
