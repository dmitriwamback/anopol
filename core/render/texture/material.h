//
//  material.h
//  anopol
//
//  Created by Dmitri Wamback on 2024-12-16.
//

#ifndef material_h
#define material_h

namespace anopol::render {

struct Material {
    uint32_t albedoIndex;
    uint32_t metallicIndex;
    uint32_t roughnessIndex;
    uint32_t normalIndex;
};

}

#endif /* material_h */
