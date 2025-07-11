//
//  collision_ray_thread.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-04-18.
//

#ifndef collision_ray_thread_h
#define collision_ray_thread_h

namespace anopol::pipeline {

void CollideAndRaycastBatch(anopol::batch::Batch batch) {
    
    while (true) {
        for (anopol::render::Renderable* r : batch.meshCombineGroup.renderables) {
            
            anopol::collision::collision col = anopol::collision::GJKCollisionWithCamera(r);
            if (col.collided) {
                if (glm::dot(col.normal, r->position - anopol::camera::camera.cameraPosition) > 0) {
                    col.normal = -col.normal;
                }
                
                std::cout << col.depth << '\n';
                
                static std::mutex camMutex;
                {
                    std::lock_guard<std::mutex> lock(camMutex);
                    anopol::camera::camera.cameraPosition += col.normal * col.depth;
                    anopol::camera::camera.updateLookAt();
                }
            }
            
            anopol::camera::Ray ray{};
            ray.origin = anopol::camera::camera.cameraPosition;
            ray.direction = anopol::camera::camera.mouseRay;
            
            std::optional<anopol::camera::Intersection> intersection = anopol::camera::Raycast(ray, r);
            if (intersection.has_value()) {
                intersection->target->color = glm::vec3(1.0f);
            }
        }
    }
    anopol::camera::camera.updateLookAt();
}

}

#endif /* collision_ray_thread_h */
