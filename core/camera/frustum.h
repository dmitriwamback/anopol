//
//  frustum.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-26.
//

#ifndef frustum_h
#define frustum_h

namespace anopol::camera {

struct Frustum {
    glm::vec4 planes[6];
};

Frustum CreateFrustumPlanes(const glm::mat4& viewProjection) {
    Frustum frustum{};
    const glm::mat4& m = viewProjection;
    
    glm::vec4 rowX = glm::vec4(viewProjection[0][0], viewProjection[0][1], viewProjection[0][2], viewProjection[0][3]); // row 0
    glm::vec4 rowY = glm::vec4(viewProjection[1][0], viewProjection[1][1], viewProjection[1][2], viewProjection[1][3]); // row 1
    glm::vec4 rowZ = glm::vec4(viewProjection[2][0], viewProjection[2][1], viewProjection[2][2], viewProjection[2][3]); // row 2
    glm::vec4 rowW = glm::vec4(viewProjection[3][0], viewProjection[3][1], viewProjection[3][2], viewProjection[3][3]); // row 3

    frustum.planes[0] = rowW + rowX; // Left
    frustum.planes[1] = rowW - rowX; // Right
    frustum.planes[2] = rowW - rowY; // Top
    frustum.planes[3] = rowW + rowY; // Bottom
    frustum.planes[4] = rowW + rowZ; // Near
    frustum.planes[5] = rowW - rowZ; // Far

    for (int i = 0; i < 6; ++i) {
        float length = glm::length(glm::vec3(frustum.planes[i]));
        frustum.planes[i] /= length;
    }
    return frustum;
}

bool isSphereInFrustum(glm::vec3 position, float radius, const Frustum& frustum) {
    for (int i = 0; i < 6; ++i) {
        const glm::vec4& plane = frustum.planes[i];
        float distance = glm::dot(glm::vec3(plane), position) + plane.w;
        if (distance < -radius)
            return false;
    }
    return true;
}

}

#endif /* frustum_h */
