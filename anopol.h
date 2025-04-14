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

#include "core/math/math.h"

#include "ll/mem.h"
#include "ll/internal.h"

#include "core/camera/frustum.h"
#include "core/camera/camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "core/render/texture/stb_image.h"
#include "core/render/texture/material.h"
#include "core/render/texture/texture.h"

#include "core/structs/shadow.h"

#include "core/render/vertex.h"
#include "core/render/buffer/uniform_buffer.h"
#include "core/render/buffer/vertex_buffer.h"
#include "core/render/buffer/index_buffer.h"
#include "core/render/buffer/instance_buffer.h"
#include "core/render/buffer/push_constants.h"

#include "core/render/renderable.h"
#include "core/render/asset.h"

#include "core/meshutil/mesh_combine_structs.h"
#include "core/meshutil/batch.h"
#include "core/meshutil/mesh_combine.h"
#include "core/meshutil/dynamic_upload.h"

#include "core/gjkepa/support.h"
#include "core/gjkepa/simplex.h"
#include "core/gjkepa/expanding_polytope_algorithm.h"
#include "core/gjkepa/gilbert_johnson_keerthi.h"

#include "core/pipeline/pipeline.h"
#include "core/pipeline/scene.h"

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
    
    instanceInfo.enabledLayerCount      = static_cast<uint32_t>(validationLayers.size());
    instanceInfo.ppEnabledLayerNames    = validationLayers.data();
    
    if (vkCreateInstance(&instanceInfo, nullptr, &context->instance) != VK_SUCCESS) anopol_assert("Couldn't create VkInstance");
    
    glfwCreateWindowSurface(context->instance, context->window, nullptr, &context->surface);
    
    anopol::camera::Camera::initialize();
    glfwSetCursorPosCallback(context->window, anopol::camera::cursor_position_callback);
    
    anopol::ll::initializeVulkanDependenices();
    
    anopol::pipeline::Pipeline pipeline = anopol::pipeline::Pipeline::CreatePipeline("/Users/dmitriwamback/Documents/Projects/anopol/anopol/shaders/main");
    
    while (!glfwWindowShouldClose(context->window)) {
        pipeline.currentFrame = (pipeline.currentFrame + 1) % anopol_max_frames;
        
        glm::vec4 movement = glm::vec4(0.0f);

        movement.z = glfwGetKey(context->window, GLFW_KEY_A) == GLFW_PRESS ?  0.05f : 0;
        movement.w = glfwGetKey(context->window, GLFW_KEY_D) == GLFW_PRESS ? -0.05f : 0;
        movement.x = glfwGetKey(context->window, GLFW_KEY_W) == GLFW_PRESS ?  0.05f : 0;
        movement.y = glfwGetKey(context->window, GLFW_KEY_S) == GLFW_PRESS ? -0.05f : 0;
        
        anopol::camera::camera.update(movement);
        
        glfwPollEvents();
        pipeline.Bind("test");
        
        debugTime += 0.1f;
    }
    
    vkDeviceWaitIdle(context->device);
    
    pipeline.CleanUp();
    anopol::ll::freeMemory();
}
}

#endif /* anopol_h */
