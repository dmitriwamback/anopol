//
//  anopol.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-14.
//

#ifndef anopol_h
#define anopol_h

#include "anopol_definitions.h"
static anopol::anopolContext* context;

#include "ll/internal.h"
#include "core/render/vertex.h"

namespace anopol {


void initialize() {
    
    context = static_cast<anopolContext*>(malloc(1 * sizeof(anopolContext)));
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    context->window = glfwCreateWindow(1200, 800, "Anopol", nullptr, nullptr);
    
    VkApplicationInfo app{};
    app.sType                       = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName            = "Anopol";
    app.pEngineName                 = "Anopol";
    app.applicationVersion          = VK_MAKE_VERSION(1, 0, 0);
    app.engineVersion               = VK_MAKE_VERSION(1, 0, 0);
    app.apiVersion                  = VK_API_VERSION_1_0;
    
    uint32_t extensionCount;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    
    std::vector<const char*> requiredExtensions;
    for (int i = 0; i < extensionCount; i++) {
        requiredExtensions.emplace_back(extensions[i]);
    }
    
    if (validation) {
        requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
#if defined(__APPLE__)
    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    requiredExtensions.emplace_back("VK_KHR_get_physical_device_properties2");
#endif
    
    
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo           = &app;
    instanceInfo.enabledExtensionCount      = (uint32_t)requiredExtensions.size();
    instanceInfo.ppEnabledExtensionNames    = requiredExtensions.data();
    instanceInfo.enabledLayerCount          = 0;
#if defined(__APPLE__)
    instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    
    if (validation) {
        instanceInfo.enabledLayerCount      = static_cast<uint32_t>(validationLayers.size());
        instanceInfo.ppEnabledLayerNames    = validationLayers.data();
    }
    
    if (vkCreateInstance(&instanceInfo, nullptr, &context->instance) != VK_SUCCESS) anopol_assert("Couldn't create VkInstance");
    
    glfwCreateWindowSurface(context->instance, context->window, nullptr, &context->surface);
    
    anopol::ll::initializeVulkanDependenices();
}
}

#endif /* anopol_h */
