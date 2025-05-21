//
//  anopol.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-14.
//

#include <map>
#include <vector>
#include <string>
#include <optional>
#include <set>
#include <array>
#include <fstream>
#include <thread>
#include <mutex>
#include <future>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp>


#define anopol_assert(message)      throw std::runtime_error(message)
#define anopol_max_frames           3
#define anopol_max_cascades         4
#define deltaTimeMultiplier         30.0f
#define golden_ratio                static_cast<float>((1 + sqrt(5)) / 2.0f)
#define inverse_golden_ratio        1.0f / golden_ratio

float debugTime = 0;
float deltaTime = 0;

namespace anopol {
    
struct anopolContext {
    VkInstance          instance;
    
    VkPhysicalDevice    physicalDevice;
    VkDevice            device;
    
    VkQueue             graphicsQueue,
                        presentQueue;
    
    VkSurfaceKHR        surface;
    VkSwapchainKHR      swapchain;
    VkExtent2D          extent;
    VkFormat            format;
    
    GLFWwindow*         window;
    VkDebugUtilsMessengerEXT debug;
    
    std::mutex graphicsQueueMutex;
};

struct swapchainDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

struct queueFamily {
    std::optional<uint32_t> graphicsFamily,
                            presentQueue;
};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#if defined(__APPLE__)
    "VK_KHR_portability_subset"
#endif
};

#if defined(NDEBUG)
const bool validation = true;
#else
const bool validation = false;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT callbackData,
                                                    void* userData)
{
    std::cerr << callbackData.pMessage << '\n';
    return VK_FALSE;
}


bool checkValidationLayerSupport() {
    
    uint32_t layerCount;
    
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    
    std::vector<VkLayerProperties> properties;
    properties.resize(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, properties.data());
    
    for (const char* layer : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperty : properties) {
            if (strcmp(layer, layerProperty.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        
        if (!layerFound) return false;
    }
    return true;
}

glm::mat4 eulerRotation(glm::vec3 rotationDegrees) {
    
    float xangle = (rotationDegrees.x / 180.0f) * 3.14159265358f;
    float cosx = cos(xangle);
    float sinx = sin(xangle);
    
    float yangle = (rotationDegrees.y / 180.0f) * 3.14159265358f;
    float cosy = cos(yangle);
    float siny = sin(yangle);
    
    float zangle = (rotationDegrees.z / 180.0f) * 3.14159265358f;
    float cosz = cos(zangle);
    float sinz = sin(zangle);
    
    glm::mat4 xRotation = glm::mat4(glm::vec4( 1.0f,  0.0f,  0.0f,  0.0f),
                                    glm::vec4( 0.0f,  cosx, -sinx,  0.0f),
                                    glm::vec4( 0.0f,  sinx,  cosx,  0.0f),
                                    glm::vec4( 0.0f,  0.0f,  0.0f,  1.0f));
    
    glm::mat4 yRotation = glm::mat4(glm::vec4( cosy,  0.0f,  siny,  0.0f),
                                    glm::vec4( 0.0f,  1.0f,  0.0f,  0.0f),
                                    glm::vec4(-siny,  0.0f,  cosy,  0.0f),
                                    glm::vec4( 0.0f,  0.0f,  0.0f,  1.0f));
    
    glm::mat4 zRotation = glm::mat4(glm::vec4( cosz, -sinz,  0.0f,  0.0f),
                                    glm::vec4( sinz,  cosz,  0.0f,  0.0f),
                                    glm::vec4( 0.0f,  0.0f,  1.0f,  0.0f),
                                    glm::vec4( 0.0f,  0.0f,  0.0f,  1.0f));
    
    return yRotation * xRotation * zRotation;
}

glm::mat4 quaternionRotation(glm::vec3 rotationDegrees) {
    glm::quat q = glm::quat(glm::radians(rotationDegrees));
    return glm::mat4_cast(q);
}

glm::mat4 modelMatrix(glm::vec3 position, glm::vec3 scale, glm::vec3 rotation) {
    
    glm::mat4 translationMatrix = glm::mat4(1.0f);
    translationMatrix = glm::translate(translationMatrix, position);
    
    glm::mat4 scaleMatrix = glm::mat4(1.0f);
    scaleMatrix = glm::scale(scaleMatrix, scale);
    
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0)) *
                                   glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1, 0)) *
                                   glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1));
    
    return translationMatrix * rotationMatrix * scaleMatrix;
}

}
