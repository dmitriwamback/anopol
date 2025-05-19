//
//  combine.m
//  anopol
//
//  Created by Dmitri Wamback on 2025-05-01.
//

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#include <vulkan/vulkan.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "../../../render/vertex.h"

namespace anopol::metal {

class BatchCombineImplementation {
public:
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    id<MTLComputePipelineState> pipeline;
    
    BatchCombineImplementation();
    void Add(float* a, float* b, glm::vec4* result, int count);
};

}
