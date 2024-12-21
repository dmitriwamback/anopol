//
//  main.cpp
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-14.
//

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>

#include "anopol.h"

int main(int argc, const char * argv[]) {
    
    if (!glfwInit()) return -1;
    
    anopol::initialize();
}
