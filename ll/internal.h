//
//  internal.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-14.
//

#ifndef internal_h
#define internal_h

namespace anopol::ll {

std::vector<VkImage> swapchainImages;
std::vector<VkImageView> swapchainImageViews;

std::vector<VkCommandBuffer> commandbuffers = std::vector<VkCommandBuffer>();
VkRenderPass renderpass;

VkCommandPool   commandPool;
VkImage         depthImage;
VkDeviceMemory  depthImageMemory;
VkImageView     depthImageView, textureImageView;

struct msaaMultisampling {
    VkImage         image;
    VkDeviceMemory  mem;
    VkImageView     view;
};
msaaMultisampling multisampling;
VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    
    for (VkFormat format : candidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(context->physicalDevice, format, &properties);
        
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    anopol_assert("failed to find format");
}

bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

//------------------------------------------------------------------------------------------//
// Multisampling
//------------------------------------------------------------------------------------------//

VkSampleCountFlagBits getSampleCount() {
    
    VkPhysicalDeviceProperties physicalDeviceProperties;
    
    vkGetPhysicalDeviceProperties(context->physicalDevice, &physicalDeviceProperties);
    VkSampleCountFlags count = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    
    VkSampleCountFlags samples[] = {
        VK_SAMPLE_COUNT_64_BIT, VK_SAMPLE_COUNT_32_BIT,
        VK_SAMPLE_COUNT_16_BIT, VK_SAMPLE_COUNT_8_BIT,
        VK_SAMPLE_COUNT_4_BIT,  VK_SAMPLE_COUNT_2_BIT,
    };
    
    if (count & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
    if (count & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
    if (count & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
    if (count & VK_SAMPLE_COUNT_8_BIT)  return VK_SAMPLE_COUNT_8_BIT;
    if (count & VK_SAMPLE_COUNT_4_BIT)  return VK_SAMPLE_COUNT_4_BIT;
    if (count & VK_SAMPLE_COUNT_2_BIT)  return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}

//------------------------------------------------------------------------------------------//
// Commandbuffer
//------------------------------------------------------------------------------------------//

VkCommandBuffer beginSingleCommandBuffer() {
    
    VkCommandBufferAllocateInfo allocationInfo{};
    allocationInfo.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocationInfo.level                = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocationInfo.commandPool          = commandPool;
    allocationInfo.commandBufferCount   = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context->device, &allocationInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo begin{};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &begin);
    
    return commandBuffer;
}

void endSingleCommandBuffer(VkCommandBuffer commandBuffer) {
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submit{};
    submit.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount   = 1;
    submit.pCommandBuffers      = &commandBuffer;
    
    vkQueueSubmit(context->graphicsQueue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(context->graphicsQueue);
    
    vkFreeCommandBuffers(context->device, commandPool, 1, &commandBuffer);
}

//------------------------------------------------------------------------------------------//
// Image Layout Transition
//------------------------------------------------------------------------------------------//

void imageLayoutTransition(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    
    VkCommandBuffer commandBuffer = beginSingleCommandBuffer();
    
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType                            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.oldLayout                        = oldLayout;
    imageMemoryBarrier.newLayout                        = newLayout;
    imageMemoryBarrier.srcQueueFamilyIndex              = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex              = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image                            = image;
    imageMemoryBarrier.subresourceRange.baseMipLevel    = 0;
    imageMemoryBarrier.subresourceRange.levelCount      = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer  = 0;
    imageMemoryBarrier.subresourceRange.layerCount      = 1;
    
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        imageMemoryBarrier.subresourceRange.aspectMask  = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format)) {
            imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } 
    else {
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    
    VkPipelineStageFlags src, dst;
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        src = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } 
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        src = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        anopol_assert("Unsupported layout");
    }
    vkCmdPipelineBarrier(commandBuffer, src, dst, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
    
    endSingleCommandBuffer(commandBuffer);
}

//------------------------------------------------------------------------------------------//
// Buffers
//------------------------------------------------------------------------------------------//

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size         = size;
    bufferInfo.usage        = usageFlags;
    bufferInfo.sharingMode  = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(context->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) anopol_assert("Failed to create buffer");

    VkMemoryRequirements mem;
    vkGetBufferMemoryRequirements(context->device, buffer, &mem);

    VkMemoryAllocateInfo allocationInfo{};
    allocationInfo.sType            = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocationInfo.allocationSize   = mem.size;
    allocationInfo.memoryTypeIndex  = findMemoryType(mem.memoryTypeBits, properties);

    if (vkAllocateMemory(context->device, &allocationInfo, nullptr, &bufferMemory) != VK_SUCCESS) anopol_assert("Failed to allocate memory");
    vkBindBufferMemory(context->device, buffer, bufferMemory, 0);
}

void memCopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    
    VkCommandBuffer commandBuffer = beginSingleCommandBuffer();
    
    VkBufferCopy copy{};
    copy.size = size;
    vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copy);
    
    endSingleCommandBuffer(commandBuffer);
}

void memCopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    
    VkCommandBuffer commandBuffer = beginSingleCommandBuffer();
    
    VkBufferImageCopy region{};
    
    region.bufferOffset                     = 0;
    region.bufferRowLength                  = 0;
    region.bufferImageHeight                = 0;
    
    region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel        = 0;
    region.imageSubresource.baseArrayLayer  = 0;
    region.imageSubresource.layerCount      = 1;
    
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width, height, 1
    };
    
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    endSingleCommandBuffer(commandBuffer);
}

VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t numLayers = 1, uint32_t arrayLayer = 0) {
    
    VkImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image                            = image;
    viewCreateInfo.viewType                         = viewType;
    viewCreateInfo.format                           = format;
    viewCreateInfo.subresourceRange.aspectMask      = flags;
    viewCreateInfo.subresourceRange.baseMipLevel    = 0;
    viewCreateInfo.subresourceRange.levelCount      = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer  = arrayLayer;
    viewCreateInfo.subresourceRange.layerCount      = numLayers;
    
    VkImageView imageView;
    if (vkCreateImageView(context->device, &viewCreateInfo, nullptr, &imageView) != VK_SUCCESS) anopol_assert("failed to create image view");
    
    return imageView;
}

//------------------------------------------------------------------------------------------//
// Image
//------------------------------------------------------------------------------------------//

void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t arrayLayers = 1) {
    
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType           = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType       = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width    = width;
    imageCreateInfo.extent.height   = height;
    imageCreateInfo.extent.depth    = 1;
    imageCreateInfo.mipLevels       = 1;
    imageCreateInfo.arrayLayers     = arrayLayers;
    imageCreateInfo.format          = format;
    imageCreateInfo.tiling          = tiling;
    imageCreateInfo.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage           = usage;
    imageCreateInfo.samples         = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode     = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(context->device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS) anopol_assert("Failed to create image");

    VkMemoryRequirements mem;
    vkGetImageMemoryRequirements(context->device, image, &mem);

    VkMemoryAllocateInfo allocationInfo{};
    allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocationInfo.allocationSize = mem.size;
    allocationInfo.memoryTypeIndex = findMemoryType(mem.memoryTypeBits, properties);

    if (vkAllocateMemory(context->device, &allocationInfo, nullptr, &imageMemory) != VK_SUCCESS) anopol_assert("failed to allocate image memory!");

    vkBindImageMemory(context->device, image, imageMemory, 0);
}

//------------------------------------------------------------------------------------------//
// Depth
//------------------------------------------------------------------------------------------//

VkFormat findDepthFormat() {
    return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void createDepth() {
    
    VkFormat depthFormat = findDepthFormat();
    createImage(context->extent.width, context->extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    
    imageLayoutTransition(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

//------------------------------------------------------------------------------------------//
// QueueFamily
//------------------------------------------------------------------------------------------//

queueFamily findQueueFamily(VkPhysicalDevice device) {
    
    queueFamily family;
    
    uint32_t familyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> families;
    families.resize(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());
    
    int i = 0;
    for (const auto& queue : families) {
        
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) family.graphicsFamily = i;
        
        VkBool32 present;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, context->surface, &present);
        if (present) {
            family.presentQueue = i;
        }
        
        if (family.graphicsFamily.has_value() && family.presentQueue.has_value()) {
            return family;
        }
        i++;
    }
    return family;
}

swapchainDetails querySwapchainDetails(VkPhysicalDevice device) {
    swapchainDetails details;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, context->surface, &details.capabilities);
    
    uint32_t formatCount, presentModeCount;
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, context->surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, context->surface, &presentModeCount, details.presentModes.data());
    }
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, context->surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, context->surface, &formatCount, details.formats.data());
    }
        
    return details;
}

//------------------------------------------------------------------------------------------//
// Physical Devices
//------------------------------------------------------------------------------------------//

bool isPhysicalDeviceSuitable(VkPhysicalDevice device) {
    
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures   features;
    
    vkGetPhysicalDeviceProperties(device, &properties);
    vkGetPhysicalDeviceFeatures(device, &features);
    
    swapchainDetails details = querySwapchainDetails(device);
    
    bool adequate = false;
    adequate = !details.formats.empty() && !details.presentModes.empty();
    queueFamily family = findQueueFamily(device);
    return family.graphicsFamily.has_value() && family.presentQueue.has_value() && adequate;
}

VkPhysicalDevice findPhysicalDevice(std::vector<VkPhysicalDevice> devices) {
    
    for (VkPhysicalDevice device : devices) {
        
        if (isPhysicalDeviceSuitable(device)) {
            return device;
        }
    }
    
    anopol_assert("Could not find a suitable VkPhysicalDevice");
    return VK_NULL_HANDLE;
}


//------------------------------------------------------------------------------------------//
// Device
//------------------------------------------------------------------------------------------//

void createDevice() {
    std::vector<VkDeviceQueueCreateInfo> queueInfo;
    queueFamily family = findQueueFamily(context->physicalDevice);
    
    for (uint32_t qFamily : std::set<uint32_t>{family.graphicsFamily.value(), family.presentQueue.value()}) {
        
        float priority = 1.0f;
        
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType               = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex    = qFamily;
        queueCreateInfo.queueCount          = 1;
        queueCreateInfo.pQueuePriorities    = &priority;
        
        queueInfo.push_back(queueCreateInfo);
    }
    
    VkPhysicalDeviceFeatures features{};
    VkDeviceCreateInfo deviceInfo{};
    
    deviceInfo.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pQueueCreateInfos        = queueInfo.data();
    deviceInfo.queueCreateInfoCount     = 1;
    deviceInfo.pEnabledFeatures         = &features;
    deviceInfo.ppEnabledExtensionNames  = deviceExtensions.data();
    deviceInfo.enabledExtensionCount    = (uint32_t)deviceExtensions.size();
    
    if (vkCreateDevice(context->physicalDevice, &deviceInfo, nullptr, &context->device) != VK_SUCCESS) {
        anopol_assert("Couldn't create logical device");
    }
    
    vkGetDeviceQueue(context->device, family.graphicsFamily.value(), 0, &context->graphicsQueue);
    vkGetDeviceQueue(context->device, family.presentQueue.value(), 0, &context->presentQueue);
}

//------------------------------------------------------------------------------------------//
// Swapchain
//------------------------------------------------------------------------------------------//

VkSurfaceFormatKHR chooseSwapchainSurface(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    
    for (const auto& format : availableFormats) {
        if (format.format == VK_FORMAT_B8G8R8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    
    for (const auto& presentMode : availablePresentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) return presentMode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;
    
    int width, height;
    
    glfwGetFramebufferSize(context->window, &width, &height);
    VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.width = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    
    return extent;
}

void createSwapchain() {
    
    swapchainDetails details    = querySwapchainDetails(context->physicalDevice);
    VkSurfaceFormatKHR sformat  = chooseSwapchainSurface(details.formats);
    VkPresentModeKHR pMode      = chooseSwapchainPresentMode(details.presentModes);
    VkExtent2D extent           = chooseSwapchainExtent(details.capabilities);
    
    context->extent = extent;
    context->format = sformat.format;
    
    uint32_t imageCount = details.capabilities.minImageCount + 1;
    
    if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) imageCount = details.capabilities.maxImageCount;
    
    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType             = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface           = context->surface;
    swapchainInfo.minImageCount     = imageCount;
    swapchainInfo.imageFormat       = sformat.format;
    swapchainInfo.imageColorSpace   = sformat.colorSpace;
    swapchainInfo.imageExtent       = extent;
    swapchainInfo.imageArrayLayers  = 1;
    swapchainInfo.imageUsage        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    queueFamily indices = findQueueFamily(context->physicalDevice);
    
    if (indices.presentQueue != indices.graphicsFamily) {
        uint32_t q[] = { indices.graphicsFamily.value(), indices.presentQueue.value() };
        swapchainInfo.imageSharingMode          = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount     = 2;
        swapchainInfo.pQueueFamilyIndices       = q;
    }
    else {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    
    swapchainInfo.pQueueFamilyIndices   = nullptr;
    swapchainInfo.preTransform          = details.capabilities.currentTransform;
    swapchainInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode           = pMode;
    swapchainInfo.clipped               = VK_TRUE;
    swapchainInfo.oldSwapchain          = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(context->device, &swapchainInfo, nullptr, &context->swapchain) != VK_SUCCESS) anopol_assert("Couldn't create swapchain");
    
    uint32_t swapchainImageCount;
    
    vkGetSwapchainImagesKHR(context->device, context->swapchain, &swapchainImageCount, nullptr);
    
    swapchainImages.resize(swapchainImageCount);
    swapchainImageViews.resize(swapchainImages.size());
    
    vkGetSwapchainImagesKHR(context->device, context->swapchain, &swapchainImageCount, swapchainImages.data());
    
    for (size_t i = 0; i < swapchainImageCount; i++) {
        
        VkImageViewCreateInfo imageInfo{};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageInfo.image         = swapchainImages[i];
        imageInfo.viewType      = VK_IMAGE_VIEW_TYPE_2D;
        imageInfo.format        = context->format;
        
        imageInfo.components.r  = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageInfo.components.g  = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageInfo.components.b  = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageInfo.components.a  = VK_COMPONENT_SWIZZLE_IDENTITY;
        
        imageInfo.subresourceRange.aspectMask       = VK_IMAGE_ASPECT_COLOR_BIT;
        imageInfo.subresourceRange.baseMipLevel     = 0;
        imageInfo.subresourceRange.baseArrayLayer   = 0;
        imageInfo.subresourceRange.levelCount       = 1;
        imageInfo.subresourceRange.layerCount       = 1;
        
        if (vkCreateImageView(context->device, &imageInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) anopol_assert("Couldn't create image views");
    }
}

//------------------------------------------------------------------------------------------//
// Initializer
//------------------------------------------------------------------------------------------//

void initializeVulkanDependenices() {
    
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, nullptr);
    
    if (physicalDeviceCount == 0) anopol_assert("No VkPhysicalDevice(s) found");
    
    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(physicalDeviceCount);
    
    vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, physicalDevices.data());
    
    context->physicalDevice = findPhysicalDevice(physicalDevices);
    msaaSamples = getSampleCount();
    
    createDevice();
    createSwapchain();
    
    queueFamily family = anopol::ll::findQueueFamily(context->physicalDevice);
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.queueFamilyIndex = family.graphicsFamily.value();

    if (vkCreateCommandPool(context->device,
                            &poolCreateInfo, nullptr,
                            &commandPool) != VK_SUCCESS) throw std::runtime_error("CommandPool");
    
    createDepth();
}

//------------------------------------------------------------------------------------------//
// Clean up
//------------------------------------------------------------------------------------------//

void freeMemory() {
    
    vkDestroyCommandPool(context->device, commandPool, nullptr);
    vkDestroyImageView(context->device, depthImageView, nullptr);
    vkDestroyImage(context->device, depthImage, nullptr);
    vkFreeMemory(context->device, depthImageMemory, nullptr);
    
    vkDestroyDevice(context->device, nullptr);
    
    vkDestroySurfaceKHR(context->instance, context->surface, nullptr);
    vkDestroyInstance(context->instance, nullptr);
    glfwDestroyWindow(context->window);
    glfwTerminate();
    free(context);
}

void freeSwapchain() {
    
    for (VkImageView imageView : swapchainImageViews) {
        vkDestroyImageView(context->device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(context->device, context->swapchain, nullptr);
}

}

#endif /* internal_h */
