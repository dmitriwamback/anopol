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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>


#define anopol_assert(message) throw std::runtime_error(message)

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
};

struct swapchainDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

struct queueFamily {
    std::optional<uint32_t> graphicsFamily,
                            presentFamily;
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

}
