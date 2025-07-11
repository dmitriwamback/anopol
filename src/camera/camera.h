//
//  camera.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-26.
//

#ifndef camera_h
#define camera_h

namespace anopol::camera {

class Camera {
public:
    
    // vertices for a dodecahedron
    std::vector<float> vertices = {
        1.0f,  1.0f,                  1.0f,           1.0f,  1.0f,                         -1.0f,
        1.0f, -1.0f,                  1.0f,           1.0f, -1.0f,                         -1.0f,
        0.0f,  inverse_golden_ratio,  golden_ratio,   0.0f,  inverse_golden_ratio, -golden_ratio,
        0.0f, -inverse_golden_ratio,  golden_ratio,   0.0f, -inverse_golden_ratio, -golden_ratio,
        golden_ratio,  inverse_golden_ratio,  0.0f,   golden_ratio, -inverse_golden_ratio,  0.0f,
       -golden_ratio,  inverse_golden_ratio,  0.0f,  -golden_ratio, -inverse_golden_ratio,  0.0f,
        golden_ratio,  0.0f,  inverse_golden_ratio,  -golden_ratio,  0.0f,  inverse_golden_ratio,
        golden_ratio,  0.0f, -inverse_golden_ratio,  -golden_ratio,  0.0f, -inverse_golden_ratio,
        0.0f,  golden_ratio,  inverse_golden_ratio,   0.0f, -golden_ratio,  inverse_golden_ratio,
        0.0f,  golden_ratio, -inverse_golden_ratio,   0.0f, -golden_ratio, -inverse_golden_ratio
    };
    
    glm::vec3 cameraPosition, lookDirection, mouseRay, velocity, right;
    glm::mat4 cameraProjection, cameraLookAt;
    
    float pitch;
    float yaw = 3.0f * 3.141592653f/2.0f;
    float speed = 25.0f;
    float aspect = 1.0f;
    float fov = 3.14159265358f/2.0f;
    float near;
    float far;
    
    float lastMouseX;
    float lastMouseY;
    
    bool lockCamera;
    bool firstPersonView;
    
    int mouseButton = GLFW_MOUSE_BUTTON_RIGHT;
    
    static void initialize();
    void update(glm::vec4 movement);
    void updateLookAt();
    std::vector<float> GetColliderVertices(glm::vec3 desiredPosition);
};

Camera camera;

void Camera::initialize() {
    
    camera = Camera();
    
    camera.lockCamera      = false;
    camera.firstPersonView = false;
    camera.cameraPosition  = glm::vec3(0.0f, 6.0f, 4.0f);
    camera.lookDirection   = glm::vec3(0.0f, 0.0f, -1.0f);
    camera.velocity        = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.fov             = 3.14159265358f/2.0f;
    camera.near            = 0.1f;
    camera.far             = 1000.0f;
    
    int width, height;
    glfwGetWindowSize(context->window, &width, &height);
    camera.aspect = (float)width/(float)height;
    
    camera.cameraProjection = glm::perspective(camera.fov, camera.aspect, camera.near, camera.far);
    camera.cameraProjection[1][1] *= -1;
    camera.right = glm::normalize(glm::cross(camera.lookDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
}

void Camera::update(glm::vec4 movement) {
    
    if (firstPersonView) {
        // lock cursor to center
    }
    
    if (!lockCamera) {
        float forward   = movement.x;
        float backward  = movement.y;
        
        float left      = movement.z;
        float right     = movement.w;
        
        glm::vec3 motion = lookDirection;
        this->right = glm::normalize(glm::cross(motion, glm::vec3(0.0, 1.0, 0.0)));
        
        glm::vec3 totalMotion = motion * (forward + backward) - glm::normalize(glm::cross(motion, glm::vec3(0.0, 1.0, 0.0))) * (left + right);
        glm::vec3 normalizedMotion = glm::vec3(0.0f);
        
        if (glm::length(totalMotion) > 0.0f) normalizedMotion = glm::normalize(totalMotion);
        
        cameraPosition += normalizedMotion * speed * deltaTime;
        
        lookDirection = glm::normalize(glm::vec3(
                                       cos(camera.yaw) * cos(camera.pitch),
                                       sin(camera.pitch),
                                       sin(camera.yaw) * cos(camera.pitch)
                                       ));
        
        cameraLookAt = glm::lookAt(cameraPosition, cameraPosition + lookDirection, glm::vec3(0.0, 1.0, 0.0));
        
        int width, height;
        glfwGetWindowSize(context->window, &width, &height);
        aspect = (float)width/(float)height;
        
        cameraProjection = glm::perspective(fov, aspect, near, far);
        camera.cameraProjection[1][1] *= -1;
        
        velocity = totalMotion * speed * deltaTime;
    }
}

void Camera::updateLookAt() {
    cameraLookAt = glm::lookAt(cameraPosition, cameraPosition + lookDirection, glm::vec3(0.0, 1.0, 0.0));
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    
    if (camera.firstPersonView) {
        
    }
    
    else {
        if (glfwGetMouseButton(context->window, camera.mouseButton)) {
            
            float deltaX = xpos - camera.lastMouseX;
            float deltaY = ypos - camera.lastMouseY;
            
            camera.pitch -= deltaY * 0.005f;
            camera.yaw += deltaX * 0.005f;
            
            if (camera.pitch >  1.55f) camera.pitch =  1.55f;
            if (camera.pitch < -1.55f) camera.pitch = -1.55f;
            
            camera.lookDirection = glm::normalize(glm::vec3(
                                                cos(camera.yaw) * cos(camera.pitch),
                                                sin(camera.pitch),
                                                sin(camera.yaw) * cos(camera.pitch)
                                                ));
        }
    }
    
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    
    float x = 2.0f * camera.lastMouseX / (float)width - 1.0f;
    float y = 1.0f - (2.0f * camera.lastMouseY) / (float)height;
        
    glm::vec3 nearPlane = glm::vec3(x, y, 0.0f);
    glm::vec3 farPlane = glm::vec3(x, y, 1.0f);
    
    glm::mat4 newProjection = camera.cameraProjection;
    newProjection[1][1] *= -1;
    
    glm::mat4 inverseProjection = glm::inverse(newProjection);
    
    glm::vec4 nearWorld = inverseProjection * glm::vec4(nearPlane, 1.0f);
    glm::vec4 farWorld = inverseProjection * glm::vec4(farPlane, 1.0f);
    nearWorld /= nearWorld.w;
    farWorld /= farWorld.w;
    
    glm::vec3 rayDirection = glm::normalize(glm::vec3(farWorld) - glm::vec3(nearWorld));
    camera.mouseRay = glm::normalize(glm::vec3(glm::inverse(camera.cameraLookAt) * glm::vec4(rayDirection, 0.0f)));
    
    camera.lastMouseX = xpos;
    camera.lastMouseY = ypos;
}

std::vector<float> Camera::GetColliderVertices(glm::vec3 desiredPosition = glm::vec3(0.0f)) {
    
    glm::vec3 position = cameraPosition;
    if (glm::length(desiredPosition) != 0) {
        std::cout << desiredPosition.x << " " << desiredPosition.y << " " << desiredPosition.z << "\n";
        position = desiredPosition;
    }
    
    glm::mat4 model = anopol::modelMatrix(position, glm::vec3(0.75f), glm::vec3(0.0f));
    
    std::vector<float> projectedVertices = std::vector<float>();
    
    for (int i = 0; i < vertices.size()/3; i++) {
        glm::vec3 vertex = glm::vec3(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
        glm::vec3 projected = glm::vec3(model * glm::vec4(vertex, 1.0));
        projectedVertices.push_back(projected.x);
        projectedVertices.push_back(projected.y);
        projectedVertices.push_back(projected.z);
    }
    return projectedVertices;
    
}

}

#endif /* camera_h */
