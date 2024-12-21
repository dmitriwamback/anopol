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
    static Texture LoadTexture(const char* path);
};


Texture Texture::LoadTexture(const char* path) {
    
    Texture texture = Texture();
    
    int width, height, channels;
    float* pixels = stbi_loadf(path, &width, &height, &channels, STBI_rgb_alpha);
    
    VkDeviceSize imageSize = width * height * 4;
    
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
                            VK_FORMAT_R8G8B8A8_SRGB,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            textureImage,
                            textureImageMemory);
    
    anopol::ll::imageLayoutTransition(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    anopol::ll::memCopyBufferToImage(staging, textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    anopol::ll::imageLayoutTransition(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    vkDestroyBuffer(context->device, staging, nullptr);
    vkFreeMemory(context->device, stagingMemory, nullptr);
    
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image                           = textureImage;
    imageViewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format                          = VK_FORMAT_R8G8B8A8_SRGB;
    imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
    imageViewCreateInfo.subresourceRange.levelCount     = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount     = 1;
    
    if (vkCreateImageView(context->device, &imageViewCreateInfo, nullptr, &textureImageView) != VK_SUCCESS) {
        anopol_assert("Failed to create texture image view");
    }
    
    
    
    return texture;
}

}

#endif /* texture_h */
