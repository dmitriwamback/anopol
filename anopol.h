//
//  anopol.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-14.
//

#ifndef anopol_h
#define anopol_h

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>

#include "anopol_definitions.h"

static anopol::anopolContext* context;
std::array<VkWriteDescriptorSet, 4> GLOBAL_PIPELINE_DESCRIPTOR_SETS{};
VkDescriptorSetLayoutBinding GLOBAL_TEXTURE_BINDING{};
VkDescriptorSetLayoutBinding GLOBAL_INSTANCE_BINDING{};
VkDescriptorSetLayoutBinding GLOBAL_UNIFORM_BUFFER_BINDING{};
VkDescriptorSetLayoutBinding GLOBAL_BATCHING_BINDING{};
VkDescriptorSetLayout        GLOBAL_ANOPOL_DESCRIPTOR_SET_LAYOUT{};

anopol::descriptorSets* ANOPOL_DESCRIPTOR_SETS;

#include "core/pipeline/lighting.h"
#include "core/math/math.h"

#include "ll/mem.h"
#include "ll/internal.h"

#define STB_IMAGE_IMPLEMENTATION
#include "core/render/texture/stb_image.h"
#include "core/render/texture/material.h"
#include "core/render/texture/texture.h"

#include "core/structs/shadow.h"

#include "core/render/vertex.h"

#include "core/render/buffer/vertex_buffer.h"
#include "core/render/buffer/index_buffer.h"
#include "core/render/buffer/instance_buffer.h"
#include "core/render/buffer/push_constants.h"

#include "core/render/renderable.h"
#include "core/render/asset.h"

#include "core/camera/ray.h"
#include "core/camera/camera.h"
#include "core/camera/frustum.h"
#include "core/render/buffer/uniform_buffer.h"

#if defined(__APPLE__)
#define APPLE_USE_METAL_GPU_HELPERS
//#define APPLE_USE_OPENCL_GPU_HELPERS
#include "core/batch/bgpu/mac/macos_batch_combine_wrapper.h"
#endif

#include "core/batch/mesh_combine_structs.h"
#include "core/batch/mesh_combine.h"
#include "core/batch/batch.h"
#include "core/batch/dynamic_upload.h"

#include "core/render/offscreen.h"

#include "core/gjkepa/support.h"
#include "core/gjkepa/simplex.h"
#include "core/gjkepa/expanding_polytope_algorithm.h"
#include "core/gjkepa/gilbert_johnson_keerthi.h"

#include "core/pipeline/collision_ray_thread.h"

#include "core/pipeline/pipeline_util.h"
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
    
    
    ANOPOL_DESCRIPTOR_SETS = static_cast<anopol::descriptorSets*>(malloc(1 * sizeof(anopol::descriptorSets)));
    std::array<VkDescriptorPoolSize, 4> poolSizes{};
    
    poolSizes[0].type                   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // Uniform Buffer
    poolSizes[0].descriptorCount        = (uint32_t)anopol_max_frames;
    poolSizes[1].type                   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // Instance Buffer
    poolSizes[1].descriptorCount        = (uint32_t)anopol_max_frames;
    poolSizes[2].type                   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // Batching Buffer
    poolSizes[2].descriptorCount        = (uint32_t)anopol_max_frames;
    poolSizes[3].type                   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // Texture Buffer
    poolSizes[3].descriptorCount        = 1024;
    
    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount    = static_cast<uint32_t>(poolSizes.size());
    poolCreateInfo.pPoolSizes       = poolSizes.data();
    poolCreateInfo.maxSets          = (uint32_t)anopol_max_frames + 4;
    
    if (vkCreateDescriptorPool(context->device, &poolCreateInfo, nullptr, &ANOPOL_DESCRIPTOR_SETS->descriptorPool) != VK_SUCCESS) anopol_assert("Failed to create descriptor pool");
    
    GLOBAL_INSTANCE_BINDING.binding                     = 1;
    GLOBAL_INSTANCE_BINDING.descriptorType              = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    GLOBAL_INSTANCE_BINDING.descriptorCount             = 1;
    GLOBAL_INSTANCE_BINDING.stageFlags                  = VK_SHADER_STAGE_VERTEX_BIT;
    
    GLOBAL_UNIFORM_BUFFER_BINDING.binding               = 2;
    GLOBAL_UNIFORM_BUFFER_BINDING.descriptorType        = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    GLOBAL_UNIFORM_BUFFER_BINDING.descriptorCount       = 1;
    GLOBAL_UNIFORM_BUFFER_BINDING.stageFlags            = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    
    GLOBAL_BATCHING_BINDING.binding                     = 3;
    GLOBAL_BATCHING_BINDING.descriptorType              = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    GLOBAL_BATCHING_BINDING.descriptorCount             = 1;
    GLOBAL_BATCHING_BINDING.stageFlags                  = VK_SHADER_STAGE_VERTEX_BIT;
    
    GLOBAL_TEXTURE_BINDING.binding                      = 4;
    GLOBAL_TEXTURE_BINDING.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    GLOBAL_TEXTURE_BINDING.descriptorCount              = anopol_max_textures;
    GLOBAL_TEXTURE_BINDING.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    GLOBAL_TEXTURE_BINDING.pImmutableSamplers           = nullptr;
    
    VkDescriptorSetLayoutBinding bindings[] = {GLOBAL_UNIFORM_BUFFER_BINDING, GLOBAL_INSTANCE_BINDING, GLOBAL_BATCHING_BINDING, GLOBAL_TEXTURE_BINDING};
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 4;
    layoutInfo.pBindings    = bindings;
    
    if (vkCreateDescriptorSetLayout(context->device, &layoutInfo, nullptr, &GLOBAL_ANOPOL_DESCRIPTOR_SET_LAYOUT) != VK_SUCCESS) anopol_assert("Failed to create descriptor");
    
    std::vector<VkDescriptorSetLayout> descriptorLayouts(anopol_max_frames, GLOBAL_ANOPOL_DESCRIPTOR_SET_LAYOUT);
    
    VkDescriptorSetAllocateInfo descriptorAllocationInfo{};
    descriptorAllocationInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorAllocationInfo.descriptorPool     = ANOPOL_DESCRIPTOR_SETS->descriptorPool;
    descriptorAllocationInfo.descriptorSetCount = (uint32_t)anopol_max_frames;
    descriptorAllocationInfo.pSetLayouts        = descriptorLayouts.data();
    
    ANOPOL_DESCRIPTOR_SETS->descriptorSets.resize(anopol_max_frames);
    if (vkAllocateDescriptorSets(context->device, &descriptorAllocationInfo, ANOPOL_DESCRIPTOR_SETS->descriptorSets.data()) != VK_SUCCESS) anopol_assert("Failed to allocate descriptor sets");
    
    
    anopol::pipeline::Pipeline pipeline = anopol::pipeline::Pipeline::CreatePipeline("/Users/dmitriwamback/Documents/Projects/anopol/anopol/shaders/main");
    
    double previousTime = glfwGetTime();
    double previousDeltaTime = glfwGetTime();
    int frameCount = 0;
    
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
        
        double currentTime = glfwGetTime();
        frameCount++;
        
        if (currentTime - previousTime >= 1.0) {

            glfwSetWindowTitle(context->window, ("Anopol FPS: " + std::to_string(frameCount)).c_str());

            frameCount = 0;
            previousTime = currentTime;
        }
                
        debugTime += 0.1f;
        
        double currentDeltatime = glfwGetTime();
        deltaTime = (currentDeltatime - previousDeltaTime);
        previousDeltaTime = currentDeltatime;
    }
    
    vkDeviceWaitIdle(context->device);
    
    pipeline.CleanUp();
    vkDestroyDescriptorPool(context->device, ANOPOL_DESCRIPTOR_SETS->descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(context->device, GLOBAL_ANOPOL_DESCRIPTOR_SET_LAYOUT, nullptr);
    free(ANOPOL_DESCRIPTOR_SETS);
    
    anopol::ll::freeMemory();
}
}

#endif /* anopol_h */
