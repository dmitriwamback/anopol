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

std::vector<VkFramebuffer> framebuffers = std::vector<VkFramebuffer>();
std::vector<VkCommandBuffer> commandbuffers = std::vector<VkCommandBuffer>();
VkRenderPass renderpass;


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
            family.presentFamily = i;
        }
        
        if (family.graphicsFamily.has_value() && family.presentFamily.has_value()) {
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
    return family.graphicsFamily.has_value() && family.presentFamily.has_value() && adequate;
}

VkPhysicalDevice findPhysicalDevice(std::vector<VkPhysicalDevice> devices) {
    
    for (VkPhysicalDevice device : devices) {
        
        if (isPhysicalDeviceSuitable(device)) return device;
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
    
    for (uint32_t qFamily : std::set<uint32_t>{family.graphicsFamily.value(), family.presentFamily.value()}) {
        
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
    vkGetDeviceQueue(context->device, family.presentFamily.value(), 0, &context->presentQueue);
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
    
    if (indices.presentFamily != indices.graphicsFamily) {
        uint32_t q[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
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
    framebuffers.resize(swapchainImages.size());
    
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
    createDevice();
    createSwapchain();
}
}

#endif /* internal_h */
