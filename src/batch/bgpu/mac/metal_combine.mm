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
#include "../../../core/vertex.h"
#include "macos_batch_combine_wrapper.h"

namespace anopol::metal {

class BatchCombineImplementation {
public:
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    id<MTLComputePipelineState> pipeline;
    
    BatchCombineImplementation();
    void Combine(std::vector<anopol::render::Vertex> vertices);
};

BatchCombineImplementation::BatchCombineImplementation() {
    device = MTLCreateSystemDefaultDevice();
    commandQueue = [device newCommandQueue];
    
    NSString *path = @"/Users/dmitriwamback/Documents/Projects/anopol/anopol/core/batch/bgpu/mac/mtl_batch_compute.metallib";
    NSData *data = [NSData dataWithContentsOfFile:path];
    dispatch_data_t dispatchData = dispatch_data_create(data.bytes, data.length, NULL, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    
    NSError *error = nil;
    id<MTLLibrary> library = [device newLibraryWithData:dispatchData error:&error];
    id<MTLFunction> function = [library newFunctionWithName:@"batchMergeVertices"];
    pipeline = [device newComputePipelineStateWithFunction:function error:&error];
}

void BatchCombineImplementation::Combine(std::vector<anopol::render::Vertex> vertices) {
    
    
}

}
