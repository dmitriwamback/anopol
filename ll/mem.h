//
//  mem.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-10-14.
//

#ifndef mem_h
#define mem_h

namespace anopol::ll {

uint32_t findMemoryType(uint32_t filter, VkMemoryPropertyFlags properties) {
    
    VkPhysicalDeviceMemoryProperties mem;
    vkGetPhysicalDeviceMemoryProperties(context->physicalDevice, &mem);
    
    for (uint32_t i = 0; i < mem.memoryTypeCount; i++) {
        if (filter & (1 << 0) && (mem.memoryTypes[i].propertyFlags & properties) == properties) return i;
    }
    anopol_assert("Couldn't find memory type");
}

}

#endif /* mem_h */
