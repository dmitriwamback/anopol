//
//  texture.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-12-16.
//

#ifndef texture_h
#define texture_h

namespace anopol::render::texture {

class Texture {
public:
    VkImageView textureImageView;
    VkImage textureImage;
    VkSampler sampler;
    
    static Texture LoadTexture(const char* path);
    void Dealloc();
    
private:
    VkDeviceMemory textureImageMemory;
};


Texture Texture::LoadTexture(const char* path) {
    
    Texture texture = Texture();
    
    int width, height, channels;
    float* pixels = stbi_loadf(path, &width, &height, &channels, STBI_rgb_alpha);
    
    VkDeviceSize imageSize = uint64_t(width) * uint64_t(height) * 4 * sizeof(float);
    
    VkBuffer staging;
    VkDeviceMemory stagingMemory;
    
    anopol::ll::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, stagingMemory);
    
    void *data;
    vkMapMemory(context->device, stagingMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(context->device, stagingMemory);
    
    stbi_image_free(pixels);
    
    VkImage textureImage;
    VkImageView textureImageView;
    VkDeviceMemory textureImageMemory;
    
    anopol::ll::createImage(width, height,
                            VK_FORMAT_R32G32B32A32_SFLOAT,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            textureImage,
                            textureImageMemory);
    
    anopol::ll::imageLayoutTransition(textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    anopol::ll::memCopyBufferToImage(staging, textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    anopol::ll::imageLayoutTransition(textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    vkDestroyBuffer(context->device, staging, nullptr);
    vkFreeMemory(context->device, stagingMemory, nullptr);
    
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image                           = textureImage;
    imageViewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format                          = VK_FORMAT_R32G32B32A32_SFLOAT;
    imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
    imageViewCreateInfo.subresourceRange.levelCount     = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount     = 1;
    
    if (vkCreateImageView(context->device, &imageViewCreateInfo, nullptr, &textureImageView) != VK_SUCCESS) {
        anopol_assert("Failed to create texture image view");
    }
    
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(context->physicalDevice, &supportedFeatures);
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = supportedFeatures.samplerAnisotropy ? VK_TRUE : VK_FALSE;
    
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = deviceFeatures.samplerAnisotropy ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = deviceFeatures.samplerAnisotropy ? 16.0f : 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    vkCreateSampler(context->device, &samplerInfo, nullptr, &texture.sampler);
    
    texture.textureImageView = textureImageView;
    texture.textureImage = textureImage;
    texture.textureImageMemory = textureImageMemory;
    
    return texture;
}

void Texture::Dealloc() {
    vkDestroySampler(context->device, sampler, nullptr);
    vkDestroyImage(context->device, textureImage, nullptr);
    vkDestroyImageView(context->device, textureImageView, nullptr);
    vkFreeMemory(context->device, textureImageMemory, nullptr);
}

}

#endif /* texture_h */
