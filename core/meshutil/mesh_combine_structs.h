//
//  mesh_combine_structs.h
//  anopol
//
//  Created by Dmitri Wamback on 2025-04-13.
//

#ifndef mesh_combine_structs_h
#define mesh_combine_structs_h

namespace anopol::batch {

enum meshDrawType {
    indexed, nonIndexed
};

typedef struct batchDrawInformation {
    
    meshDrawType drawType;
    
    uint32_t firstIndex;
    uint32_t indexCount;
    uint32_t vertexOffset;
    uint32_t object;
    
    uint32_t firstVertex;
    uint32_t vertexCount;
    
    uint32_t texture;
    
} batchDrawInformation;

}

#endif /* mesh_combine_structs_h */
