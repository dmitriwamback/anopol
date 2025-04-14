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
    
    std::vector<float> vertices = {
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
    };
    
    glm::vec3 cameraPosition, lookDirection;
    glm::mat4 cameraProjection, cameraLookAt;
    
    float pitch;
    float yaw = 3.0f * 3.141592653f/2.0f;
    
    float lastMouseX;
    float lastMouseY;
    
    bool lockCamera;
    bool firstPersonView;
    
    int mouseButton = GLFW_MOUSE_BUTTON_RIGHT;
    
    static void initialize();
    void update(glm::vec4 movement);
    void updateLookAt();
    std::vector<float> GetColliderVertices();
};

Camera camera;

void Camera::initialize() {
    
    camera = Camera();
    
    camera.lockCamera      = false;
    camera.firstPersonView = false;
    camera.cameraPosition  = glm::vec3(0.0f, 6.0f, 4.0f);
    camera.lookDirection   = glm::vec3(0.0f, 0.0f, -1.0f);
    
    camera.cameraProjection = glm::perspective(3.14159265358f/2.0f, 3.0f/2.0f, 0.01f, 1000.0f);
    camera.cameraProjection[1][1] *= -1;
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
        
        cameraPosition += motion * (forward + backward) * 10.0f;
        cameraPosition -= glm::normalize(glm::cross(motion, glm::vec3(0.0, 1.0, 0.0))) * (left + right) * 10.0f;
        
        lookDirection = glm::normalize(glm::vec3(
                                       cos(camera.yaw) * cos(camera.pitch),
                                       sin(camera.pitch),
                                       sin(camera.yaw) * cos(camera.pitch)
                                       ));
        
        cameraLookAt = glm::lookAt(cameraPosition, cameraPosition + lookDirection, glm::vec3(0.0, 1.0, 0.0));
        
        int width, height;
        glfwGetWindowSize(context->window, &width, &height);
        float aspect = (float)width/(float)height;
        
        cameraProjection = glm::perspective(3.14159265358f/2.0f, aspect, 0.1f, 1000.0f);
        camera.cameraProjection[1][1] *= -1;
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
    camera.lastMouseX = xpos;
    camera.lastMouseY = ypos;
}

std::vector<float> Camera::GetColliderVertices() {
    
    glm::mat4 model = anopol::modelMatrix(cameraPosition, glm::vec3(1.0f), glm::vec3(0.0f));
    
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
